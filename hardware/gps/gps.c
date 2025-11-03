/*
** Copyright 2006, The Android Open Source Project
** Copyright 2009, Michael Trimarchi <michael@panicking.kicks-ass.org>
** Copyright 2015, Keith Conger <keith.conger@gmail.com>
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free
** Software Foundation; either version 2, or (at your option) any later
** version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
** more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59
** Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**/

#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define  LOG_TAG  "gps_serial"

#include <cutils/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <hardware/gps.h>

#if (12 <= __ANDROID_API__) || defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || defined(_DEFAULT_SOURCE)
    #define _USE_TIMEGM
#endif

/* this is the state of our connection to the qemu_gpsd daemon */
typedef struct {
    int                     init;
    int                     fd;
    GpsCallbacks            *callbacks;
    GpsStatus               status;
    pthread_t               thread;
    int                     control[2];
} GpsState;

static GpsState       _gps_state[1];
static int            id_in_fixed[12];
static unsigned short period_in_ms;
static long           time_sync;   // seconds threshold to trigger sync via helper

// HW-detection + fallback
static volatile int   g_hw_detected    = 0;
static volatile int   g_fallback_active= 0;
static int64_t        g_session_start_ms = 0;

// Enable debug logs
#define  GPS_DEBUG  1

#define  DFR(...)   ALOGD(__VA_ARGS__)

#if GPS_DEBUG
#  define  D(...)   ALOGD(__VA_ARGS__)
#else
#  define  D(...)   ((void)0)
#endif

#define GPS_DEV_SLOW_UPDATE_RATE (10)
#define GPS_DEV_HIGH_UPDATE_RATE (1)

static void gps_dev_set_meas_rate(int fd, unsigned short period_ms);

/* ===================== UBX CONFIG FOR U-BLOX M8 ===================== */
#define UBX_SYNC_CHAR1 0xB5
#define UBX_SYNC_CHAR2 0x62

/* property helper: default ON if unset */
static int prop_get_bool_default_on(const char* key) {
    char v[PROPERTY_VALUE_MAX];
    if (property_get(key, v, "") <= 0) return 1;
    return (v[0] == '1' || v[0] == 't' || v[0] == 'T' || !strcasecmp(v, "true") || !strcasecmp(v, "on"));
}

static void ubx_cksum(const uint8_t* data, uint16_t len, uint8_t* cka, uint8_t* ckb) {
    uint8_t a = 0, b = 0;
    for (uint16_t i = 0; i < len; ++i) { a += data[i]; b += a; }
    *cka = a; *ckb = b;
}

static ssize_t ubx_send(int fd, uint8_t cls, uint8_t id, const void* payload, uint16_t len) {
    uint8_t hdr[6] = {UBX_SYNC_CHAR1, UBX_SYNC_CHAR2, cls, id, (uint8_t)(len & 0xFF), (uint8_t)(len >> 8)};
    uint8_t sum[2];
    uint8_t tmp[4 + (len ? len : 0)];
    tmp[0]=cls; tmp[1]=id; tmp[2]=(uint8_t)(len & 0xFF); tmp[3]=(uint8_t)(len >> 8);
    if (len) memcpy(&tmp[4], payload, len);
    ubx_cksum(tmp, (uint16_t)(4 + len), &sum[0], &sum[1]);
    if (write(fd, hdr, 6) != 6) return -1;
    if (len && write(fd, payload, len) != (ssize_t)len) return -1;
    if (write(fd, sum, 2) != 2) return -1;
    return 6 + len + 2;
}

static void ubx_cfg_rxm_continuous(int fd) {
    /* UBX-CFG-RXM (0x06 0x11), len=2: lpMode=0 (continuous), reserved=0 */
    uint8_t p[2] = {0x00, 0x00};
    ubx_send(fd, 0x06, 0x11, p, sizeof(p));
}

static void ubx_cfg_gnss_multiconstellation(int fd) {
    /* UBX-CFG-GNSS (0x06 0x3E) -> GPS+Galileo+GLONASS+SBAS+QZSS, 32 total channels */
    struct __attribute__((packed)) {
        uint8_t numTrkCh;
        uint8_t numBlocks;
        uint16_t reserved2;
        struct {
            uint8_t gnssId, resTrkCh, maxTrkCh, reserved1;
            uint32_t flags;
        } b[5];
    } cfg = {
        .numTrkCh = 32,
        .numBlocks = 5,
        .reserved2 = 0,
        .b = {
            {0, 8, 8, 0, 0x00000001}, /* GPS     */
            {1, 3, 3, 0, 0x00000001}, /* SBAS    */
            {2, 8, 8, 0, 0x00000001}, /* Galileo */
            {5, 3, 3, 0, 0x00000001}, /* QZSS    */
            {6, 8, 8, 0, 0x00000001}, /* GLONASS */
        }
    };
    ubx_send(fd, 0x06, 0x3E, &cfg, sizeof(cfg));
}

/* UBX-CFG-NMEA (0x06 0x17): emit combined GN talker ($GNGSV), high precision, NMEA 4.1 */
static void ubx_cfg_nmea_gn_combined(int fd) {
    /* M8 payload length = 20:
       filter(1)=0, nmeaVersion(1)=0x41, numSV(1)=0 (auto), flags(1)=HP,
       gnssToFilter(4)=0 (no filtering),
       svNumbering(1)=0 (strict), mainTalkerId(1)=0 (GN), gsvTalkerId(1)=0 (match main),
       version(1)=0, bdsTalkerId(2)=0, reserved(6)=0 */
    struct __attribute__((packed)) {
        uint8_t  filter;
        uint8_t  nmeaVersion;
        uint8_t  numSV;
        uint8_t  flags;
        uint32_t gnssToFilter;
        uint8_t  svNumbering;
        uint8_t  mainTalkerId;
        uint8_t  gsvTalkerId;
        uint8_t  version;
        uint16_t bdsTalkerId;
        uint8_t  reserved2[6];
    } p = {0};
    p.nmeaVersion   = 0x41;     /* NMEA 4.1 */
    p.flags         = (1 << 1); /* High precision fields */
    p.mainTalkerId  = 0;        /* GN */
    p.gsvTalkerId   = 0;        /* use main talker */
    ubx_send(fd, 0x06, 0x17, &p, sizeof(p));
}

