#include <led_strip.h>
#include <device_common.h>
#include <config.h>

static const uint8_t LEDC_CHANNEL_0 = 0;
static const uint32_t LEDC_FREQ_HZ = 5000;
static const uint8_t LEDC_RESOLUTION_BITS = 8;

enum LEDStripState
{
  TARGET_BRIGHTNESS_REACHED,
  START_TRANSITION_TO_BRIGHTNESS,
  TRANSITION_TO_BRIGHTNESS,
  TRANSITION_TO_BRIGHTNESS_DONE,
  CONFIRM_START,
  CONFIRM_OFF_1,
  CONFIRM_ON_1,
  CONFIRM_OFF_2,
  CONFIRM_ON_2,
  CONFIRM_STATE_START,
  CONFIRM_STATE,
  CONFIRM_DONE,
};

Stream *debug_uart_led_strip = nullptr;

LEDStripState _ledCurrentState = TARGET_BRIGHTNESS_REACHED; // The state the LED strip hardware is currently in
uint8_t _ledTransitionStartBrightness = 0;                  // The brightness the LED strip hardware was set to at the start of the transition
uint8_t _ledCurrentBrightness = 0;                          // The actual LED strip hardware brightness
uint8_t _ledTargetBrightness = 0;                           // The target LED strip hardware brightness to be reached in the transition
int16_t _ledBrightnessGap = 0;                              // Brightness gap to bridge between brightness on transition start and desired brightness (might be negative)
unsigned long _loopTimeStamp = 0;                           // The moment in time the current loop cycle has been entered
unsigned long _transitionStartTs = 0;                       // The moment in time the current transition started (0 when there is no current transition)
unsigned long _transitionDuration = 0;                      // How much time the current transition has taken (0 when there is no current transition)

LEDStripState _ledSavedState = TARGET_BRIGHTNESS_REACHED; // For storing the state when confirming
uint8_t _ledSavedBrightness = 0;                          // For storing the current brightness when confirming
bool _alsoConfirmValue = false;                           // Whether after confirming input a visual state confirmation is also required
bool _confirmThisState = false;                           // When giving a visual state confirmation, visualize this state
unsigned long _lastConfirmTs = 0;                         // For delaying the led on/off cycles
unsigned long _delayConfirmMs = 0;                        // For delaying the led on/off cycles
unsigned long _savedTransitionDuration = 0;               // The time left for the interrupted a current transition

void setBrightness(uint8_t value);
void startTransitionToBrightness();
void transitionToBrightness();
void transitionToBrightnessDone();
void doConfirm();

uint8_t ledStripCurrentBrightness() { return _ledCurrentBrightness; }
uint8_t ledStripTargetBrightness() { return _ledTargetBrightness; }

void ledStripDebug(Stream &terminalStream)
{
  debug_uart_led_strip = &terminalStream;
}

void ledStripSetup()
{
  ledcSetup(LEDC_CHANNEL_0, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
  ledcAttachPin(LED_STRIP_PIN, LEDC_CHANNEL_0);
  setBrightness(_ledTargetBrightness);
}

void ledStripSetTargetBrightness(uint8_t brightness)
{
  if (_ledTargetBrightness != brightness)
  {
    _ledTargetBrightness = brightness;
    _ledCurrentState = START_TRANSITION_TO_BRIGHTNESS;
  }
}

void ledStripLoop()
{
  _loopTimeStamp = millis();

  switch (_ledCurrentState)
  {
  case START_TRANSITION_TO_BRIGHTNESS:
  {
    startTransitionToBrightness();
    break;
  }
  case TRANSITION_TO_BRIGHTNESS:
  {
    transitionToBrightness();
    break;
  }
  case TRANSITION_TO_BRIGHTNESS_DONE:
  {
    transitionToBrightnessDone();
    break;
  }
  case CONFIRM_START:
  case CONFIRM_OFF_1:
  case CONFIRM_OFF_2:
  case CONFIRM_ON_1:
  case CONFIRM_ON_2:
  case CONFIRM_STATE_START:
  case CONFIRM_STATE:
  case CONFIRM_DONE:
  {
    doConfirm();
  }
  }
}

void ledStripConfirm(bool alsoConfirmValue, bool confirmThisValue)
{
  if (debug_uart_led_strip != nullptr)
  {
    debug_uart_led_strip->print(F("ledStripConfirm: "));
  }

  switch (_ledCurrentState)
  {
  case START_TRANSITION_TO_BRIGHTNESS:
  case TRANSITION_TO_BRIGHTNESS:
  case TRANSITION_TO_BRIGHTNESS_DONE:
  {
    unsigned long tsDiffMs = _loopTimeStamp - _transitionStartTs;
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->print(F("  interrupting transition, remaining time (ms): "));
      debug_uart_led_strip->println(tsDiffMs);
    }
    break;
  }
  case TARGET_BRIGHTNESS_REACHED:
  {
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->println(F("  confirm while target brightness reached"));
    }
    break;
  }
  default:
  {
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->println(F("  already confirming (request ignored)"));
    }
    return;
  }
  }
  _ledSavedState = _ledCurrentState;
  _alsoConfirmValue = alsoConfirmValue;
  _confirmThisState = confirmThisValue;
  _ledCurrentState = CONFIRM_START;
}

