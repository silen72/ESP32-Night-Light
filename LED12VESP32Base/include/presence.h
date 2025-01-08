#ifndef _PRESENCE_H_
#define _PRESENCE_H_

#include <Arduino.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
uint8_t const RADAR_RX_PIN = 33;
uint8_t const RADAR_TX_PIN = 32;

struct LD2410Firmware
{
    bool Valid = false;
    uint8_t Major = 0;
    uint8_t Minor = 0;
    uint32_t Bugfix = 0;
};

struct LD2410Config
{
    bool Valid = false;
    uint8_t max_gate = 0;
    uint8_t max_moving_gate = 0;
    uint8_t max_stationary_gate = 0;
    uint16_t sensor_idle_time = 0;
    uint8_t motion_sensitivity[9] = {0,0,0,0,0,0,0,0,0};
    uint8_t stationary_sensitivity[9] = {0,0,0,0,0,0,0,0,0};
};

struct LD2410Detection
{
    bool presenceDetected = false;
    bool stationaryTargetDetected = false;
    uint16_t stationaryTargetDistance = 0;
    uint8_t stationaryTargetEnergy = 0;
    bool movingTargetDetected = false;
    uint16_t movingTargetDistance = 0;
    uint8_t movingTargetEnergy = 0;
};

LD2410Firmware firmwareInfo();
LD2410Detection presenceInfo();
LD2410Config currentConfig();
bool isConnected();
bool requestFactoryReset();

void presenceDebug(Stream &terminalStream);

void presenceSetup();
void presenceLoop();

#endif