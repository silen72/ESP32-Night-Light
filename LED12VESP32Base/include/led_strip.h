#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#include <Arduino.h>

void ledStripSetup();
void ledStripLoop();

// Query the brightness the LED strip is supposed to have.
uint8_t ledStripTargetBrightness();
// Query the brightness the LED strip currently has.
uint8_t ledStripCurrentBrightness();

// Set a new target brightness value.
void ledStripSetTargetBrightness(uint8_t brightness);
// Set a new transistion durcation
void ledStripSetTransitionDuration(uint16_t value);
uint16_t ledStripGetTransitionDuration();

// Gives a visual confirmation via the LED strip (two short off-on-pulses).
// Can also visualize "on" or "off" after the two blinks. Set alsoConfirmState to true for this and set alsoConfirmState.
// The latter controls whether it show a brief "on" period or a short "off" period.
void ledStripConfirm(bool alsoConfirmState = false, bool confirmThisState = false);

// Debug messages get printed to this stream. Do not set it if you don't want debug messages.
void ledStripDebug(Stream &terminalStream);

#endif