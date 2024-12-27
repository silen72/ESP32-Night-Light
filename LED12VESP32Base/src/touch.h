#ifndef _TOUCH_H_
#define _TOUCH_H_

#include <Arduino.h>

// green wire
static uint8_t const TOUCH1_PIN = 27;
// orange wire
static uint8_t const TOUCH2_PIN = 14;
// yellow wire
static uint8_t const TOUCH3_PIN = 12;
// white wire
static uint8_t const TOUCH4_PIN = 13;

void touchSetup();
void touchLoop();

#endif