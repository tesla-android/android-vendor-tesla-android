# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2022 Tesla Android Project
#

PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/tesla_android_rpi4.mk \
    $(LOCAL_DIR)/tesla_android_radxa_zero.mk

COMMON_LUNCH_CHOICES := \
    tesla_android_rpi4-user \
    tesla_android_rpi4-userdebug \
    tesla_android_rpi4-eng \
    tesla_android_radxa_zero-user \
    tesla_android_radxa_zero-userdebug \
    tesla_android_radxa_zero-eng

