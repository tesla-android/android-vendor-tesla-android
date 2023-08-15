################################################################################

# Version

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.build.version=2023.32.1 \

################################################################################

# Flavor and signing

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.releasetype=RELEASE \

PRODUCT_DEFAULT_DEV_CERTIFICATE := $(LOCAL_PATH)/signing/releasekey
PRODUCT_OTA_PUBLIC_KEYS := $(LOCAL_PATH)/signing/platform
PRODUCT_EXTRA_RECOVERY_KEYS := $(LOCAL_PATH)/signing/platform

################################################################################

# OTA Updates

PRODUCT_PACKAGES += \
    Updater \

PRODUCT_PROPERTY_OVERRIDES += \
    tesla-android.updater.allow_downgrading=true \

GD_SUPER_PARTITION_SIZE_MB := 7500

################################################################################

# Configuration manager

PRODUCT_PACKAGES += \
    tesla-android-configuration-manager \

# Renderer and isH264 are unused now 

PRODUCT_PROPERTY_OVERRIDES += \
    persist.tesla-android.softap.band_type=2 \
    persist.tesla-android.softap.channel=44 \
    persist.tesla-android.softap.channel_width=3 \
    persist.tesla-android.softap.is_enabled=1 \
    persist.tesla-android.offline-mode.is_enabled=1 \
    persist.tesla-android.offline-mode.telemetry.is_enabled=1 \
    persist.tesla-android.offline-mode.tesla-firmware-downloads=1 \
    persist.tesla-android.virtual-display.resolution.width=1088 \
    persist.tesla-android.virtual-display.resolution.height=832 \
    persist.tesla-android.virtual-display.density=200 \
    persist.tesla-android.virtual-display.lowres=0 \
    persist.tesla-android.virtual-display.renderer=0 \
    persist.drm_hwc.headless.config="1088x832@30" \
    persist.tesla-android.browser_audio.is_enabled=0 \
    persist.tesla-android.browser_audio.volume=100 \
    persist.tesla-android.virtual-display.is_responsive=0 \
    persist.tesla-android.virtual-display.is_h264=0 \

################################################################################

# GPS

PRODUCT_PACKAGES += \
    android.hardware.gnss@2.0-service.tesla-android \
    tesla-android-virtual-gnss

BOARD_SEPOLICY_DIRS += vendor/tesla-android/hardware/gnss/sepolicy

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \

################################################################################

# Screen orientation lock

PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.screen.landscape.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.screen.landscape.xml \

PRODUCT_PROPERTY_OVERRIDES += \
    config.override_forced_orient=false \
    ro.sf.hwrotation=0 \
    persist.demo.hdmirotation=landscape \
    persist.demo.rotationlock=true \
    persist.demo.remoterotation=landscape \

################################################################################

# Prebuilt applications

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/applications/preinstall.sh:$(TARGET_COPY_OUT_VENDOR)/etc/preinstall/preinstall.sh \
    $(LOCAL_PATH)/applications/autokit/AutoKit.apk:$(TARGET_COPY_OUT_VENDOR)/etc/preinstall/AutoKit.apk_ \

PRODUCT_PACKAGES += \
    AutoKit \

################################################################################

# Google Apps

$(call inherit-product, vendor/gapps/arm64/arm64-vendor.mk) 

################################################################################

# Virtual touchscreen

PRODUCT_PACKAGES += \
	tesla-android-virtual-touchscreen \

################################################################################

# Virtual display

PRODUCT_PACKAGES += \
    tesla-android-virtual-display \

################################################################################

# ih8sn

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/services/ih8sn/addon.d/60-ih8sn.sh:$(TARGET_COPY_OUT_SYSTEM)/addon.d/60-ih8sn.sh \
    $(LOCAL_PATH)/services/ih8sn/bin/ih8sn:$(TARGET_COPY_OUT_SYSTEM)/bin/ih8sn \
    $(LOCAL_PATH)/services/ih8sn/etc/init/ih8sn.rc:$(TARGET_COPY_OUT_SYSTEM)/etc/init/ih8sn.rc \
    $(LOCAL_PATH)/services/ih8sn/etc/ih8sn.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/ih8sn.conf \

################################################################################

# LineageOS browser

PRODUCT_PACKAGES += \
    Jelly \

################################################################################

# Modems

PRODUCT_PACKAGES += \
    usb_modeswitch \
    tesla-android-usb-networking-initialiser \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/services/usb_modeswitch/12d1.conf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/usb_modeswitch/12d1.conf \
    $(LOCAL_PATH)/services/usb_modeswitch/1bbb-f000.conf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/usb_modeswitch/1bbb-f000.conf \

################################################################################

# v4l2

PRODUCT_PACKAGES += \
    v4l2-dbg \
    v4l2-compliance \
    v4l2-ctl \

################################################################################

# Tablet mode

PRODUCT_CHARACTERISTICS := tablet

################################################################################

# lighttpd

PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/services/lighttpd/lighttpd:$(TARGET_COPY_OUT_VENDOR)/bin/lighttpd \
     $(LOCAL_PATH)/services/lighttpd/lighttpd.conf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/lighttpd.conf \
     $(LOCAL_PATH)/services/lighttpd/certificates/device.teslaandroid.com/fullchain.pem:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/certificates/device.teslaandroid.com/fullchain.pem \
     $(LOCAL_PATH)/services/lighttpd/certificates/device.teslaandroid.com/privkey.pem:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/certificates/device.teslaandroid.com/privkey.pem \
     $(LOCAL_PATH)/services/lighttpd/certificates/fullscreen.device.teslaandroid.com/fullchain.pem:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/certificates/fullscreen.device.teslaandroid.com/fullchain.pem \
     $(LOCAL_PATH)/services/lighttpd/certificates/fullscreen.device.teslaandroid.com/privkey.pem:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/certificates/fullscreen.device.teslaandroid.com/privkey.pem \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/AssetManifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/AssetManifest.json \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/FontManifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/FontManifest.json \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/NOTICES:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/NOTICES \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/fonts/MaterialIcons-Regular.otf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/fonts/MaterialIcons-Regular.otf \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/fonts/Roboto-Regular.ttf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/fonts/Roboto-Regular.ttf \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/images/png/tesla-android-logo.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/images/png/tesla-android-logo.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/browserconfig.xml:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//browserconfig.xml \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/canvaskit.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/canvaskit.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/canvaskit.wasm:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/canvaskit.wasm \
     $(LOCAL_PATH)/services/lighttpd/www-default/favicon.ico:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//favicon.ico \
     $(LOCAL_PATH)/services/lighttpd/www-default/flutter.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//flutter.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/flutter_service_worker.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//flutter_service_worker.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-192x192.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-192x192.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-36x36.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-36x36.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-48x48.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-48x48.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-72x72.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-72x72.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/android-icon-96x96.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/android-icon-96x96.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-114x114.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-114x114.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-120x120.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-120x120.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-152x152.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-152x152.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-180x180.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-180x180.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-57x57.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-57x57.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-60x60.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-60x60.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-72x72.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-72x72.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-76x76.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-76x76.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon-precomposed.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon-precomposed.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/apple-icon.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/apple-icon.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/favicon-16x16.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/favicon-16x16.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/favicon-32x32.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/favicon-32x32.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/favicon-96x96.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/favicon-96x96.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/manifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/manifest.json \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/ms-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/ms-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/ms-icon-150x150.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/ms-icon-150x150.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/ms-icon-310x310.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/ms-icon-310x310.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/icons/ms-icon-70x70.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//icons/ms-icon-70x70.png \
     $(LOCAL_PATH)/services/lighttpd/www-default/index.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//index.html \
     $(LOCAL_PATH)/services/lighttpd/www-default/main.dart.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//main.dart.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/main.dart.js.map:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//main.dart.js.map \
     $(LOCAL_PATH)/services/lighttpd/www-default/online/connectivity_check.txt:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//online/connectivity_check.txt \
     $(LOCAL_PATH)/services/lighttpd/www-default/online/status.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//online/status.html \
     $(LOCAL_PATH)/services/lighttpd/www-default/pcmplayer.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//pcmplayer.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/version.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//version.json \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/AssetManifest.bin:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/AssetManifest.bin \
     $(LOCAL_PATH)/services/lighttpd/www-default/assets/shaders/ink_sparkle.frag:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//assets/shaders/ink_sparkle.frag \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/chromium/canvaskit.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/chromium/canvaskit.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/chromium/canvaskit.wasm:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/chromium/canvaskit.wasm \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/skwasm.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/skwasm.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/skwasm.wasm:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/skwasm.wasm \
     $(LOCAL_PATH)/services/lighttpd/www-default/canvaskit/skwasm.worker.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default//canvaskit/skwasm.worker.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/h264.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/h264.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/h264_worker.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/h264_worker.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/webGLWebCodecs.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/webGLWebCodecs.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/workerWebGLWebCodecs.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/workerWebGLWebCodecs.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/imgTag.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/imgTag.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/android.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/android.html \
     $(LOCAL_PATH)/services/lighttpd/www-default/estimator.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/estimator.js \
     $(LOCAL_PATH)/services/lighttpd/www-default/reconnecting-websocket.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-default/reconnecting-websocket.js \
     $(LOCAL_PATH)/services/lighttpd/www-redirect/index.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/lighttpd/www-redirect/index.html \

################################################################################

# Overlays

PRODUCT_PACKAGES += \
    tesla_android_overlay_frameworks_base_core \
    tesla_android_overlay_settings_provider \
    tesla_android_overlay_systemui \
    tesla_android_overlay_service_wifi_resources \

################################################################################

# Init

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init/init.tesla-android.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.tesla-android.rc \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/services/iosTethering/bin/usbmuxd:$(TARGET_COPY_OUT_SYSTEM)/bin/usbmuxd \
    $(LOCAL_PATH)/services/iosTethering/lib/libusb-1.0.so:$(TARGET_COPY_OUT_SYSTEM)/lib/libusb-1.0.so \
    $(LOCAL_PATH)/services/iosTethering/lib/libimobiledevice-1.0.so:$(TARGET_COPY_OUT_SYSTEM)/lib/libimobiledevice-1.0.so \
    $(LOCAL_PATH)/services/iosTethering/lib/libplist-2.0.so:$(TARGET_COPY_OUT_SYSTEM)/lib/libplist-2.0.so \
    $(LOCAL_PATH)/services/iosTethering/lib/libssl.so.1.1:$(TARGET_COPY_OUT_SYSTEM)/lib/libssl.so.1.1 \
    $(LOCAL_PATH)/services/iosTethering/lib/libcrypto.so.1.1:$(TARGET_COPY_OUT_SYSTEM)/lib/libcrypto.so.1.1 \
    $(LOCAL_PATH)/services/iosTethering/lib/libusbmuxd-2.0.so:$(TARGET_COPY_OUT_SYSTEM)/lib/libusbmuxd-2.0.so \
