#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

#include "config.h"

Preferences myPrefs;

static const int16_t CURRENT_VERSION = 1;

static const char *PrefsNamespace = "configuration";
static const char *PrefInitDoneVersion = "idv";

void configSetup()
{
    myPrefs.begin(PrefsNamespace, false);
    // detect new device
    if (!myPrefs.isKey(PrefInitDoneVersion))
    {
        myPrefs.putShort(PrefInitDoneVersion, CURRENT_VERSION);
    }

    // preparation only as of yet: migrate to newer data structure
    switch (myPrefs.getShort(PrefInitDoneVersion))
    {
    case CURRENT_VERSION:
        // no migration needed
        break;
    default:
        break;
    }
}

void configLoop()
{
    // do nothing (yet)
}

void factoryReset()
{
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init();  // initialize the NVS partition.
    ESP.restart();     // reboot
}

// ToDo:
// 1) move all sanity checks for values to be saved out of this file
// 2) call myPrefs.putXXX only when the value differs from the stored value

uint8_t brightnessStep() { return myPrefs.getUChar(PrefBrightnessStep, DEFAULT_BRIGHTNESS_STEP); }
void setBrightnessStep(uint8_t value)
{
    if (value == 0)
        value = 1;
    myPrefs.putUChar(PrefBrightnessStep, value);
}

uint16_t transitionDurationMs() { return myPrefs.getUShort(PrefTransitionDurationMs, DEFAULT_TRANSITION_DURATION_MS); }
void setTransitionDurationMs(uint16_t value) { myPrefs.putUShort(PrefTransitionDurationMs, value); }

uint8_t maxBrightness() { return myPrefs.getUChar(PrefMaxBrightness, DEFAULT_MAX_BRIGHTNESS); }
void setMaxBrightness(uint8_t value) { myPrefs.putUChar(PrefMaxBrightness, value); }

uint8_t onBrightness() { return myPrefs.getUChar(PrefOnBrightness, DEFAULT_ON_BRIGHTNESS); }
void setOnBrightness(uint8_t value) { myPrefs.putUChar(PrefOnBrightness, value); }

uint8_t nightLightBrightness() { return myPrefs.getUChar(PrefNightLightBrightness, DEFAULT_NIGHTLIGHT_BRIGHTNESS); }
void setNightLightBrightness(uint8_t value) { myPrefs.putUChar(PrefNightLightBrightness, value); }

uint8_t maxNightLightBrightness() { return myPrefs.getUChar(PrefMaxNightLightBrightness, DEFAULT_MAX_NIGHTLIGHT_BRIGHTNESS); }
void setMaxNightLightBrightness(uint8_t value) { myPrefs.putUChar(PrefMaxNightLightBrightness, value); }

bool allowNightLight() { return myPrefs.getBool(PrefAllowNightLight, DEFAULT_ALLOW_NIGHTLIGHT); }
void setAllowNightLight(bool value) { myPrefs.putBool(PrefAllowNightLight, value); }

uint16_t nightLightOnDuration() { return myPrefs.getUShort(PrefNightLightOnDuration, DEFAULT_NIGHTLIGHT_ON_DURATION_S); }
void setNightLightOnDuration(uint16_t value) { myPrefs.putUShort(PrefNightLightOnDuration, value); }

uint16_t nightLightThreshold() { return myPrefs.getUShort(PrefNightLightLdrThreshold, DEFAULT_LDR_NIGHTLIGHT_THRESHOLD); }
void setNightLightThreshold(uint16_t value) { myPrefs.putUShort(PrefNightLightLdrThreshold, value); }

uint16_t minMovingTargetDistance() { return myPrefs.getUShort(PrefMinMovingTargetDistance, DEFAULT_MIN_MOVING_TARGET_DISTANCE); }
void setMinMovingTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMinMovingTargetDistance, value); }

uint16_t maxMovingTargetDistance() { return myPrefs.getUShort(PrefMaxMovingTargetDistance, DEFAULT_MAX_MOVING_TARGET_DISTANCE); }
void setMaxMovingTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMaxMovingTargetDistance, value); }

