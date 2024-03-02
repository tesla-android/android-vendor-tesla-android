# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2022 Tesla Android Project

$(call inherit-product, vendor/tesla-android/vendor.mk)

$(call inherit-product, vendor/devices-community/gd_cm4/gd_cm4.mk)

PRODUCT_BOARD_PLATFORM := broadcom
PRODUCT_NAME := tesla_android_cm4
PRODUCT_DEVICE := gd_cm4
PRODUCT_BRAND := RaspberryPI
PRODUCT_MODEL := cm4
PRODUCT_MANUFACTURER := RaspberryPiFoundation

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.device=cm4 \
    persist.tesla-android.updater.uri=https://ota.teslaandroid.com/api/v1/cm4/release \

GD_LCD_DENSITY = 200
