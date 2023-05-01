# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2022 Tesla Android Project

$(call inherit-product, vendor/tesla-android/vendor.mk)

$(call inherit-product, vendor/devices-community/gd_rpi4/gd_rpi4.mk)

PRODUCT_BOARD_PLATFORM := broadcom
PRODUCT_NAME := tesla_android_rpi4
PRODUCT_DEVICE := gd_rpi4
PRODUCT_BRAND := RaspberryPI
PRODUCT_MODEL := rpi4
PRODUCT_MANUFACTURER := RaspberryPiFoundation

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.device=rpi4 \
    tesla-android.updater.uri=https://ota.teslaandroid.com/api/v1/rpi4/release \

GD_LCD_DENSITY = 200
