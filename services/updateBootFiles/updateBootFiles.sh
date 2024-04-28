#!/system/bin/sh

bootFilesVersion=$(getprop persist.tesla_android.bootFilesVersion)
echo ${bootFilesVersion}
if [ "${bootFilesVersion}" != "1" ]; then
   cp /vendor_dlkm/boot/bootcode.bin /bootloader/bootcode.bin
   cp /vendor_dlkm/boot/start_x.elf /bootloader/start_x.elf
   cp /vendor_dlkm/boot/start4x.elf /bootloader/start4x.elf
   cp /vendor_dlkm/boot/fixup_x.dat /bootloader/fixup_x.dat
   cp /vendor_dlkm/boot/fixup4x.dat /bootloader/fixup4x.dat
   cp /vendor_dlkm/boot/bcm2711-rpi-4-b.dtb /bootloader/bcm2711-rpi-4-b.dtb
   cp /vendor_dlkm/boot/bcm2711-rpi-400.dtb /bootloader/bcm2711-rpi-400.dtb
   cp /vendor_dlkm/boot/bcm2711-rpi-cm4.dtb /bootloader/bcm2711-rpi-cm4.dtb
   setprop persist.tesla_android.bootFilesVersion 1
   reboot
fi
