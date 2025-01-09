# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2022 Tesla Android Project
#

PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/tesla_android_rpi4.mk \
    $(LOCAL_DIR)/tesla_android_cm4.mk \
    $(LOCAL_DIR)/tesla_android_radxa_zero.mk

COMMON_LUNCH_CHOICES := \
    tesla_android_rpi4-trunk_staging-user \
    tesla_android_rpi4-trunk_staging-userdebug \
    tesla_android_rpi4-trunk_staging-eng \
    tesla_android_cm4-trunk_staging-user \
    tesla_android_cm4-trunk_staging-userdebug \
    tesla_android_cm4-trunk_staging-eng \
    tesla_android_radxa_zero-trunk_staging-user \
    tesla_android_radxa_zero-trunk_staging-userdebug \
    tesla_android_radxa_zero-trunk_staging-eng

