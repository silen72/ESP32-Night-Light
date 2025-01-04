#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Arduino.h>

static const uint8_t DEFAULT_BRIGHTNESS_STEP = 8;
static const uint16_t DEFAULT_TRANSITION_DURATION_MS = 1000;
static const uint8_t DEFAULT_BRIGHTNESS_UPPER_LIMIT = 210;
static const uint8_t DEFAULT_ON_BRIGHTNESS = 210;
static const uint8_t DEFAULT_NIGHTLIGHT_BRIGHTNESS = 16;
static const bool DEFAULT_ALLOW_NIGHTLIGHT = true;
static const uint16_t DEFAULT_LDR_NIGHTLIGHT_THRESHOLD = 30;
static const uint16_t DEFAULT_NIGHTLIGHT_ON_DURATION_S = 30; // night light stays on for 30s, then checks if it is still needed
static const bool DEFAULT_RESTORE_STATE_AFTER_POWERLOSS = true;
static const uint16_t DEFAULT_MIN_MOVING_TARGET_DISTANCE = 0;
static const uint16_t DEFAULT_MAX_MOVING_TARGET_DISTANCE = UINT16_MAX;
static const uint8_t DEFAULT_MIN_MOVING_TARGET_ENERGY = 0;
static const uint8_t DEFAULT_MAX_MOVING_TARGET_ENERGY = UINT8_MAX;
static const uint16_t DEFAULT_MIN_STATIONARY_TARGET_DISTANCE = 0;
static const uint16_t DEFAULT_MAX_STATIONARY_TARGET_DISTANCE = UINT16_MAX;
static const uint8_t DEFAULT_MIN_STATIONARY_TARGET_ENERGY = 0;
static const uint8_t DEFAULT_MAX_STATIONARY_TARGET_ENERGY = UINT8_MAX;

// night light stays on for this many seconds, then checks if it is still needed
uint16_t nightLightOnDuration();
// night light stays on for this many seconds, then checks if it is still needed
void setNightLightOnDuration(uint16_t value);

// night light is turned on, when the value read from the ldr (-> LDR_PIN) is below this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
uint16_t nightLightThreshold();
// night light is turned on, when the value read from the ldr (-> LDR_PIN) is below this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
void setNightLightThreshold(uint16_t value);

// brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
uint8_t nightLightBrightness();
// brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
void setNightLightBrightness(uint8_t value);

// this can turn off the night light mode (when set to false)
bool allowNightLight();
// enable / disable the night light mode (when set to false)
void setAllowNightLight(bool value);

// when this is set to true, the LED strip will return to its state from before the power loss
bool restoreAfterPowerloss();
// when this is set to true, the LED strip will return to its state from before the power loss
void setRestoreAfterPowerloss(bool value);

// limit LED strip brightness (0..255, 0 = off, 255 = full on)
uint8_t maxBrightness();
// limit LED strip brightness (0..255, 0 = off, 255 = full on)
void setMaxBrightness(uint8_t value);

// LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
uint8_t onBrightness();
// LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
void setOnBrightness(uint8_t value);

// the duration of a transition between two LED strip brightness states
uint16_t transitionDurationMs();
// the duration of a transition between two LED strip brightness states
void setTransitionDurationMs(uint16_t value);

// each push of the brightness buttons will raise / lower the brightness of the LED strip by this value
uint8_t stepBrightness();
// each push of the brightness buttons will raise / lower the brightness of the LED strip by this value (allowed values: 1 .. 255)
void setStepBrightness(uint8_t value);

// presence detection: a stationary target must be at least this far away from the sensor (in cm) to be considered (to enable the night light)
uint16_t minStationaryTargetDistance();
// presence detection: a stationary target must be at least this far away from the sensor (in cm) to be considered (to enable the night light)
void setMinStationaryTargetDistance(uint16_t value);
// presence detection: a stationary target must be at most this far away from the sensor (in cm) to be considered (to enable the night light)
uint16_t maxStationaryTargetDistance();
// presence detection: a stationary target must be at most this far away from the sensor (in cm) to be considered (to enable the night light)
void setMaxStationaryTargetDistance(uint16_t value);

// presence detection: a stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
uint8_t minStationaryTargetEnergy();
// presence detection: a stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
void setMinStationaryTargetEnergy(uint8_t value);
// presence detection: a stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
uint8_t maxStationaryTargetEnergy();
// presence detection: a stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
void setMaxStationaryTargetEnergy(uint8_t value);

// presence detection: a moving target must be at least this far away from the sensor (in cm) to be considered (to enable the night light)
uint16_t minMovingTargetDistance();
// presence detection: a moving target must be at least this far away from the sensor (in cm) to be considered (to enable the night light)
void setMinMovingTargetDistance(uint16_t value);
// presence detection: a moving target must be at most this far away from the sensor (in cm) to be considered (to enable the night light)
uint16_t maxMovingTargetDistance();
// presence detection: a moving target must be at most this far away from the sensor (in cm) to be considered (to enable the night light)
void setMaxMovingTargetDistance(uint16_t value);
// presence detection: a moving target must have at least this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
uint8_t minMovingTargetEnergy();
// presence detection: a moving target must have at least this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
void setMinMovingTargetEnergy(uint8_t value);
// presence detection: a moving target must have at most this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
uint8_t maxMovingTargetEnergy();
// presence detection: a moving target must have at most this "energy" (confidence) (0 .. 100) to be considered (to enable the night light)
void setMaxMovingTargetEnergy(uint8_t value);

void configSetup();
void configLoop();

#endif