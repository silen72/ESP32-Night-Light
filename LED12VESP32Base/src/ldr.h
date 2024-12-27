#ifndef _LDR_H_
#define _LDR_H_

#include <Arduino.h>

// LDR is connected to GPIO36 / VP
static const uint8_t LDR_PIN = 36;

void ldrSetup();
void ldrLoop();

// the averaged measured brightness
uint16_t averagedBrightness();
// the most recently measured brightness
uint16_t measuredBrightness();

// measure and calculate average brightness every x milliseconds
uint32_t measurementDelayMs();
// measure and calculate average brightness every x milliseconds
void setMeasurementDelayMs(uint32_t value);

unsigned long lastMeasureTs();

#endif