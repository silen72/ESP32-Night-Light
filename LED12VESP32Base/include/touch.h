#ifndef _TOUCH_H_
#define _TOUCH_H_

#include <Arduino.h>

typedef std::function<void(uint8_t)> TouchCallbackFunction;

enum ButtonNumber
{
    ONE,
    TWO,
    THREE,
    FOUR
};

void setClickHandler(const ButtonNumber button, const TouchCallbackFunction &f);
void setDoubleClickHandler(const ButtonNumber button, const TouchCallbackFunction &f);
void setTripleClickHandler(const ButtonNumber button, const TouchCallbackFunction &f);
void setLongClickHandler(const ButtonNumber button, const TouchCallbackFunction &f);
void setReleasedHandler(const ButtonNumber button, const TouchCallbackFunction &f);

void touchDebug(Stream &terminalStream);

void touchSetup();
void touchLoop();

#endif