static void ubx_cfg_nav5_fast_fix(int fd) {
    /* UBX-CFG-NAV5 (0x06 0x24), len=36 (M8) */
    struct __attribute__((packed)) {
        uint16_t mask;
        uint8_t dynModel;
        uint8_t fixMode;
        int32_t fixedAlt;
        uint32_t fixedAltVar;
        int8_t minElev;
        uint8_t drLimit;
        uint16_t pDop, tDop;
        uint16_t pAcc, tAcc;
        uint8_t staticHoldThresh;
        uint8_t dgnssTimeout;
        uint8_t cnoThreshNumSVs;
        uint8_t cnoThresh;
        uint16_t reserved1;
        uint16_t staticHoldMaxDist;
        uint8_t utcStandard;
        uint8_t reserved2[5];
    } p = {0};
    p.mask = 0x000F;           /* dyn|fixMode|minElev|posFixMode */
    p.dynModel = 6;            /* Automotive */
    p.fixMode = 3;             /* 3D only */
    p.minElev = 5;             /* deg */
    p.pDop = 30; p.tDop = 30;  /* 3.0 */
    p.pAcc = 100;              /* cm */
    p.tAcc = 300;              /* ms */
    ubx_send(fd, 0x06, 0x24, &p, sizeof(p));
}

static void ubx_cfg_navx5_relaxed_gate(int fd) {
    /* UBX-CFG-NAVX5 (0x06 0x23): minSVs=4, minCNO=5 dB-Hz -> allow weak initial fix */
    struct __attribute__((packed)) {
        uint16_t version;
        uint16_t mask1;
        uint32_t reserved0;
        uint8_t minSVs;
        uint8_t maxSVs;
        uint8_t minCNO;
        uint8_t reserved1;
        uint8_t iniFix3D;
        uint8_t reserved2[2];
        uint8_t ackAiding;
        uint32_t reserved3[2];
        uint16_t wknRollover;
        uint8_t reserved4[2];
        uint8_t usePPP;
        uint8_t aopCfg;
        uint16_t reserved5;
        uint32_t aopOrbMaxErr;
        uint8_t reserved6[7];
        uint8_t useAdr;
    } p = {0};
    p.version = 0;
    p.mask1   = 0x0018; /* minSVs + minCNO */
    p.minSVs  = 4;
    p.minCNO  = 5;
    p.iniFix3D = 1;
    ubx_send(fd, 0x06, 0x23, &p, sizeof(p));
}

static void ubx_cfg_sbas_egnos(int fd) {
    /* UBX-CFG-SBAS (0x06 0x16): enable, use ranging+corrections+integrity, auto scan */
    struct __attribute__((packed)) {
        uint8_t mode;
        uint8_t usage;
        uint8_t maxSBAS;
        uint8_t scanmode2;
        uint32_t scanmode1;
    } p = {0};
    p.mode = 1;      /* enable */
    p.usage = 7;     /* ranging + corrections + integrity */
    p.maxSBAS = 3;
    p.scanmode2 = 0;
    p.scanmode1 = 0; /* auto */
    ubx_send(fd, 0x06, 0x16, &p, sizeof(p));
}

static void ubx_cfg_rate_1hz(int fd) {
    /* UBX-CFG-RATE (0x06 0x08), measRate=1000ms, navRate=1, timeRef=GPS(1) */
    struct __attribute__((packed)) { uint16_t measRate, navRate, timeRef; } p = {1000, 1, 1};
    ubx_send(fd, 0x06, 0x08, &p, sizeof(p));
}

static void ubx_cfg_ant_defaults(int fd) {
    /* UBX-CFG-ANT (0x06 0x13), flags=0x0005 (PIO+short/open), pins=0 */
    struct __attribute__((packed)) { uint16_t flags, pins; } p = {0x0005, 0x0000};
    ubx_send(fd, 0x06, 0x13, &p, sizeof(p));
}

static void ubx_cfg_msg_nav(int fd) {
    /* UBX-CFG-MSG (0x06 0x01): enable NAV-PVT (0x01 0x07) + NAV-SAT (0x01 0x35) on UART1 */
    struct __attribute__((packed)) { uint8_t cls, id, rateUart1, rateUart2, rateUSB, rateSPI, rateI2C; } m;
    m.cls=0x01; m.id=0x07; m.rateUart1=1; m.rateUart2=0; m.rateUSB=0; m.rateSPI=0; m.rateI2C=0;
    ubx_send(fd, 0x06, 0x01, &m, sizeof(m));
    m.cls=0x01; m.id=0x35; m.rateUart1=1; m.rateUart2=0; m.rateUSB=0; m.rateSPI=0; m.rateI2C=0;
    ubx_send(fd, 0x06, 0x01, &m, sizeof(m));
}

static void ubx_cfg_cfg_save(int fd) {
    /* UBX-CFG-CFG (0x06 0x09): save to BBR + Flash (IO|MSG|INF|NAV|RXM|SEN) */
    struct __attribute__((packed)) { uint32_t clearMask, saveMask, loadMask; } p = {0, 0x00001F1F, 0};
    ubx_send(fd, 0x06, 0x09, &p, sizeof(p));
}

