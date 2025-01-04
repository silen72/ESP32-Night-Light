#ifndef _TOUCH_H_
#define _TOUCH_H_

#include <Arduino.h>

typedef std::function<void(uint8_t)> TouchCallbackFunction;

enum ButtonNumber
{
    one,
    two,
    three,
    four
};

void setClickHandler(ButtonNumber button, TouchCallbackFunction f);
void setDoubleClickHandler(ButtonNumber button, TouchCallbackFunction f);
void setTripleClickHandler(ButtonNumber button, TouchCallbackFunction f);
void setLongClickHandler(ButtonNumber button, TouchCallbackFunction f);

// green wire
static uint8_t const TOUCH1_PIN = 27;
// orange wire
static uint8_t const TOUCH2_PIN = 14;
// yellow wire
static uint8_t const TOUCH3_PIN = 12;
// white wire
static uint8_t const TOUCH4_PIN = 13;

void touchDebug(Stream &terminalStream);

void touchSetup();
void touchLoop();

#endif