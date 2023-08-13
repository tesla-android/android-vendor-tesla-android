/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <log/log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <time.h>

#include "gnss.h"
#include "gnss_configuration.h"
#include "gnss_measurement.h"
#include "agnss.h"

namespace {
constexpr char kGnssDeviceName[] = "Tesla Android GPS";
};

namespace goldfish {

sp<ahg20::IGnssCallback> goldfish::Gnss20::sGnssCallback = nullptr;

Return<sp<ahg20::IGnssConfiguration>> Gnss20::getExtensionGnssConfiguration_2_0() {
    return new GnssConfiguration20();
}

Return<sp<ahg20::IGnssDebug>> Gnss20::getExtensionGnssDebug_2_0() {
    return nullptr;
}

Return<sp<ahg20::IAGnss>> Gnss20::getExtensionAGnss_2_0() {
    return new AGnss20();
}

Return<sp<ahg20::IAGnssRil>> Gnss20::getExtensionAGnssRil_2_0() {
    return nullptr;
}

Return<sp<ahg20::IGnssMeasurement>> Gnss20::getExtensionGnssMeasurement_2_0() {
    return new GnssMeasurement20();
}

Return<bool> Gnss20::setCallback_2_0(const sp<ahg20::IGnssCallback>& callback) {
    if (callback == nullptr) {
        return false;
    } else {
        using Caps = ahg20::IGnssCallback::Capabilities;
        callback->gnssSetCapabilitiesCb_2_0(Caps::MEASUREMENTS | 0);
        callback->gnssNameCb(kGnssDeviceName);
        callback->gnssSetSystemInfoCb({.yearOfHw = 2023});
        sGnssCallback = callback;
        return true;
    }
}

Return<sp<ahgmc10::IMeasurementCorrections>> Gnss20::getExtensionMeasurementCorrections() {
    return nullptr;
}

Return<sp<ahgvc10::IGnssVisibilityControl>> Gnss20::getExtensionVisibilityControl() {
    return nullptr;
}

Return<sp<ahg20::IGnssBatching>> Gnss20::getExtensionGnssBatching_2_0() {
    return nullptr;
}

Return<bool> Gnss20::injectBestLocation_2_0(const ahg20::GnssLocation& location) {
    (void)location;
    return true;
}

Return<bool> Gnss20::setPositionMode_1_1(ahg10::IGnss::GnssPositionMode mode,
                                         ahg10::IGnss::GnssPositionRecurrence recurrence,
                                         uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
                                         uint32_t preferredTimeMs, bool lowPowerMode) {
    (void)mode;
    (void)recurrence;
    (void)minIntervalMs;
    (void)preferredAccuracyMeters;
    (void)preferredTimeMs;
    (void)lowPowerMode;
    return true;
}

Return<bool> Gnss20::start() {
    if (mIsActive) {
        ALOGW("Gnss has started. Restarting...");
        stop();
    }

    mIsActive = true;
    mThread = std::thread([this]() {
    	unsigned short flags =
                 ahg10::GnssLocationFlags::HAS_LAT_LONG |
                 ahg10::GnssLocationFlags::HAS_VERTICAL_ACCURACY |
                 ahg10::GnssLocationFlags::HAS_HORIZONTAL_ACCURACY;

        char latitudeDegreesString[PROP_VALUE_MAX];
        char longitudeDegreesString[PROP_VALUE_MAX];
        char verticalAccuracyMetersString[PROP_VALUE_MAX];
        char timestampString[PROP_VALUE_MAX];
        char speedString[PROP_VALUE_MAX];
        char bearingString[PROP_VALUE_MAX];

        ahg10::GnssLocation location;

        while (mIsActive == true) {
        	if (property_get("persist.tesla-android.gps.latitude", latitudeDegreesString, "") > 0
        		&& property_get("persist.tesla-android.gps.longitude", longitudeDegreesString, "") > 0
        		&& property_get("persist.tesla-android.gps.vertical_accuracy", verticalAccuracyMetersString, "") > 0
        		&& property_get("persist.tesla-android.gps.timestamp", timestampString, "") > 0) {

                double speedMetersPerSec = 0.0;
                if (property_get("persist.tesla-android.gps.speed", speedString, "") > 0
                    && std::string(speedString) != "not-available") {
                    speedMetersPerSec = atof(speedString);
                    flags |= ahg10::GnssLocationFlags::HAS_SPEED;
                }

                double bearingDegrees = 0.0;
                if (property_get("persist.tesla-android.gps.bearing", bearingString, "") > 0
                    && std::string(bearingString) != "not-available") {
                    bearingDegrees = atof(bearingString);
                    flags |= ahg10::GnssLocationFlags::HAS_BEARING;
                }

            	location = {
                     .gnssLocationFlags = flags,
                     .latitudeDegrees = atof(latitudeDegreesString),
                     .longitudeDegrees = atof(longitudeDegreesString),
                     .altitudeMeters = 0.0, // always 0 in Flutter Web
                     .speedMetersPerSec = static_cast<float>(speedMetersPerSec),
                     .bearingDegrees = static_cast<float>(bearingDegrees),
                     .horizontalAccuracyMeters = static_cast<float>(atof(verticalAccuracyMetersString)), // Required for Location object to be considered complete
		     .verticalAccuracyMeters = 0.0,
                     .speedAccuracyMetersPerSecond = 0.0, // always 0 in Flutter Web
                     .bearingAccuracyDegrees = 0.0, // always 0 in Flutter Web
                     .timestamp = atoll(timestampString),
            	};
           	this->reportLocation(location);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    return true;
}

Return<bool> Gnss20::stop() {
    mIsActive = false;
    if (mThread.joinable()) {
        mThread.join();
    }
    return true;
}

Return<void> Gnss20::cleanup() {
    return {};
}

Return<bool> Gnss20::injectTime(int64_t timeMs, int64_t timeReferenceMs,
                                int32_t uncertaintyMs) {
    (void)timeMs;
    (void)timeReferenceMs;
    (void)uncertaintyMs;
    return true;
}

Return<bool> Gnss20::injectLocation(double latitudeDegrees, double longitudeDegrees,
                                    float accuracyMeters) {
    (void)latitudeDegrees;
    (void)longitudeDegrees;
    (void)accuracyMeters;
    return false;
}

Return<void> Gnss20::deleteAidingData(ahg10::IGnss::GnssAidingData aidingDataFlags) {
    (void)aidingDataFlags;
    return {};
}

Return<sp<ahg10::IGnssGeofencing>> Gnss20::getExtensionGnssGeofencing() {
    return nullptr;
}

Return<sp<ahg10::IGnssNavigationMessage>> Gnss20::getExtensionGnssNavigationMessage() {
    return nullptr;
}

Return<sp<ahg10::IGnssXtra>> Gnss20::getExtensionXtra() {
    return nullptr;
}

//// deprecated and old versions ///////////////////////////////////////////////
Return<bool> Gnss20::setCallback_1_1(const sp<ahg11::IGnssCallback>&) {
    return false;
}

Return<sp<ahg11::IGnssMeasurement>> Gnss20::getExtensionGnssMeasurement_1_1() {
    return nullptr;
}

Return<sp<ahg11::IGnssConfiguration>> Gnss20::getExtensionGnssConfiguration_1_1() {
    return nullptr;
}

Return<bool> Gnss20::setCallback(const sp<ahg10::IGnssCallback>&) {
    return false;
}

Return<sp<ahg10::IGnssMeasurement>> Gnss20::getExtensionGnssMeasurement() {
    return nullptr;
}

Return<sp<ahg10::IAGnss>> Gnss20::getExtensionAGnss() {
    return nullptr;
}

Return<sp<ahg10::IGnssNi>> Gnss20::getExtensionGnssNi() {
    return nullptr;
}

Return<sp<ahg10::IGnssDebug>> Gnss20::getExtensionGnssDebug() {
    return nullptr;
}

Return<sp<ahg10::IGnssBatching>> Gnss20::getExtensionGnssBatching() {
    return nullptr;
}

Return<bool> Gnss20::injectBestLocation(const ahg10::GnssLocation&) {
    return false;
}

Return<bool> Gnss20::setPositionMode(ahg10::IGnss::GnssPositionMode,
                                     ahg10::IGnss::GnssPositionRecurrence,
                                     uint32_t, uint32_t, uint32_t) {
    return false;
}

Return<sp<ahg10::IGnssConfiguration>> Gnss20::getExtensionGnssConfiguration() {
    return nullptr;
}

Return<sp<ahg10::IAGnssRil>> Gnss20::getExtensionAGnssRil() {
    return nullptr;
}

Return<void> Gnss20::reportLocation(const ahg10::GnssLocation& location) const {
    std::unique_lock<std::mutex> lock(mMutex);
    if (sGnssCallback == nullptr) {
        ALOGE("%s: sGnssCallback is null.", __func__);
        return {};
    }
    sGnssCallback->gnssLocationCb(location);
    return {};
}

}  // namespace goldfish