static void gps_dev_configure_ublox_m8(int fd) {
    if (fd < 0) return;
    if (!prop_get_bool_default_on("persist.tesla-android.gnss.ubx_enable")) {
        D("UBX config skipped by property");
        return;
    }
    D("UBX: configuring u-blox M8 for fast fix + multi-GNSS + warm-start");
    ubx_cfg_rxm_continuous(fd);
    ubx_cfg_gnss_multiconstellation(fd);
    ubx_cfg_nmea_gn_combined(fd);    /* NEW: combined $GNGSV output */
    ubx_cfg_nav5_fast_fix(fd);
    ubx_cfg_navx5_relaxed_gate(fd);
    ubx_cfg_sbas_egnos(fd);
    ubx_cfg_rate_1hz(fd);
    ubx_cfg_ant_defaults(fd);
    ubx_cfg_msg_nav(fd);
    ubx_cfg_cfg_save(fd);
}
/* =================== END UBX CONFIG FOR U-BLOX M8 ==================== */

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****                 H E L P E R S                         *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

static inline int64_t wall_time_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
static inline int64_t now_monotonic_ms(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* --- derive a reasonable horizontal accuracy (meters) from HDOP/fix --- */
static inline float derive_hacc_m(float hdop, int fix_quality) {
    if (hdop > 0.0f && isfinite(hdop)) return fmaxf(5.0f, hdop * 5.0f); /* rule-of-thumb */
    if (fix_quality > 0) return 30.0f;                                  /* standalone GPS */
    return 100.0f;                                                       /* no fix yet */
}

/* --- sanity check for coordinates --- */
static inline int valid_latlon(double lat, double lon) {
    if (!isfinite(lat) || !isfinite(lon)) return 0;
    if (lat < -90.0 || lat > 90.0) return 0;
    if (lon < -180.0 || lon > 180.0) return 0;
    if (lat == 0.0 && lon == 0.0) return 0; /* framework often rejects 0,0 */
    return 1;
}

/* property helpers */
static int prop_get_bool(const char* key) {
    char v[PROPERTY_VALUE_MAX];
    property_get(key, v, "0");
    return (v[0] == '1' || v[0] == 't' || v[0] == 'T' || !strcasecmp(v, "true"));
}
static double prop_get_double(const char* key, double defv) {
    char v[PROPERTY_VALUE_MAX];
    if (property_get(key, v, "") <= 0) return defv;
    return strtod(v, NULL);
}
static float prop_get_float(const char* key, float defv) {
    char v[PROPERTY_VALUE_MAX];
    if (property_get(key, v, "") <= 0) return defv;
    return (float)strtod(v, NULL);
}
static int64_t prop_get_i64(const char* key, int64_t defv) {
    char v[PROPERTY_VALUE_MAX];
    if (property_get(key, v, "") <= 0) return defv;
    return (int64_t)strtoll(v, NULL, 10);
}

/* Trigger the privileged timeset helper via init properties. */
static void gps_timesync_trigger(int64_t utc_ms) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)(utc_ms / 1000LL));
    if (property_set("sys.vendor.gps.epoch_s", buf) < 0) {
        ALOGE("gps_timesync: property_set epoch failed");
        return;
    }
    if (property_set("ctl.start", "vendor.gps-timeset") < 0) {
        ALOGE("gps_timesync: failed to start vendor.gps-timeset");
        return;
    }
    D("gps_timesync: requested system time set to %s (epoch s)", buf);
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

#define  MAX_NMEA_TOKENS  32

typedef struct {
    const char*  p;
    const char*  end;
} Token;

