#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#include <Arduino.h>

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