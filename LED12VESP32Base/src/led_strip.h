#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#include <Arduino.h>

static uint8_t const LED_STRIP_PIN = LED_BUILTIN;

void ledStripSetup();
void ledStripLoop();

// set a new target brightness value
void ledStripSetTargetBrightness(uint8_t brightness);

// give a visual confirmation via the LED strip
void ledStripConfirm();

// do a sweep of all brightnesses
void ledStripTest();


void setDebugStream(Stream &terminalStream);

#endif