typedef struct {
    int     count;
    Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end )
{
    int count = 0;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentence
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (count < MAX_NMEA_TOKENS) {
            t->tokens[count].p = p;
            t->tokens[count].end = q;
            count += 1;
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
    Token  tok;

    if (index < 0 || index >= t->count || index >= MAX_NMEA_TOKENS)
        tok.p = tok.end = "";
    else
        tok = t->tokens[index];

    return tok;
}

static int
str2int( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;

    for ( ; len > 0; len--, p++ ) {
        int  c;

        if (p >= end)
            goto Fail;

        c = *p - '0';
        if ((unsigned)c >= 10)
            goto Fail;

        result = result*10 + c;
    }
    return  result;

Fail:
    return -1;
}

static double
str2float( const char*  p, const char*  end )
{
    size_t len = end - p;
    char   temp[32];

    if (len >= sizeof(temp))
        return 0.;

    memcpy( temp, p, len );
    temp[len] = '\0';
    return strtod( temp, NULL );
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

#define  NMEA_MAX_SIZE  255

typedef struct {
    int     pos;
    int     overflow;
    int     utc_year;
    int     utc_mon;
    int     utc_day;
    bool    gsa; // TRUE if GSA sentence was detected
    GpsLocation  fix;
    GpsSvStatus sv_status;
    gps_location_callback  callback;
    char    in[ NMEA_MAX_SIZE+1 ];
} NmeaReader;

void update_gps_status(GpsStatusValue val)
{
    GpsState*  state = _gps_state;
    state->status.status=val;
    if (state->callbacks->status_cb)
        state->callbacks->status_cb(&state->status);
}

void update_gps_svstatus(GpsSvStatus *val)
{
    GpsState*  state = _gps_state;
    if (state->callbacks->sv_status_cb)
        state->callbacks->sv_status_cb(val);
}

void update_gps_location(GpsLocation *fix)
{
    GpsState*  state = _gps_state;
    if (state->callbacks->location_cb)
        state->callbacks->location_cb(fix);
}

#ifndef _USE_TIMEGM
static time_t get_utc_diff()
{
    // 3rd January, 1970, Time: 00:00:00
    static struct tm tm = { 0, 0, 0, 3, 0, 70, 0, 0, -1 };
    return (2 * 24 * 3600) - mktime(&tm);
}
#endif // _USE_TIMEGM

static void
nmea_reader_init( NmeaReader*  r )
{
    memset( r, 0, sizeof(*r) );

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
    r->gsa      = false;
    r->callback = NULL;
    r->fix.size = sizeof(r->fix);
}

/* -------- GSV epoch combiner: merge all talkers into one snapshot -------- */
typedef struct {
    int   total_pages[6];   // per talker index
    int   got_page[6];      // bitmask of pages seen
    int   fill;
    GpsSvStatus comb;       // combined SV list
    int64_t last_flush_ms;
} GsvEpoch;

static GsvEpoch g_epoch;

static inline char current_talker_char(const NmeaReader* r) {
    if (r->pos >= 3 && r->in[0] == '$' && r->in[1] == 'G') return r->in[2];
    return 'P';
}

static inline int talker_index(char tlk) {
    switch (tlk) {
        case 'P': return 0; /* GPS */
        case 'A': return 1; /* Galileo */
        case 'L': return 2; /* GLONASS */
        case 'Q': return 3; /* QZSS */
        case 'B': return 4; /* BeiDou */
        case 'N': return 5; /* GN (combined) */
        default:  return 5;
    }
}

static inline int is_gps_prn_for_mask(int prn) {
    return prn >= 1 && prn <= 32;
}

static void gsv_epoch_reset_if_stale(void) {
    int64_t now = now_monotonic_ms();
    if (now - g_epoch.last_flush_ms > 1500) {
        memset(&g_epoch, 0, sizeof(g_epoch));
        g_epoch.comb.size = sizeof(GpsSvStatus);
        g_epoch.last_flush_ms = now;
    }
}

static void gsv_epoch_append(Token prn, Token elev, Token azi, Token snr) {
    if (g_epoch.fill >= GPS_MAX_SVS) return;
    if (prn.end <= prn.p) return;
    int i = g_epoch.fill++;
    g_epoch.comb.sv_list[i].prn       = str2int(prn.p, prn.end);
    g_epoch.comb.sv_list[i].elevation = str2int(elev.p, elev.end);
    g_epoch.comb.sv_list[i].azimuth   = str2int(azi.p , azi.end);
    g_epoch.comb.sv_list[i].snr       = str2int(snr.p , snr.end);
}

static void gsv_epoch_note_page(char tlk, int page, int total) {
    int ti = talker_index(tlk);
    if (total > 0) g_epoch.total_pages[ti] = total;
    if (page > 0 && page <= 16) g_epoch.got_page[ti] |= (1 << (page - 1));
}

static int talker_complete(int ti) {
    int total = g_epoch.total_pages[ti];
    if (total <= 0 || total > 16) return 0;
    int mask = (1 << total) - 1;
    return (g_epoch.got_page[ti] & mask) == mask;
}

static int all_done_or_timeout(void) {
    int64_t now = now_monotonic_ms();
    if (talker_complete(5)) return 1; /* GN done */
    for (int ti = 0; ti < 5; ++ti) {
        if (talker_complete(ti)) {
            if (now - g_epoch.last_flush_ms >= 300) return 1;
        }
    }
    return 0;
}

static void gsv_epoch_publish_if_ready(const NmeaReader* r) {
    if (!all_done_or_timeout()) return;
    if (g_epoch.fill <= 0) { g_epoch.last_flush_ms = now_monotonic_ms(); return; }
    g_epoch.comb.num_svs = g_epoch.fill;
    g_epoch.comb.used_in_fix_mask = r->sv_status.used_in_fix_mask;
    update_gps_svstatus(&g_epoch.comb);
    memset(&g_epoch, 0, sizeof(g_epoch));
    g_epoch.comb.size = sizeof(GpsSvStatus);
    g_epoch.last_flush_ms = now_monotonic_ms();
}

/* -------------------------------------------------------- */

static int
nmea_reader_update_time( NmeaReader*  r, Token  tok, time_t *gmt )
{
    struct tm  tm;

    if (tok.p + 6 > tok.end)
        return -1;

    if (r->utc_year < 0) {
        // no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r( &now, &tm );
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    tm.tm_hour  = str2int(tok.p, tok.p+2);
    tm.tm_min   = str2int(tok.p+2, tok.p+4);
    tm.tm_sec   = (int) str2float(tok.p+4, tok.end);
    tm.tm_year  = r->utc_year - 1900;
    tm.tm_mon   = r->utc_mon - 1;
    tm.tm_mday  = r->utc_day;
    tm.tm_isdst = -1;

#ifdef _USE_TIMEGM
    *gmt = timegm( &tm );
#else
    *gmt = mktime( &tm ) + get_utc_diff();
#endif
    r->fix.timestamp = (long long) *gmt * 1000;
    return 0;
}

static int
nmea_reader_update_date( NmeaReader*  r, Token  date_tok, Token  time_tok )
{
    Token  tok = date_tok;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        D("Date not properly formatted: '%.*s'", (int)(tok.end - tok.p), tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        D("Date not properly formatted: '%.*s'", (int)(tok.end - tok.p), tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    time_t gmt;
    int result = nmea_reader_update_time( r, time_tok, &gmt );

    // Time sync: trigger privileged helper if drift beyond threshold
    if (time_sync > 0) {
        long dif = (long) (time(NULL) - gmt); // seconds
        if (dif < -time_sync || dif > time_sync) {
            int64_t utc_ms = (int64_t)gmt * 1000LL;
            D("gps_serial: requesting system time sync to %ld (diff=%ld s)", (long)gmt, dif);
            gps_timesync_trigger(utc_ms);
        }
    }

    return result;
}

static double
convert_from_hhmm( Token  tok )
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}

static int
nmea_reader_update_latlong( NmeaReader*  r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi )
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        D("Latitude is too short: '%.*s'", (int)(tok.end - tok.p), tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        D("Longitude is too short: '%.*s'", (int)(tok.end - tok.p), tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    return 0;
}

static int
nmea_reader_update_altitude( NmeaReader*  r,
                             Token        altitude,
                             Token        units )
{
    (void)units; /* silence -Wunused-parameter */
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    r->fix.altitude = str2float(tok.p, tok.end);
    return 0;
}

/* Treat NMEA token as HDOP and derive meters; set flag only when there is a fix */
static int nmea_reader_update_accuracy(NmeaReader* r, Token accuracy, bool is_fix)
{
    if (accuracy.p >= accuracy.end)
        return -1;

    float hdop = (float) str2float(accuracy.p, accuracy.end);
    float hacc = derive_hacc_m(hdop, is_fix ? 1 : 0);

    r->fix.accuracy = hacc;

    if (is_fix) {
        r->fix.flags |= GPS_LOCATION_HAS_ACCURACY;
    }
    return 0;
}

static int
nmea_reader_update_bearing( NmeaReader*  r,
                            Token        bearing )
{
    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    r->fix.bearing  = str2float(tok.p, tok.end);
    return 0;
}

static int
nmea_reader_update_speed( NmeaReader*  r,
                          Token        speed )
{
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags |= GPS_LOCATION_HAS_SPEED;
    r->fix.speed = (float) (str2float(tok.p, tok.end) * (1.852 / 3.6));
    return 0;
}

static void
nmea_reader_parse( NmeaReader*  r )
{
    /* we received a complete sentence, now parse it */
    NmeaTokenizer  tzer[1];
    Token          tok;
    struct timeval tv;

    D("Received: '%.*s'", r->pos, r->in);
    if (r->pos < 9) {
        D("Too short. discarded.");
        return;
    }

    r->in[r->pos] = 0;

    gettimeofday(&tv, NULL);
    if (_gps_state->init)
        _gps_state->callbacks->nmea_cb(tv.tv_sec*1000+tv.tv_usec/1000, r->in, r->pos);

    nmea_tokenizer_init(tzer, r->in, r->in + r->pos);

    // Mark HW detected on first parsed sentence
    if (!g_hw_detected) {
        property_set("persist.tesla-android.gps_hw.is_detected", "1");
        g_hw_detected = 1;
        g_fallback_active = 0; // stop fallback if it was active
    }

#if GPS_DEBUG
    {
        int  n;
        D("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tokd = nmea_tokenizer_get(tzer,n);
            D("%2d: '%.*s'", n, (int)(tokd.end - tokd.p), tokd.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        D("Sentence id '%.*s' too short, ignored.", (int)(tok.end - tok.p), tok.p);
        return;
    }

    // ignore first two characters.
    tok.p += 2;

    bool send_msg = false;
    if ( !memcmp(tok.p, "GGA", 3) ) {
        // GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
        Token  tok_fix           = nmea_tokenizer_get(tzer,6);
        Token  tok_accuracy      = nmea_tokenizer_get(tzer,8);
        Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer,10);

        int fix = str2int(tok_fix.p, tok_fix.end);
        if (0 < fix)
        {
            time_t gmt;
            nmea_reader_update_time(r, tok_time, &gmt);
            nmea_reader_update_latlong(r, tok_latitude, tok_latitudeHemi.p[0], tok_longitude, tok_longitudeHemi.p[0]);
            nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);
        }

        if (!r->gsa)
        {
            nmea_reader_update_accuracy(r, tok_accuracy, 0 < fix);
            if (fix > 0) send_msg = true;   /* only send after a real fix */
        }
    } else if ( !memcmp(tok.p, "GSA", 3) ) {
        // Determine talker from the raw sentence to decide mask updates
        char tlk = current_talker_char(r);

        Token tok_fix = nmea_tokenizer_get(tzer,2);   // 1=no, 2=2D, 3=3D
        int fix = str2int(tok_fix.p, tok_fix.end);

        if (fix == 2) r->fix.flags &= ~GPS_LOCATION_HAS_ALTITUDE;

        // IDs 3..14
        uint32_t gps_mask = r->sv_status.used_in_fix_mask; // keep existing bits
        for (int i = 0; i < 12; i++) {
            Token tok_id = nmea_tokenizer_get(tzer, 3 + i);
            if (tok_id.end > tok_id.p) {
                int prn = str2int(tok_id.p, tok_id.end);
                id_in_fixed[i] = prn;
                D("Satellite used '%.*s'", (int)(tok_id.end - tok_id.p), tok_id.p);

                // Only set mask bits for **GPS PRNs 1..32**; ignore others
                if (tlk == 'P' || tlk == 'N') { // GPxxx or GNxxx may carry GPS IDs
                    if (is_gps_prn_for_mask(prn)) gps_mask |= (1u << (prn - 1));
                }
            } else {
                id_in_fixed[i] = 0;
            }
        }
        r->sv_status.used_in_fix_mask = gps_mask;

        // HDOP as accuracy (token 16), but set flag only if there is at least 2D
        Token tok_hdop = nmea_tokenizer_get(tzer,16);
        nmea_reader_update_accuracy(r, tok_hdop, fix > 1);
        if (fix > 1) send_msg = true;

        r->gsa = true;
    } else if ( !memcmp(tok.p, "GSV", 3) ) {
        char tlk = current_talker_char(r);

        Token tok_num_messages   = nmea_tokenizer_get(tzer,1);
        Token tok_msg_number     = nmea_tokenizer_get(tzer,2);
        /* Token tok_svs_inview  = nmea_tokenizer_get(tzer,3); (informational) */

        int total = str2int(tok_num_messages.p,tok_num_messages.end);
        int page  = str2int(tok_msg_number.p  ,tok_msg_number.end);

        gsv_epoch_reset_if_stale();
        gsv_epoch_note_page(tlk, page, total);

        // Up to 4 entries per page
        Token p1 = nmea_tokenizer_get(tzer,4),  e1 = nmea_tokenizer_get(tzer,5),
              a1 = nmea_tokenizer_get(tzer,6),  s1 = nmea_tokenizer_get(tzer,7);
        Token p2 = nmea_tokenizer_get(tzer,8),  e2 = nmea_tokenizer_get(tzer,9),
              a2 = nmea_tokenizer_get(tzer,10), s2 = nmea_tokenizer_get(tzer,11);
        Token p3 = nmea_tokenizer_get(tzer,12), e3 = nmea_tokenizer_get(tzer,13),
              a3 = nmea_tokenizer_get(tzer,14), s3 = nmea_tokenizer_get(tzer,15);
        Token p4 = nmea_tokenizer_get(tzer,16), e4 = nmea_tokenizer_get(tzer,17),
              a4 = nmea_tokenizer_get(tzer,18), s4 = nmea_tokenizer_get(tzer,19);

        gsv_epoch_append(p1,e1,a1,s1);
        gsv_epoch_append(p2,e2,a2,s2);
        gsv_epoch_append(p3,e3,a3,s3);
        gsv_epoch_append(p4,e4,a4,s4);

        gsv_epoch_publish_if_ready(r);

    } else if ( !memcmp(tok.p, "RMC", 3) ) {
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
        Token  tok_speed         = nmea_tokenizer_get(tzer,7);
        Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
        Token  tok_date          = nmea_tokenizer_get(tzer,9);

        D("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
        if (tok_fixStatus.p[0] == 'A') {
            nmea_reader_update_date( r, tok_date, tok_time );

            nmea_reader_update_latlong( r, tok_latitude,
                                           tok_latitudeHemi.p[0],
                                           tok_longitude,
                                           tok_longitudeHemi.p[0] );

            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed  ( r, tok_speed );
        }
    } else if ( !memcmp(tok.p, "VTG", 3) ) {
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer,9);

        if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != 'N') {
            Token  tok_bearing       = nmea_tokenizer_get(tzer,1);
            Token  tok_speed         = nmea_tokenizer_get(tzer,5);

            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed  ( r, tok_speed );
        }
    } else {
        tok.p -= 2;
        D("Unknown sentence '%.*s'", (int)(tok.end - tok.p), tok.p);
    }

#if GPS_DEBUG
    if (r->fix.flags) {
        char   temp[256];
        char*  p   = temp;
        char*  end = p + sizeof(temp);

        p += snprintf( p, end-p, "Sending fix" );
        if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
            p += snprintf(p,end-p, " accuracy=%g", r->fix.accuracy);
        }
        time_t secs = (time_t)(r->fix.timestamp / 1000LL);
        struct tm utc;
        gmtime_r(&secs, &utc);
        char tbuf[32];
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", &utc);
        p += snprintf(p, end-p, " time=%sZ", tbuf);
        D("%s\n", temp);
    }
#endif
    if (send_msg)
    {
        /* Ensure all required fields are present and sane */
        const int have_latlon = (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) != 0;
        const int have_acc    = (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) != 0;

        /* If we have a fix but accuracy flag missing, synthesize it conservatively */
        if (!have_acc) {
            r->fix.accuracy = derive_hacc_m(/*hdop*/ -1.0f, /*fix_quality*/ 1);
            r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
        }

        /* If timestamp is zero (no RMC/GGA time parsed yet), use wall clock */
        if (r->fix.timestamp == 0) {
            r->fix.timestamp = wall_time_ms();
        }

        /* Final guard: don't send (0,0) or invalid coords */
        if (have_latlon && valid_latlon(r->fix.latitude, r->fix.longitude)) {
            if (_gps_state->callbacks->location_cb)
            {
                _gps_state->callbacks->location_cb(&r->fix);
                r->fix.flags = 0;
            }
            else
            {
                D("No callback, keeping data until needed !");
            }
        } else {
            D("Skip report: invalid/empty latlon (flags=%x lat=%f lon=%f)",
              r->fix.flags, r->fix.latitude, r->fix.longitude);
        }
    }
}

static void
nmea_reader_addc( NmeaReader*  r, int  c )
{
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= (int) sizeof(r->in)-1 ) {
        r->overflow = 1;
        r->pos      = 0;
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos       += 1;

    if (c == '\n') {
        nmea_reader_parse( r );
        r->pos = 0;
    }
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       F A L L B A C K   I N J E C T O R               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

// Emits one synthetic SV status (0 seen), then one location fix from properties
static void fallback_inject_browser_fix(void) {
    GpsState* state = _gps_state;
    if (!state || !state->init || !state->callbacks) return;

    // Read properties
    double lat = prop_get_double("persist.tesla-android.gps.latitude", 0.0);
    double lon = prop_get_double("persist.tesla-android.gps.longitude", 0.0);
    double alt = prop_get_double("persist.tesla-android.gps.altitude", NAN);
    float acc  = prop_get_float ("persist.tesla-android.gps.accuracy", 10.0f);
    float spd  = prop_get_float ("persist.tesla-android.gps.speed", NAN);
    float brg  = prop_get_float ("persist.tesla-android.gps.bearing", NAN);
    int64_t t_ms = prop_get_i64("persist.tesla-android.gps.timestamp", wall_time_ms());

    // Basic sanity
    if (!valid_latlon(lat, lon)) {
        ALOGW("fallback: invalid lat/lon from properties; skipping injection");
        return;
    }

    // Mock a tiny SV snapshot (0 in view so we don't imply HW lock)
    GpsSvStatus sv;
    memset(&sv, 0, sizeof(sv));
    sv.size = sizeof(GpsSvStatus);
    sv.num_svs = 0;
    if (state->callbacks->sv_status_cb) {
        state->callbacks->sv_status_cb(&sv);
    }

    // Push a complete location
    GpsLocation fix;
    memset(&fix, 0, sizeof(fix));
    fix.size = sizeof(fix);
    fix.flags = GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY;

    fix.latitude  = lat;
    fix.longitude = lon;
    fix.accuracy  = (acc > 0.f ? acc : 10.f);
    fix.timestamp = (GpsUtcTime)t_ms;

    if (!isnan(alt)) {
        fix.flags |= GPS_LOCATION_HAS_ALTITUDE;
        fix.altitude = alt;
    }
    if (!isnan(spd)) {
        fix.flags |= GPS_LOCATION_HAS_SPEED;
        fix.speed = spd;
    }
    if (!isnan(brg)) {
        fix.flags |= GPS_LOCATION_HAS_BEARING;
        fix.bearing = brg;
    }

    if (state->callbacks->location_cb) {
        ALOGD("gps_serial: fallback location injected lat=%.6f lon=%.6f acc=%.1f ts=%lld",
              fix.latitude, fix.longitude, (double)fix.accuracy, (long long)fix.timestamp);
        state->callbacks->location_cb(&fix);
    }
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2
};

static void
gps_state_done( GpsState*  s )
{
    // tell the thread to quit, and wait for it
    char   cmd = CMD_QUIT;
    void*  dummy;
    write( s->control[0], &cmd, 1 );
    pthread_join(s->thread, &dummy);

    // close the control socket pair
    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;

    // close connection to the QEMU GPS daemon
    close( s->fd ); s->fd = -1;
    s->init = 0;
}

static void
gps_state_start( GpsState*  s )
{
    char  cmd = CMD_START;
    int   ret;

    do {
        ret = write( s->control[0], &cmd, 1 );
    } while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_START command: ret=%d: %s",
                __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_stop( GpsState*  s )
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static int
epoll_register( int  epoll_fd, int  fd )
{
    struct epoll_event  ev;
    int                 ret, flags;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}

/* this is the main thread */
static void
gps_state_thread( void*  arg )
{
    GpsState*   state = (GpsState*) arg;
    NmeaReader  reader[1];
    int         epoll_fd   = epoll_create(2);
    int         started    = 0;
    int         gps_fd     = state->fd;
    int         control_fd = state->control[1];

    nmea_reader_init( reader );

    // register control file descriptors for polling
    epoll_register( epoll_fd, control_fd );
    epoll_register( epoll_fd, gps_fd );

    D("GPS thread running");

    // now loop
    for (;;) {
        struct epoll_event   events[2];
        int                  ne, nevents;

        nevents = epoll_wait( epoll_fd, events, 2, 1000 /* 1s tick to drive fallback timer */ );
        if (nevents < 0) {
            if (errno != EINTR)
                ALOGE("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                ALOGE("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                return;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;

                if (fd == control_fd) {
                    char  cmd = (char)255;
                    int   ret;
                    D("GPS control fd event");
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        D("GPS thread quitting on demand");
                        return;
                    } else if (cmd == CMD_START) {
                        if (!started) {
                            D("GPS thread starting  location_cb=%p", state->callbacks->location_cb);
                            started = 1;
                            update_gps_status(GPS_STATUS_SESSION_BEGIN);
                            gps_dev_set_meas_rate(state->fd, period_in_ms);

                            // Arm detection window
                            g_session_start_ms = now_monotonic_ms();
                            g_hw_detected = 0;
                            g_fallback_active = 0;
                            property_set("persist.tesla-android.gps_hw.is_detected", "0");
                        }
                    } else if (cmd == CMD_STOP) {
                        if (started) {
                            D("GPS thread stopping");
                            started = 0;
                            update_gps_status(GPS_STATUS_SESSION_END);
                            gps_dev_set_meas_rate(state->fd, GPS_DEV_SLOW_UPDATE_RATE * 1000);
                            g_fallback_active = 0;
                        }
                    }
                } else if (fd == gps_fd) {
                    char  buff[32];
                    for (;;) {
                        int  nn, ret;

                        ret = read( fd, buff, sizeof(buff) );
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ALOGE("Error while reading from GPS daemon socket: %s:", strerror(errno));
                            break;
                        }
                        for (nn = 0; nn < ret; nn++)
                            nmea_reader_addc( reader, buff[nn] );
                    }
                } else {
                    ALOGE("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }

        // Late check for browser fallback: after 60s if no HW NMEA, enable and inject every tick
        if (started && !g_hw_detected) {
            int64_t elapsed = now_monotonic_ms() - g_session_start_ms;
            if (!g_fallback_active && elapsed >= 60000) {
                if (prop_get_bool("persist.tesla-android.gps.is_active")) {
                    ALOGD("gps_serial: no HW NMEA after 60s, enabling browser GNSS fallback");
                    g_fallback_active = 1;
                } else {
                    ALOGD("gps_serial: HW not detected after 60s and browser GNSS inactive");
                }
            }
        }
        if (started && g_fallback_active) {
            fallback_inject_browser_fix(); // every ~1s (tick)
        }
    }
}

static void
gps_state_init( GpsState*  state, GpsCallbacks* callbacks )
{
    char   prop[PROPERTY_VALUE_MAX];
    char   device[256];

    state->init       = 1;
    state->control[0] = -1;
    state->control[1] = -1;
    state->fd         = -1;
    state->callbacks  = callbacks;
    D("gps_state_init");

    // Look for a kernel-provided device name
    if (property_get("persist.vendor.gps.device", prop, "") == 0) {
        D("no kernel-provided gps device name");
        return;
    }

    snprintf(device, sizeof(device), "/dev/%s",prop);
    do {
        state->fd = open( device, O_RDWR );
    } while (state->fd < 0 && errno == EINTR);

    if (state->fd < 0) {
        ALOGE("could not open gps serial device %s: %s", device, strerror(errno) );
        return;
    }

    D("GPS will read from %s", device);

    period_in_ms = GPS_DEV_HIGH_UPDATE_RATE * 1000;
    if (property_get("persist.vendor.gps.max_rate", prop, "") != 0)
    {
        unsigned long rate = strtoul(prop, NULL, 10);
        if (0 < rate && rate < 66)
            period_in_ms = (unsigned short) (rate * 1000);
        else if (250 <= rate && rate < 65536)
            period_in_ms = (unsigned short) rate;
    }

    D("measure rate is set to %u ms", period_in_ms);

    time_sync = 0;
    if (property_get("persist.vendor.gps.time_sync", prop, "") != 0)
    {
        time_sync = atol(prop); // seconds drift threshold
    }

    D("time_sync is %s (threshold=%lds)",
      (time_sync > 0) ? "enabled" : "disabled", time_sync);

    // Disable echo on serial lines
    if ( isatty( state->fd ) ) {
        struct termios  ios;
        tcgetattr( state->fd, &ios );
        ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
        ios.c_oflag &= (~ONLCR); /* Stop \n -> \r\n translation on output */
        ios.c_iflag &= (~(ICRNL | INLCR)); /* Stop \r -> \n & \n -> \r translation on input */
        ios.c_iflag |= (IGNCR | IXOFF);  /* Ignore \r & XON/XOFF on input */
        // Set baud rate and other flags
        property_get("persist.vendor.gps.ttybaud", prop, "9600");
        if (strcmp(prop, "4800") == 0) {
            ALOGE("Setting gps baud rate to 4800");
            ios.c_cflag = B4800 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(prop, "9600") == 0) {
            ALOGE("Setting gps baud rate to 9600");
            ios.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(prop, "19200") == 0) {
            ALOGE("Setting gps baud rate to 19200");
            ios.c_cflag = B19200 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(prop, "38400") == 0) {
            ALOGE("Setting gps baud rate to 38400");
            ios.c_cflag = B38400 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(prop, "57600") == 0) {
            ALOGE("Setting gps baud rate to 57600");
            ios.c_cflag = B57600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(prop, "115200") == 0) {
            ALOGE("Setting gps baud rate to 115200");
            ios.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else {
            ALOGE("GPS baud rate unknown: '%s'", prop);
            return;
        }

        tcsetattr( state->fd, TCSANOW, &ios );
    }

    /* Apply u-blox M8 fast-fix + multi-GNSS + warm-start configuration */
    gps_dev_configure_ublox_m8(state->fd);

    gps_dev_set_meas_rate(state->fd, GPS_DEV_SLOW_UPDATE_RATE * 1000);

    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        ALOGE("Could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }

    state->thread = callbacks->create_thread_cb( "gps_state_thread", gps_state_thread, state );

    if ( !state->thread ) {
        ALOGE("Could not create GPS thread: %s", strerror(errno));
        goto Fail;
    }

    D("GPS state initialized");

    return;

Fail:
    gps_state_done( state );
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

static int
serial_gps_init(GpsCallbacks* callbacks)
{
    D("serial_gps_init");
    GpsState*  s = _gps_state;

    if (!s->init)
        gps_state_init(s, callbacks);

    if (s->fd < 0)
        return -1;

    return 0;
}

static void
serial_gps_cleanup(void)
{
    GpsState*  s = _gps_state;

    if (s->init)
        gps_state_done(s);
}

static int
serial_gps_start()
{
    GpsState*  s = _gps_state;

    if (!s->init) {
        DFR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_start(s);
    return 0;
}

static int
serial_gps_stop()
{
    GpsState*  s = _gps_state;

    if (!s->init) {
        DFR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_stop(s);
    return 0;
}

static int
serial_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    (void)time; (void)timeReference; (void)uncertainty;
    return 0;
}

static int
serial_gps_inject_location(double latitude, double longitude, float accuracy)
{
    (void)latitude; (void)longitude; (void)accuracy;
    return 0;
}

static void
serial_gps_delete_aiding_data(GpsAidingData flags)
{
    (void)flags;
}

static int serial_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
        uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
    GpsState*  s = _gps_state;

    if (!s->init) {
        D("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("set_position_mode: mode=%d recurrence=%d min_interval=%u preferred_accuracy=%u preferred_time=%u",
            mode, recurrence, min_interval, preferred_accuracy, preferred_time);

    return 0;
}

static const void*
serial_gps_get_extension(const char* name)
{
    (void)name;
    return NULL;
}

static const GpsInterface  serialGpsInterface = {
    sizeof(GpsInterface),
    serial_gps_init,
    serial_gps_start,
    serial_gps_stop,
    serial_gps_cleanup,
    serial_gps_inject_time,
    serial_gps_inject_location,
    serial_gps_delete_aiding_data,
    serial_gps_set_position_mode,
    serial_gps_get_extension,
};

const GpsInterface* gps_get_hardware_interface(struct gps_device_t* dev)
{
    D("GPS dev get_hardware_interface");
    (void)dev;
    return &serialGpsInterface;
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       D E V I C E                                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

static void gps_dev_send(int fd, char *msg, int size)
{
    int n = 0;
    do {

        int ret = write(fd, msg + n, size - n);

        if (ret < 0 && errno == EINTR) {
            continue;
        }

        n += ret;

    } while (n < size);
}

static void gps_dev_calc_ubx_csum(unsigned char *msg, int size, unsigned char *ck_a, unsigned char *ck_b)
{
    *ck_a = *ck_b = 0;
    for (int i = 0; i < size; ++i)
    {
        *ck_a += msg[i];
        *ck_b += *ck_a;
    }
}

static void gps_dev_set_meas_rate(int fd, unsigned short period_ms)
{
    // B5 62 06 08 06 00 F4 01 01 00 01 00 0B 77
    unsigned char buff[14] = "\xB5\x62\x06\x08\x06\x00";

    *((unsigned short *)(buff + 6)) = period_ms;
    *((unsigned short *)(buff + 8)) = 1;
    *((unsigned short *)(buff + 10)) = 1;

    gps_dev_calc_ubx_csum(buff + 2, 10, buff + 12, buff + 13);

    gps_dev_send(fd, (char *)buff, sizeof(buff));
}

static int open_gps(const struct hw_module_t* module, char const* name, struct hw_device_t** device)
{
    D("GPS dev open_gps");
    (void)name;
    struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->get_gps_interface = gps_get_hardware_interface;

    *device = &dev->common;
    return 0;
}

static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "Serial GPS Module",
    .author = "Keith Conger",
    .methods = &gps_module_methods,
};
