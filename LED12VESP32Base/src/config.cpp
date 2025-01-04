#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

Preferences myPrefs;

static const int16_t CURRENT_VERSION = 1;

static const char *PrefsNamespace = "configuration";

static const char *PrefInitDoneVersion = "idv";
static const char *PrefStepBrightness = "stbr";
static const char *PrefTransitionDurationMs = "ptdm";
static const char *PrefMaxBrightness = "mbr";
static const char *PrefOnBrightness = "obr";
static const char *PrefOnDuration = "odu";
static const char *PrefNightLightBrightness = "nlbr";
static const char *PrefAllowNightLight = "alnl";
static const char *PrefNightLightLdrThreshold = "nllt";
static const char *PrefRestoreAfterPowerloss = "rapl";
static const char *PrefMinMovingTargetDistance = "mimd";
static const char *PrefMaxMovingTargetDistance = "msmd";
static const char *PrefMinStationaryTargetDistance = "misd";
static const char *PrefMaxStationaryTargetDistance = "mssd";
static const char *PrefMinMovingTargetEnergy = "mime";
static const char *PrefMaxMovingTargetEnergy = "msme";
static const char *PrefMinStationaryTargetEnergy = "mise";
static const char *PrefMaxStationaryTargetEnergy = "msse";
static const char *PrefSavedState = "sast";

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

uint8_t stepBrightness() { return myPrefs.getUChar(PrefStepBrightness, DEFAULT_BRIGHTNESS_STEP); }
void setStepBrightness(uint8_t value)
{
    if (value > 0)
        myPrefs.putUChar(PrefStepBrightness, value);
}

uint16_t transitionDurationMs() { return myPrefs.getUShort(PrefTransitionDurationMs, DEFAULT_TRANSITION_DURATION_MS); }
void setTransitionDurationMs(uint16_t value) { myPrefs.putUShort(PrefTransitionDurationMs, value); }

uint8_t maxBrightness() { return myPrefs.getUChar(PrefMaxBrightness, DEFAULT_BRIGHTNESS_UPPER_LIMIT); }
void setMaxBrightness(uint8_t value) { myPrefs.putUChar(PrefMaxBrightness, value); }

uint8_t onBrightness() { return myPrefs.getUChar(PrefOnBrightness, DEFAULT_ON_BRIGHTNESS); }
void setOnBrightness(uint8_t value) { myPrefs.putUChar(PrefOnBrightness, value); }

uint8_t nightLightBrightness() { return myPrefs.getUChar(PrefNightLightBrightness, DEFAULT_NIGHTLIGHT_BRIGHTNESS); }
void setNightLightBrightness(uint8_t value) { myPrefs.putUChar(PrefNightLightBrightness, value); }

bool allowNightLight() { return myPrefs.getBool(PrefAllowNightLight, DEFAULT_ALLOW_NIGHTLIGHT); }
void setAllowNightLight(bool value) { myPrefs.putBool(PrefAllowNightLight, value); }

uint16_t nightLightOnDuration() { return myPrefs.getUShort(PrefOnDuration, DEFAULT_NIGHTLIGHT_ON_DURATION_S); }
void setNightLightOnDuration(uint16_t value) { myPrefs.putUShort(PrefOnDuration, value); }

uint16_t nightLightThreshold() { return myPrefs.getUShort(PrefNightLightLdrThreshold, DEFAULT_LDR_NIGHTLIGHT_THRESHOLD); }
void setNightLightThreshold(uint16_t value) { myPrefs.putUShort(PrefNightLightLdrThreshold, value); }

bool restoreAfterPowerloss() { return myPrefs.getBool(PrefRestoreAfterPowerloss, DEFAULT_RESTORE_STATE_AFTER_POWERLOSS); }
void setRestoreAfterPowerloss(bool value) { myPrefs.putBool(PrefRestoreAfterPowerloss, value); }

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