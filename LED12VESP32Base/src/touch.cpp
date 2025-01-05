#include <Button2.h>

#include "touch.h"

Button2 touch1, touch2, touch3, touch4;

TouchCallbackFunction clicked_cb[4] = {NULL};
TouchCallbackFunction doubleClicked_cb[4] = {NULL};
TouchCallbackFunction tripleClicked_cb[4] = {NULL};
TouchCallbackFunction longClicked_cb[4] = {NULL};
TouchCallbackFunction released_cb[4] = {NULL};

Stream *debug_uart_touch = nullptr;				// The stream used for the debugging

static const uint8_t LAST_BUTTON = 3;

uint8_t buttonNumber(Button2 &btn)
{
  if (btn == touch1)
    return 0;
  if (btn == touch2)
    return 1;
  if (btn == touch3)
    return 2;
  if (btn == touch4)
    return LAST_BUTTON;
  return LAST_BUTTON + 1;
}

void clicked(Button2 &btn)
{
  uint8_t number = buttonNumber(btn);
  if (number <= LAST_BUTTON && clicked_cb[number] != NULL)
    clicked_cb[number](number);
  if (debug_uart_touch != nullptr)
  {
    debug_uart_touch->print(F("clicked: "));
    debug_uart_touch->println(number);
  }
}

void doubleClicked(Button2 &btn)
{
  uint8_t number = buttonNumber(btn);
  if (number <= LAST_BUTTON && doubleClicked_cb[number] != NULL)
    doubleClicked_cb[number](number);
  if (debug_uart_touch != nullptr)
  {
    debug_uart_touch->print(F("double clicked: "));
    debug_uart_touch->println(number);
  }
}

void longClicked(Button2 &btn)
{
  uint8_t number = buttonNumber(btn);
  if (number <= LAST_BUTTON && longClicked_cb[number] != NULL)
    longClicked_cb[number](number);
  if (debug_uart_touch != nullptr)
  {
    debug_uart_touch->print(F("long clicked: "));
    debug_uart_touch->println(number);
  }
}

void tripleClicked(Button2 &btn)
{
  uint8_t number = buttonNumber(btn);
  if (number <= LAST_BUTTON && tripleClicked_cb[number] != NULL)
    tripleClicked_cb[number](number);
  if (debug_uart_touch != nullptr)
  {
    debug_uart_touch->print(F("triple clicked: "));
    debug_uart_touch->println(number);
  }
}

void released(Button2 &btn)
{
  uint8_t number = buttonNumber(btn);
  if (number <= LAST_BUTTON && released_cb[number] != NULL)
    released_cb[number](number);
  if (debug_uart_touch != nullptr)
  {
    debug_uart_touch->print(F("released: "));
    debug_uart_touch->println(number);
  }
}


void setupButton(const uint8_t pin, Button2 &btn)
{
  // the touch buttons are active high
  btn.begin(pin, INPUT_PULLDOWN, false);
  btn.setClickHandler(clicked);
  btn.setDoubleClickHandler(doubleClicked);
  btn.setLongClickDetectedHandler(longClicked);
  btn.setLongClickDetectedRetriggerable(true);
  btn.setTripleClickHandler(tripleClicked);
  btn.setReleasedHandler(released);
  if (debug_uart_touch != nullptr)
  {
    
    debug_uart_touch->println(" Button ID:\t" + String(btn.getID()));
    debug_uart_touch->println(" Longpress Time:\t" + String(btn.getLongClickTime()) + "ms");
    debug_uart_touch->println(" DoubleClick Time:\t" + String(btn.getDoubleClickTime()) + "ms");
    debug_uart_touch->println();
  }
}

void setClickHandler(ButtonNumber button, TouchCallbackFunction f){  clicked_cb[button] = f;}
void setDoubleClickHandler(ButtonNumber button, TouchCallbackFunction f){  doubleClicked_cb[button] = f;}
void setLongClickHandler(ButtonNumber button, TouchCallbackFunction f){  longClicked_cb[button] = f;}
void setTripleClickHandler(ButtonNumber button, TouchCallbackFunction f){  tripleClicked_cb[button] = f;}
void setReleasedHandler(ButtonNumber button, TouchCallbackFunction f){  released_cb[button] = f;}

void touchDebug(Stream &terminalStream)
{
  debug_uart_touch = &terminalStream;
}

void touchSetup()
{
  // The "OFF" Button
  // click: switch off the current light
  //        when night light: disable night light
  //        will do nothing, when light is off
  // long:  toggle (disable / enable) night light
  touch1.setID(0);
  touch1.setLongClickTime(2000);
  setupButton(TOUCH1_PIN, touch1);
  touch1.setLongClickDetectedRetriggerable(false);

  // The "LESS" Button
  // click: decreases brighness one step
  // long:  decreases brighness one step as long as the button is pressed
  touch2.setID(1);
  //touch2.setLongClickTime(500);
  setupButton(TOUCH2_PIN, touch2);

  // The "MORE" Button
  // click: increases brighness one step
  // long:  increases brighness one step as long as the button is pressed
  touch3.setID(2);
  //touch3.setLongClickTime(500);
  setupButton(TOUCH3_PIN, touch3);

  // The "ON" Button
  // click: switch on light (when night light or off)
  // long:  save current brightness as default for light
  //        when night light: save current brightness as default for night light
  //        will do nothing, when light is off
  touch4.setID(3);
  touch4.setLongClickTime(2000);
  setupButton(TOUCH4_PIN, touch4);
  touch4.setLongClickDetectedRetriggerable(false);
}

void touchLoop()
{
  touch1.loop();
  touch2.loop();
  touch3.loop();
  touch4.loop();
}
