# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2022 Tesla Android Project

$(call inherit-product, vendor/tesla-android/vendor.mk)

$(call inherit-product, device/glodroid/rpi4/rpi4.mk)

PRODUCT_BOARD_PLATFORM := broadcom
PRODUCT_NAME := tesla_android_rpi4
PRODUCT_DEVICE := rpi4
PRODUCT_BRAND := RaspberryPI
PRODUCT_MODEL := rpi4
PRODUCT_MANUFACTURER := RaspberryPiFoundation