void debugPrintStateText(LEDStripState state, bool addPrintln = false)
{
  if (debug_uart_led_strip != nullptr)
  {
    switch (state)
    {
    case TARGET_BRIGHTNESS_REACHED:
      debug_uart_led_strip->print(F("TARGET_BRIGHTNESS_REACHED"));
      break;
    case START_TRANSITION_TO_BRIGHTNESS:
      debug_uart_led_strip->print(F("START_TRANSITION_TO_BRIGHTNESS"));
      break;
    case TRANSITION_TO_BRIGHTNESS:
      debug_uart_led_strip->print(F("TRANSITION_TO_BRIGHTNESS"));
      break;
    case TRANSITION_TO_BRIGHTNESS_DONE:
      debug_uart_led_strip->print(F("TRANSITION_TO_BRIGHTNESS_DONE"));
      break;
    case CONFIRM_START:
      debug_uart_led_strip->print(F("CONFIRM_START"));
      break;
    case CONFIRM_OFF_1:
      debug_uart_led_strip->print(F("CONFIRM_OFF_1"));
      break;
    case CONFIRM_ON_1:
      debug_uart_led_strip->print(F("CONFIRM_ON_1"));
      break;
    case CONFIRM_OFF_2:
      debug_uart_led_strip->print(F("CONFIRM_OFF_2"));
      break;
    case CONFIRM_ON_2:
      debug_uart_led_strip->print(F("CONFIRM_ON_2"));
      break;
    case CONFIRM_STATE_START:
      debug_uart_led_strip->print(F("CONFIRM_STATE_START"));
      break;
    case CONFIRM_STATE:
      debug_uart_led_strip->print(F("CONFIRM_STATE"));
      break;
    case CONFIRM_DONE:
      debug_uart_led_strip->print(F("CONFIRM_DONE"));
      break;
    default:
      break;
    }
    if (addPrintln)
      debug_uart_led_strip->println();
  }
}

void setBrightness(uint8_t value)
{
  ledcWrite(LEDC_CHANNEL_0, value);
  if (debug_uart_led_strip != nullptr)
  {
    debug_uart_led_strip->print(F(" => set LED brightness: "));
    debug_uart_led_strip->println(value);
  }
}

void startTransitionToBrightness()
{
  _ledTransitionStartBrightness = _ledCurrentBrightness;
  _ledBrightnessGap = (int16_t)_ledTargetBrightness - (int16_t)_ledTransitionStartBrightness;
  _transitionStartTs = _loopTimeStamp;
  _ledCurrentState = TRANSITION_TO_BRIGHTNESS;
  if (debug_uart_led_strip != nullptr)
  {
    debug_uart_led_strip->print(F("startTransitionToBrightness from "));
    debug_uart_led_strip->print((int16_t)_ledCurrentBrightness);
    debug_uart_led_strip->print(F(" to "));
    debug_uart_led_strip->println((int16_t)_ledTargetBrightness);
    debug_uart_led_strip->print(F("  gap: "));
    debug_uart_led_strip->println(_ledBrightnessGap);
  }
}

void transitionToBrightness()
{
  // check time left to reach desired brightness (in ms)
  _transitionDuration = _loopTimeStamp - _transitionStartTs;
  if (_transitionDuration > transitionDurationMs())
  {
    // time's up
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->println(F("transitionToBrightness"));
      debug_uart_led_strip->print(F(" duration: "));
      debug_uart_led_strip->println(_transitionDuration);
      debug_uart_led_strip->println(F(" => transition duration's up"));
    }
    _transitionDuration = 0;
    _ledCurrentBrightness = _ledTargetBrightness;
    setBrightness(_ledCurrentBrightness);
    _ledCurrentState = TRANSITION_TO_BRIGHTNESS_DONE;
  }
  else
  {
    // time left, make transition smooth
    int32_t a = _ledBrightnessGap * _transitionDuration;
    int16_t diff = a / transitionDurationMs();
    int16_t newval = _ledTransitionStartBrightness + diff;
    uint8_t newBrightness = (uint8_t)newval;
    if (newBrightness != _ledCurrentBrightness)
    {
      if (debug_uart_led_strip != nullptr)
      {
        debug_uart_led_strip->println(F("transitionToBrightness"));
        debug_uart_led_strip->print(F(" duration: "));
        debug_uart_led_strip->println(_transitionDuration);
        debug_uart_led_strip->print(F(" => transition from "));
        debug_uart_led_strip->print((int16_t)_ledCurrentBrightness);
        debug_uart_led_strip->print(F(" to "));
        debug_uart_led_strip->println(newBrightness);
      }
      _ledCurrentBrightness = newBrightness;
      setBrightness(_ledCurrentBrightness);
    }
  }
}

