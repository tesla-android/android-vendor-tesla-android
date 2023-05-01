################################################################################

# OTA Updates

PRODUCT_PACKAGES += \
    Updater \

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.build.version=2023.18.0 \
    ro.tesla-android.releasetype=RELEASE \
    tesla-android.updater.allow_downgrading=true \

################################################################################

# Release keys

PRODUCT_DEFAULT_DEV_CERTIFICATE := $(LOCAL_PATH)/signing/releasekey
PRODUCT_OTA_PUBLIC_KEYS := $(LOCAL_PATH)/signing/platform
PRODUCT_EXTRA_RECOVERY_KEYS := $(LOCAL_PATH)/signing/platform

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
     $(LOCAL_PATH)/services/lighttpd/www/assets/AssetManifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/AssetManifest.json \
     $(LOCAL_PATH)/services/lighttpd/www/assets/FontManifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/FontManifest.json \
     $(LOCAL_PATH)/services/lighttpd/www/assets/NOTICES:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/NOTICES \
     $(LOCAL_PATH)/services/lighttpd/www/assets/fonts/MaterialIcons-Regular.otf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/fonts/MaterialIcons-Regular.otf \
     $(LOCAL_PATH)/services/lighttpd/www/assets/fonts/Roboto-Regular.ttf:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/fonts/Roboto-Regular.ttf \
     $(LOCAL_PATH)/services/lighttpd/www/assets/images/png/tesla-android-logo.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/assets/images/png/tesla-android-logo.png \
     $(LOCAL_PATH)/services/lighttpd/www/browserconfig.xml:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/browserconfig.xml \
     $(LOCAL_PATH)/services/lighttpd/www/canvaskit/canvaskit.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/canvaskit/canvaskit.js \
     $(LOCAL_PATH)/services/lighttpd/www/canvaskit/canvaskit.wasm:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/canvaskit/canvaskit.wasm \
     $(LOCAL_PATH)/services/lighttpd/www/canvaskit/profiling/canvaskit.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/canvaskit/profiling/canvaskit.js \
     $(LOCAL_PATH)/services/lighttpd/www/canvaskit/profiling/canvaskit.wasm:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/canvaskit/profiling/canvaskit.wasm \
     $(LOCAL_PATH)/services/lighttpd/www/favicon.ico:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/favicon.ico \
     $(LOCAL_PATH)/services/lighttpd/www/flutter.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/flutter.js \
     $(LOCAL_PATH)/services/lighttpd/www/flutter_service_worker.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/flutter_service_worker.js \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-192x192.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-192x192.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-36x36.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-36x36.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-48x48.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-48x48.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-72x72.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-72x72.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/android-icon-96x96.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/android-icon-96x96.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-114x114.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-114x114.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-120x120.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-120x120.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-152x152.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-152x152.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-180x180.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-180x180.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-57x57.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-57x57.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-60x60.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-60x60.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-72x72.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-72x72.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-76x76.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-76x76.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon-precomposed.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon-precomposed.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/apple-icon.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/apple-icon.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/favicon-16x16.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/favicon-16x16.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/favicon-32x32.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/favicon-32x32.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/favicon-96x96.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/favicon-96x96.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/manifest.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/manifest.json \
     $(LOCAL_PATH)/services/lighttpd/www/icons/ms-icon-144x144.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/ms-icon-144x144.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/ms-icon-150x150.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/ms-icon-150x150.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/ms-icon-310x310.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/ms-icon-310x310.png \
     $(LOCAL_PATH)/services/lighttpd/www/icons/ms-icon-70x70.png:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/icons/ms-icon-70x70.png \
     $(LOCAL_PATH)/services/lighttpd/www/index.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/index.html \
     $(LOCAL_PATH)/services/lighttpd/www/player.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/player.html \
     $(LOCAL_PATH)/services/lighttpd/www/main.dart.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/main.dart.js \
     $(LOCAL_PATH)/services/lighttpd/www/pcmplayer.js:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/pcmplayer.js \
     $(LOCAL_PATH)/services/lighttpd/www/online/status.html:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/online/status.html \
     $(LOCAL_PATH)/services/lighttpd/www/online/connectivity_check.txt:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/online/connectivity_check.txt \
     $(LOCAL_PATH)/services/lighttpd/www/version.json:$(TARGET_COPY_OUT_VENDOR)/tesla-android/www/version.json \

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
