#ifndef _DEVICE_COMMON_H_
#define _DEVICE_COMMON_H_

#include <Arduino.h>

#define MONITOR_SERIAL Serial

// LDR is connected to GPIO36 / SENSOR_VP
static const uint8_t LDR_PIN = 36;

// LD2410 Presence detector
#define RADAR_SERIAL Serial1
static const uint8_t RADAR_RX_PIN = 33; // yellow wire
static const uint8_t RADAR_TX_PIN = 32; // green wire

// LED-Strip is controlled by a MOSFET connected to gpio 2 = LED_BUILTIN
static const uint8_t LED_STRIP_PIN = LED_BUILTIN;

// Touch buttons
static const uint8_t TOUCH1_PIN = 27; // green wire
static const uint8_t TOUCH2_PIN = 14; // orange wire
static const uint8_t TOUCH3_PIN = 12; // yellow wire
static const uint8_t TOUCH4_PIN = 13; // white wire

// Firmware version
static const uint16_t FIRMWARE_VERSION_MAJOR = 0;
static const uint16_t FIRMWARE_VERSION_MINOR = 6;
static const uint16_t FIRMWARE_VERSION_PATCH = 0;

#endif