void transitionToBrightnessDone()
{
  if (debug_uart_led_strip != nullptr)
  {
    debug_uart_led_strip->println(F("transitionToBrightnessDone"));
  }
  setBrightness(_ledTargetBrightness);
  _ledCurrentState = TARGET_BRIGHTNESS_REACHED;
}

void doConfirm()
{
  // delay when needed
  if (_loopTimeStamp - _lastConfirmTs <= _delayConfirmMs)
    return;

  LEDStripState nextState;

  debugPrintStateText(_ledCurrentState, true);

  switch (_ledCurrentState)
  {

  case CONFIRM_START:
    // save current state to be restored when confirmation is done (see CONFIRM_DONE)
    _delayConfirmMs = 0;
    _ledSavedBrightness = _ledCurrentBrightness;
    _savedTransitionDuration = _transitionDuration;
    nextState = CONFIRM_OFF_1;
    break;

  case CONFIRM_OFF_1:
    // blink, phase 1: switch off light for 200ms
    setBrightness(0);
    _delayConfirmMs = 200;
    nextState = CONFIRM_ON_1;
    break;

  case CONFIRM_ON_1:
    // blink, phase 2: set light to full brightness for 200ms
    setBrightness(maxBrightness());
    _delayConfirmMs = 200;
    nextState = CONFIRM_OFF_2;
    break;

  case CONFIRM_OFF_2:
    // blink, phase 3: switch off light for 200ms
    setBrightness(0);
    _delayConfirmMs = 200;
    nextState = CONFIRM_ON_2;
    break;

  case CONFIRM_ON_2:
    // blink, phase 4: set light to full brightness for 200ms
    setBrightness(maxBrightness());
    _delayConfirmMs = 200;
    // next state depends on whether a value is to be visualized also
    nextState = _alsoConfirmValue ? CONFIRM_STATE_START : CONFIRM_DONE;
    break;

  case CONFIRM_STATE_START:
    // visualize a bool value, phase 1: set brightness in contrast to value
    // coming from CONFIRM_ON_2, brightness is now full on
    if (_confirmThisState)
    {
      setBrightness(0);
    }
    // the contrast (-> OFF) only needs to be visible (for 200ms), when _confirmThisState is true
    _delayConfirmMs = _confirmThisState ? 200 : 0;
    nextState = CONFIRM_STATE;
    break;

  case CONFIRM_STATE:
    // visualize a bool value, phase 2: set brightness according to value for 1000ms
    setBrightness(_confirmThisState ? maxBrightness() : 0);
    _delayConfirmMs = 1000;
    nextState = CONFIRM_DONE;
    break;

  case CONFIRM_DONE:
    setBrightness(_ledSavedBrightness);
    _delayConfirmMs = 0;
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->print(F("  restore state: "));
      debugPrintStateText(_ledSavedState, true);
    }
    _transitionDuration = _savedTransitionDuration;
    _transitionStartTs = _loopTimeStamp - _transitionDuration;
    nextState = _ledSavedState;
    break;

  default:
    if (debug_uart_led_strip != nullptr)
    {
      debug_uart_led_strip->println(F("DEFAULT - THIS IS A BUG!"));
      debug_uart_led_strip->print(F("  state: "));
      debugPrintStateText(_ledCurrentState, true);
    }
    _delayConfirmMs = 0;
    nextState = CONFIRM_DONE;
  }

  if (debug_uart_led_strip != nullptr && _ledCurrentState != nextState)
  {
    debug_uart_led_strip->print(F("switching state from "));
    debugPrintStateText(_ledCurrentState);
    debug_uart_led_strip->print(F(" to "));
    debugPrintStateText(nextState, true);
  }
  _lastConfirmTs = _loopTimeStamp;
  _ledCurrentState = nextState;
}
