#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#include <Arduino.h>

static uint8_t const LED_STRIP_PIN = LED_BUILTIN;
static uint8_t const LEDC_CHANNEL_0 = 0;
static uint32_t const LEDC_FREQ_HZ = 5000;
static uint8_t const LEDC_RESOLUTION_BITS = 8;

void ledStripSetup();
void ledStripLoop();

// 
uint8_t ledStripTargetBrightness();
uint8_t ledStripCurrentBrightness();


// set a new target brightness value
void ledStripSetTargetBrightness(uint8_t brightness);

// give a visual confirmation via the LED strip
void ledStripConfirm(bool alsoConfirmState = false, bool confirmThisState = false);

void ledStripDebug(Stream &terminalStream);

#endif