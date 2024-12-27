#ifndef _PRESENCE_H_
#define _PRESENCE_H_

#include <Arduino.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 33
#define RADAR_TX_PIN 32

struct LD2410Firmware
{
    uint8_t Major;
    uint8_t Minor;
    uint32_t Bugfix;
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
bool isConnected();

void presenceSetup();
void presenceLoop();

#endif