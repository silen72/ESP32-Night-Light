#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Arduino.h>

static const uint8_t DEFAULT_BRIGHTNESS_STEP = 8;
static const uint16_t DEFAULT_TRANSITION_DURATION_MS = 1000;
static const uint8_t DEFAULT_MAX_BRIGHTNESS = 210;
static const uint8_t DEFAULT_ON_BRIGHTNESS = 210;
static const uint8_t DEFAULT_NIGHTLIGHT_BRIGHTNESS = 16;
static const uint8_t DEFAULT_MAX_NIGHTLIGHT_BRIGHTNESS = 128;
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

// Get preference: Night light stays on for this many seconds, then checks if it is still needed
uint16_t nightLightOnDuration();
// Set preference: Night light stays on for this many seconds, then checks if it is still needed
void setNightLightOnDuration(uint16_t value);

// Get preference: Night light is turned on, when the value read from the ldr (-> LDR_PIN) is less than or equal this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
uint16_t nightLightThreshold();
// Set preference: Night light is turned on, when the value read from the ldr (-> LDR_PIN) is less than or equal this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
void setNightLightThreshold(uint16_t value);

// Get preference: Brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
uint8_t nightLightBrightness();
// Set preference: Brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
void setNightLightBrightness(uint8_t value);

// Get preference: Limit LED strip brightness (0..255, 0 = off, 255 = full on)
uint8_t maxNightLightBrightness();
// Set preference: Limit LED strip brightness (0..255, 0 = off, 255 = full on)
void setMaxNightLightBrightness(uint8_t value);

// Get preference: Whether night light mode is enabled
bool allowNightLight();
// Set preference: Enable / disable the night light mode
void setAllowNightLight(bool value);

// Get preference: Maximum allowed LED strip brightness (0..255, 0 = off, 255 = full on)
uint8_t maxBrightness();
// Set preference: Maximum allowed LED strip brightness (0..255, 0 = off, 255 = full on)
void setMaxBrightness(uint8_t value);

// Get preference: LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
uint8_t onBrightness();
// Set preference: LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
void setOnBrightness(uint8_t value);

// Get preference: The duration of a transition between two LED strip brightness states
uint16_t transitionDurationMs();
// Set preference: The duration of a transition between two LED strip brightness states
void setTransitionDurationMs(uint16_t value);

// Get preference: Brightness will be increased and decreased by this value (possible values: 1 .. 255)
uint8_t stepBrightness();
// Set preference: Brightness will be increased and decreased by this value (possible values: 1 .. 255, 0 will be treated as 1)
void setStepBrightness(uint8_t value);

// Get preference for presence detection: A stationary target must be at least this far away from the sensor (in cm) to be considered to enable the night light
uint16_t minStationaryTargetDistance();
// Set preference for presence detection: A stationary target must be at least this far away from the sensor (in cm) to be considered to enable the night light
void setMinStationaryTargetDistance(uint16_t value);
// Get preference for presence detection: A stationary target must be at most this far away from the sensor (in cm) to be considered to enable the night light
uint16_t maxStationaryTargetDistance();
// Set preference for presence detection: A stationary target must be at most this far away from the sensor (in cm) to be considered to enable the night light
void setMaxStationaryTargetDistance(uint16_t value);

// Get preference for presence detection: A stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t minStationaryTargetEnergy();
// Set preference for presence detection: A stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMinStationaryTargetEnergy(uint8_t value);
// Get preference for presence detection: A stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t maxStationaryTargetEnergy();
// Set preference for presence detection: A stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMaxStationaryTargetEnergy(uint8_t value);

// Get preference for presence detection: A moving target must be at least this far away from the sensor (in cm) to be considered to enable the night light
uint16_t minMovingTargetDistance();
// Set preference for presence detection: A moving target must be at least this far away from the sensor (in cm) to be considered to enable the night light
void setMinMovingTargetDistance(uint16_t value);
// Get preference for presence detection: A moving target must be at most this far away from the sensor (in cm) to be considered to enable the night light
uint16_t maxMovingTargetDistance();
// Set preference for presence detection: A moving target must be at most this far away from the sensor (in cm) to be considered to enable the night light
void setMaxMovingTargetDistance(uint16_t value);
// Get preference for presence detection: A moving target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t minMovingTargetEnergy();
// Set preference for presence detection: A moving target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMinMovingTargetEnergy(uint8_t value);
// Get preference for presence detection: A moving target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t maxMovingTargetEnergy();
// Set preference for presence detection: A moving target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMaxMovingTargetEnergy(uint8_t value);

// configure the handling of preferences and configurations
void configSetup();
// does nothing as of yet
void configLoop();

#endif