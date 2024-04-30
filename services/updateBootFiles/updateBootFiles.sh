#!/system/bin/sh

bootFilesVersion=$(getprop persist.tesla_android.bootFilesVersion)
echo ${bootFilesVersion}
if [ "${bootFilesVersion}" != "1" ]; then
   mount /dev/block/mmcblk0p1 /data/vendor/tesla-android/bootFiles
   cp /vendor_dlkm/boot/bootcode.bin /data/vendor/tesla-android/bootFiles/bootcode.bin
   cp /vendor_dlkm/boot/start_x.elf /data/vendor/tesla-android/bootFiles/start_x.elf
   cp /vendor_dlkm/boot/start4x.elf /data/vendor/tesla-android/bootFiles/start4x.elf
   cp /vendor_dlkm/boot/fixup_x.dat /data/vendor/tesla-android/bootFiles/fixup_x.dat
   cp /vendor_dlkm/boot/fixup4x.dat /data/vendor/tesla-android/bootFiles/fixup4x.dat
   cp /vendor_dlkm/boot/bcm2711-rpi-4-b.dtb /data/vendor/tesla-android/bootFiles/bcm2711-rpi-4-b.dtb
   cp /vendor_dlkm/boot/bcm2711-rpi-400.dtb /data/vendor/tesla-android/bootFiles/bcm2711-rpi-400.dtb
   cp /vendor_dlkm/boot/bcm2711-rpi-cm4.dtb /data/vendor/tesla-android/bootFiles/bcm2711-rpi-cm4.dtb
   setprop persist.tesla_android.bootFilesVersion 1
   reboot
fi
