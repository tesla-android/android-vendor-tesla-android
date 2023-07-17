# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2023 Tesla Android Project

$(call inherit-product, vendor/tesla-android/vendor.mk)

$(call inherit-product, vendor/devices-community/radxa_zero/radxa_zero.mk)

PRODUCT_BOARD_PLATFORM := amlogic
PRODUCT_NAME := tesla_android_radxa_zero
PRODUCT_DEVICE := radxa_zero
PRODUCT_BRAND := radxa_zero
PRODUCT_MODEL := radxa_zero
PRODUCT_MANUFACTURER := radxa

PRODUCT_PROPERTY_OVERRIDES += \
    ro.tesla-android.device=radxa_zero \
    tesla-android.updater.uri=https://ota.teslaandroid.com/api/v1/radxa_zero/release \

GD_LCD_DENSITY = 200
