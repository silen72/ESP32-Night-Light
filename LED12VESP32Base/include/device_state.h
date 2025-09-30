#ifndef _DEVICE_STATE_H_
#define _DEVICE_STATE_H_

#include <Arduino.h>
#include <config.h>

// The states the device may be in
enum State
{
    OFF,                          // Light is off. Constantly checks whether condition for night light is met. This is the default state after power on.
    START_TRANSIT_TO_ON,          // Initialize the transition to a new brightness (> 0). The new brightness is set in _targetBrightness. Next state is TRANSIT_TO_ON.
    TRANSIT_TO_ON,                // Gradually increases or decreases the brightness to _targetBrightness, until _targetBrightness equals the actual brightness. The next state is then ON.
    ON,                           // Light is on at the desired brightness. This state only changes by someone touch one of the touch buttons (or power loss).
    START_TRANSIT_TO_OFF,         // Initialize the transition to OFF (brightness = 0). _targetBrightness is set t0 0. Next state is TRANSIT_TO_OFF.
    TRANSIT_TO_OFF,               // Gradually decreases the brightness to 0 until the actual brightness is 0. The next state is then OFF.
    START_TRANSIT_TO_NIGHT_LIGHT, // Initialize the transition to night light brightness. The new brightness is set in _targetBrightness. Next state is TRANSIT_TO_NIGHT_LIGHT.
    TRANSIT_TO_NIGHT_LIGHT,       // Gradually increases or decreases the brightness to _targetBrightness, until _targetBrightness equals the actual brightness. The next state is then NIGHT_LIGHT_ON.
    NIGHT_LIGHT_ON                // Night light is on at night light brightness. Constantly checks whether condition for night light is still met. If not, next state is START_TRANSIT_TO_OFF.
};

struct DeviceStateInfo
{
    State state; // Current state of the device

    bool allowNightLightMode;           // Whether night light should be turned on when it is dark enough and presence is detected
    unsigned long nightLightOnDuration; // The duration the night light stays on even when presence is no longer detected
    uint8_t nightLightBrightness;       // Night light will be set to this brightness when enabled
    uint8_t maxNightLightBrightness;    // Maximum allowed night light brightness
    uint8_t onBrightness;               // Light will be set to this brightness when enabled
    uint8_t maxBrightness;              // Maximum allowed brightness
    uint8_t stepBrightness;             // Brightness will be in/decreased by this step value
    uint8_t brightness;                 // The target brightness. When it differs from the actual brightness, the actual brightness will be gradually modified to be equal.

    uint16_t transitionDurationMs; // A smooth transition between the current and a target brightness will be finished within this duration.

    uint16_t ldrValue;            // The current measurement coming from the ldr (dark 0 .. 4095 bright). Gets updated every loop cycle.
    uint16_t nightLightThreshold; // LDR values equal or less than this value warrant switching the night light on.

    bool presenceDetected;
    bool movingTargetDetected;
    uint16_t movingTargetDistance;
    uint16_t movingTargetDistanceMin;
    uint16_t movingTargetDistanceMax;
    uint8_t movingTargetEnergy;
    uint8_t movingTargetEnergyMin;
    uint8_t movingTargetEnergyMax;
    bool stationaryTargetDetected;
    uint16_t stationaryTargetDistance;
    uint16_t stationaryTargetDistanceMin;
    uint16_t stationaryTargetDistanceMax;
    uint8_t stationaryTargetEnergy;
    uint8_t stationaryTargetEnergyMin;
    uint8_t stationaryTargetEnergyMax;
    unsigned long noPresenceDuration;   // How long no presence has been detected
};

void debugPrintStateText(Stream *stream, State state, bool addPrintln = false);

// Query the current state, all at once
DeviceStateInfo getDeviceState();

// Set values (for web api and web interface)
void modifyAllowNightLightMode(bool value = DEFAULT_ALLOW_NIGHTLIGHT);
void modifyNightLightOnDurationSeconds(uint16_t value = DEFAULT_NIGHTLIGHT_ON_DURATION_S);
void modifyNightLightBrightness(uint8_t value = DEFAULT_NIGHTLIGHT_BRIGHTNESS);
void modifyNightLightThreshold(uint16_t value = DEFAULT_LDR_NIGHTLIGHT_THRESHOLD);
void modifyMaxNightLightBrightness(uint8_t value = DEFAULT_MAX_NIGHTLIGHT_BRIGHTNESS);
void modifyOnBrightness(uint8_t value = DEFAULT_ON_BRIGHTNESS);
void modifyMaxBrightness(uint8_t value = DEFAULT_MAX_BRIGHTNESS);
void modifyBrightnessStep(uint8_t value = DEFAULT_BRIGHTNESS_STEP);
void modifyTransitionDurationMs(uint16_t value = DEFAULT_TRANSITION_DURATION_MS);
void modifyMaxMovingTargetDistance(uint16_t value = DEFAULT_MAX_MOVING_TARGET_DISTANCE);
void modifyMinMovingTargetDistance(uint16_t value = DEFAULT_MIN_MOVING_TARGET_DISTANCE);
void modifyMaxMovingTargetEnergy(uint8_t value = DEFAULT_MAX_MOVING_TARGET_ENERGY);
void modifyMinMovingTargetEnergy(uint8_t value = DEFAULT_MIN_MOVING_TARGET_ENERGY);
void modifyMaxStationaryTargetDistance(uint16_t value = DEFAULT_MAX_MOVING_TARGET_DISTANCE);
void modifyMinStationaryTargetDistance(uint16_t value = DEFAULT_MIN_MOVING_TARGET_DISTANCE);
void modifyMaxStationaryTargetEnergy(uint8_t value = DEFAULT_MAX_MOVING_TARGET_ENERGY);
void modifyMinStationaryTargetEnergy(uint8_t value = DEFAULT_MIN_MOVING_TARGET_ENERGY);

void modifyLightState(bool lampOn);

void deviceSetup();
void deviceLoop();

#endif