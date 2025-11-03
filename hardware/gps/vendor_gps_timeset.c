// Minimal helper to set system time from epoch seconds.
// Build as a vendor binary and run with CAP_SYS_TIME from init.
// Usage:
//   vendor_gps_timeset <epoch_seconds>
//   vendor_gps_timeset @<epoch_seconds>   // also accepted (toybox/date style)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

static void usage(const char* prog) {
    fprintf(stderr, "usage: %s <epoch_seconds>\n", prog);
}

static int64_t parse_epoch_arg(const char* arg) {
    if (!arg || !*arg) return -1;
    if (arg[0] == '@') arg++;
    char* end = NULL;
    errno = 0;
    long long v = strtoll(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0') return -1;
    if (v < 0) return -1;
    return (int64_t)v;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    int64_t epoch_s = parse_epoch_arg(argv[1]);
    if (epoch_s < 0) {
        fprintf(stderr, "invalid epoch seconds: '%s'\n", argv[1]);
        return 2;
    }

    struct timespec ts = { .tv_sec = (time_t)epoch_s, .tv_nsec = 0 };
    if (clock_settime(CLOCK_REALTIME, &ts) != 0) {
        fprintf(stderr, "clock_settime failed: %d (%s)\n", errno, strerror(errno));
        return 1;
    }

    return 0;
}

