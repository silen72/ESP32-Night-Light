#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

Preferences myPrefs;

static const char *PrefsNamespace = "configuration";

static const char *PrefInitDoneVersion = "idv";
static const char *PrefStepBrightness = "stbr";
static const char *PrefTransitionDurationMs = "ptdm";
static const char *PrefMaxBrightness = "mbr";
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

void configSetup()
{
    myPrefs.begin(PrefsNamespace, false);
    if (!myPrefs.isKey(PrefInitDoneVersion))
    {
        myPrefs.putShort(PrefInitDoneVersion, 1);
        setStepBrightness(8);
        setTransitionDurationMs(1000);
        setMaxBrightness(210);
        setNightLightBrightness(16);
        setAllowNightLight(true);
        setNightLightThreshold(30);
        setRestoreAfterPowerloss(true);
        setMinMovingTargetDistance(0);
        setMaxMovingTargetDistance(UINT16_MAX);
        setMinStationaryTargetDistance(0);
        setMaxStationaryTargetDistance(UINT16_MAX);
        setMinMovingTargetEnergy(0);
        setMaxMovingTargetEnergy(UINT8_MAX);
        setMinStationaryTargetEnergy(0);
        setMaxStationaryTargetEnergy(UINT8_MAX);
    }
}

void configLoop()
{
    // do nothing (yet)
}

uint8_t stepBrightness() { return myPrefs.getUChar(PrefStepBrightness); }
void setStepBrightness(uint8_t value) { myPrefs.putUChar(PrefStepBrightness, value); }

uint16_t transitionDurationMs() { return myPrefs.getUShort(PrefTransitionDurationMs); }
void setTransitionDurationMs(uint16_t value) { myPrefs.putUShort(PrefTransitionDurationMs, value); }

uint8_t maxBrightness() { return myPrefs.getUChar(PrefMaxBrightness); }
void setMaxBrightness(uint8_t value) { myPrefs.putUChar(PrefMaxBrightness, value); }

uint8_t nightLightBrightness() { return myPrefs.getUChar(PrefNightLightBrightness); }
void setNightLightBrightness(uint8_t value) { myPrefs.putUChar(PrefNightLightBrightness, value); }

bool allowNightLight() { return myPrefs.getBool(PrefAllowNightLight); }
void setAllowNightLight(bool value) { myPrefs.putBool(PrefAllowNightLight, value); }

uint16_t nightLightThreshold() { return myPrefs.getUShort(PrefNightLightLdrThreshold); }
void setNightLightThreshold(uint16_t value) { myPrefs.putUShort(PrefNightLightLdrThreshold, value); }

bool restoreAfterPowerloss() { return myPrefs.getBool(PrefRestoreAfterPowerloss);}
void setRestoreAfterPowerloss(bool value) { myPrefs.putBool(PrefRestoreAfterPowerloss, value); }

uint16_t minStationaryTargetDistance() { return myPrefs.getUShort(PrefMinStationaryTargetDistance); }
void setMinStationaryTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMinStationaryTargetDistance, value); }

uint16_t maxStationaryTargetDistance() { return myPrefs.getUShort(PrefMaxStationaryTargetDistance); }
void setMaxStationaryTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMaxStationaryTargetDistance, value); }

uint8_t minStationaryTargetEnergy() { return myPrefs.getUChar(PrefMinStationaryTargetEnergy); }
void setMinStationaryTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMinStationaryTargetEnergy, value); }

uint8_t maxStationaryTargetEnergy() { return myPrefs.getUChar(PrefMaxStationaryTargetEnergy); }
void setMaxStationaryTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMaxStationaryTargetEnergy, value); }

uint16_t minMovingTargetDistance() { return myPrefs.getUShort(PrefMinMovingTargetDistance); }
void setMinMovingTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMinMovingTargetDistance, value); }

uint16_t maxMovingTargetDistance() { return myPrefs.getUShort(PrefMaxMovingTargetDistance); }
void setMaxMovingTargetDistance(uint16_t value) { myPrefs.putUShort(PrefMaxMovingTargetDistance, value); }

uint8_t minMovingTargetEnergy() { return myPrefs.getUChar(PrefMinMovingTargetEnergy); }
void setMinMovingTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMinMovingTargetEnergy, value); }

uint8_t maxMovingTargetEnergy() { return myPrefs.getUChar(PrefMaxMovingTargetEnergy); }
void setMaxMovingTargetEnergy(uint8_t value) { myPrefs.putUChar(PrefMaxMovingTargetEnergy, value); }