uint8_t minMovingTargetEnergy() { return myPrefs.getUChar(PrefMinMovingTargetEnergy, DEFAULT_MIN_MOVING_TARGET_ENERGY); }
void setMinMovingTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMinMovingTargetEnergy, value); }

uint8_t maxMovingTargetEnergy() { return myPrefs.getUChar(PrefMaxMovingTargetEnergy, DEFAULT_MAX_MOVING_TARGET_ENERGY); }
void setMaxMovingTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMaxMovingTargetEnergy, value); }

uint16_t minStationaryTargetDistance() { return myPrefs.getUShort(PrefMinStationaryTargetDistance, DEFAULT_MIN_STATIONARY_TARGET_DISTANCE); }
void setMinStationaryTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMinStationaryTargetDistance, value); }

uint16_t maxStationaryTargetDistance() { return myPrefs.getUShort(PrefMaxStationaryTargetDistance, DEFAULT_MAX_STATIONARY_TARGET_DISTANCE); }
void setMaxStationaryTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMaxStationaryTargetDistance, value); }

uint8_t minStationaryTargetEnergy() { return myPrefs.getUChar(PrefMinStationaryTargetEnergy, DEFAULT_MIN_STATIONARY_TARGET_ENERGY); }
void setMinStationaryTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMinStationaryTargetEnergy, value); }

uint8_t maxStationaryTargetEnergy() { return myPrefs.getUChar(PrefMaxStationaryTargetEnergy, DEFAULT_MAX_STATIONARY_TARGET_ENERGY); }
void setMaxStationaryTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMaxStationaryTargetEnergy, value); }

String getWifiHostname() { return myPrefs.getString(PrefWifiHostname, DEFAULT_WIFI_HOSTNAME); }
void setWifiHostname(const String &value) { myPrefs.putString(PrefWifiHostname, value); }

String getWifiApSsid() { return myPrefs.getString(PrefWifiApSsid, DEFAULT_WIFI_AP_SSID); }
void setWifiApSsid(const String &value) { myPrefs.putString(PrefWifiApSsid, value); }

String getWifiApPassphrase() { return myPrefs.getString(PrefWifiApPassphrase, DEFAULT_WIFI_AP_PASSPHRASE); }
void setWifiApPassphrase(const String &value) { myPrefs.putString(PrefWifiApPassphrase, value); }

uint32_t wifiApIPv4Address() { return myPrefs.getLong(PrefWifiApIpAddress, DEFAULT_WIFI_AP_IP); }
void setWifiAPpIPv4Address(uint32_t value) { myPrefs.putLong(PrefWifiApIpAddress, value); }

uint32_t wifiApIPv4Netmask() { return myPrefs.getLong(PrefWifiApNetmask, DEFAULT_WIFI_AP_NETMASK); }
void setWifiAPpIPv4Netmask(uint32_t value) { myPrefs.putLong(PrefWifiApNetmask, value); }

String getWifiStaSsid() { return myPrefs.getString(PrefWifiStaSsid, DEFAULT_WIFI_STA_SSID); }
void setWifiStaSsid(const String &value) { myPrefs.putString(PrefWifiStaSsid, value); }

String getWifiStaPassphrase() { return myPrefs.getString(PrefWifiStaPassphrase, DEFAULT_WIFI_STA_PASSPHRASE); }
void setWifiStaPassphrase(const String &value) { myPrefs.putString(PrefWifiStaPassphrase, value); }

String getWebAuthPassword() { return myPrefs.getString(PrefWebAuthPassword, DEFAULT_WEB_AUTH_PASSWORD); }
void setWebAuthPassword(const String &value) { myPrefs.putString(PrefWebAuthPassword, value); }

String getWebAuthUsername() { return myPrefs.getString(PrefWebAuthUsername, DEFAULT_WEB_AUTH_USERNAME); }
void setWebAuthUsername(const String &value) { myPrefs.putString(PrefWebAuthUsername, value); }
