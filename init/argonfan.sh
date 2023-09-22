#!/system/bin/sh
# this script needs to run once the system is fully booted (and not before)
#
# sleeptime is the interval (in seconds) between fan speed adjustments based on temperature
sleeptime=60
insmod /vendor_dlkm/lib/modules/kernel/drivers/i2c/i2c-dev.ko
argon=$(i2cdetect -y 1 | grep -F '10:' | awk '{print $12}')
if [ "$argon" != "1a" ]; then
    exit 1
fi
# an argon fan was detected so now adjust it periodically
while true
do
    cputemp=$(cat /sys/class/thermal/thermal_zone0/temp)
    if [ $cputemp -lt 38000 ]; then
        fanspeed=0x00
    elif [ $cputemp -ge 38000 -a $cputemp -lt 55000 ]; then
        fanspeed=0x10
    elif [ $cputemp -ge 55000 -a $cputemp -lt 65000 ]; then
        fanspeed=0x32
    elif [ $cputemp -ge 65000 ]; then
        fanspeed=0x64
    fi
    i2cget -y 1 0x01a $fanspeed > /dev/null
    # logging high temperatures (uncomment to enable)
    #if [ $cputemp -ge 55000 ]; then
        #[[ -f /sdcard/Fancontrol/fanlog.txt ]] && echo $(date +"%d-%m-%y %H:%M:%S") $cputemp $fanspeed >> /sdcard/Fancontrol/fanlog.txt
    #fi
    sleep $sleeptime
done
