#include <Arduino.h>
#include <WebServer.h>

#include "touch.h"
#include "ldr.h"
#include "led_strip.h"
#include "config.h"
#include "presence.h"

enum State
{
  OFF,
  START_TRANSIT_TO_ON,
  TRANSIT_TO_ON,
  ON,
  OFF_CHECK,
  START_TRANSIT_TO_OFF,
  TRANSIT_TO_OFF,
  START_TRANSIT_TO_NIGHT_LIGHT,
  TRANSIT_TO_NIGHT_LIGHT,
  NIGHT_LIGHT_ON
};

State _state = OFF;

Stream *debug_uart_main = nullptr; // The stream used for the debugging

bool _allowNightLight = true;
unsigned long _nightLightEnabledTs = 0; // timestamp, when night light was switched on
unsigned long _nightLightOnDuration = 0;
uint8_t _nightLightBrightness = 0;
uint8_t _onBrightness = 0;
uint8_t _targetBrightness = 0;
uint16_t _ldrValue = 0;

LD2410Detection _prsInfo;

unsigned long _lastReport = 0;
bool _fwDone = true;
bool _cfgDone = true;

uint8_t _previousBrightness = 0;

const ButtonNumber OnButton = ButtonNumber::four;
const ButtonNumber PlusButton = ButtonNumber::three;
const ButtonNumber MinusButton = ButtonNumber::two;
const ButtonNumber OffButton = ButtonNumber::one;

void reportSerial();

void debugPrintStateText(State state, bool addPrintln = false)
{
  if (debug_uart_main != nullptr)
  {
    switch (state)
    {
    case OFF:
      debug_uart_main->print(F("OFF"));
      break;
    case START_TRANSIT_TO_ON:
      debug_uart_main->print(F("START_TRANSIT_TO_ON"));
      break;
    case TRANSIT_TO_ON:
      debug_uart_main->print(F("TRANSIT_TO_ON"));
      break;
    case ON:
      debug_uart_main->print(F("ON"));
      break;
    case OFF_CHECK:
      debug_uart_main->print(F("OFF_CHECK"));
      break;
    case START_TRANSIT_TO_OFF:
      debug_uart_main->print(F("START_TRANSIT_TO_OFF"));
      break;
    case TRANSIT_TO_OFF:
      debug_uart_main->print(F("TRANSIT_TO_OFF"));
      break;
    case START_TRANSIT_TO_NIGHT_LIGHT:
      debug_uart_main->print(F("START_TRANSIT_TO_NIGHT_LIGHT"));
      break;
    case TRANSIT_TO_NIGHT_LIGHT:
      debug_uart_main->print(F("TRANSIT_TO_NIGHT_LIGHT"));
      break;
    case NIGHT_LIGHT_ON:
      debug_uart_main->print(F("NIGHT_LIGHT_ON"));
      break;
    default:
      debug_uart_main->print(F("ERROR! unknown state"));
      break;
    }
    if (addPrintln)
      debug_uart_main->println();
  }
}

bool isNightLightEnabled()
{
  return _allowNightLight;
}

void confirmPreferenceChanged()
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->println(F("confirm otherwise invisible change via LED strip"));
  }
  ledStripConfirm();
}

bool setNightLightEnabled(bool value = true)
{
  bool changed = value != _allowNightLight;
  if (changed)
  {
    if (debug_uart_main != nullptr)
    {
      debug_uart_main->print(F("set allow night light to "));
      debug_uart_main->println(value ? F("true") : F("false"));
    }
    _allowNightLight = value;
  }
  else if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("allow night light not changed, stays "));
    debug_uart_main->println(value ? F("true") : F("false"));
  }
  return changed;
}

bool isDarkEnoughForNightLight()
{
  return _ldrValue <= nightLightThreshold();
}

bool enableNightLight()
{
  return isNightLightEnabled() && isDarkEnoughForNightLight() && _prsInfo.presenceDetected;
}

// -------------------
// ---- ON button ----
// -------------------

// switch on LED strip with previous brightness
void ctrlClickOn(uint8_t btn)
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("ON button clicked in state "));
    debugPrintStateText(_state, true);
  }
  // do nothing when already on or transitioning to on
  switch (_state)
  {
  case START_TRANSIT_TO_ON:
  case TRANSIT_TO_ON:
  case ON:
    if (debug_uart_main != nullptr)
    {
      debug_uart_main->println(F(" => ignored"));
    }
    return;
  }
  uint8_t newTargetBrightness = _onBrightness;
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F(" => set new target brightness: "));
    debug_uart_main->println(newTargetBrightness);
    debug_uart_main->print(F(" => set next state to "));
    debugPrintStateText(START_TRANSIT_TO_ON, true);
  }
  _targetBrightness = newTargetBrightness;
  _state = START_TRANSIT_TO_ON;
}

// switch on LED strip with maximum brightness
void ctrlDblClickOn(uint8_t btn)
{
  if (_targetBrightness < maxBrightness())
  {
    _targetBrightness = maxBrightness();
  }
  else
  {
    _targetBrightness = _onBrightness;
  }
  _state = START_TRANSIT_TO_ON;
}

// save currently selected brightness as default brightness
void ctrlLongClickOn(int8_t btn)
{
  // do not save "off" as default brightness
  if (_targetBrightness == 0)
    return;

  // either save night light brightness or on brightness
  // but only, when the final brightness has been reached
  switch (_state)
  {
  case NIGHT_LIGHT_ON:
    if (_targetBrightness != _nightLightBrightness)
    {
      setNightLightBrightness(_targetBrightness);
    }
    break;
  case ON:
    if (_targetBrightness != onBrightness())
    {
      setOnBrightness(_targetBrightness);
    }
    break;
  default:
    // default brightness not saved, no confirmation required
    return;
  }
  // confirm whether or not the preference has actually changed
  confirmPreferenceChanged();
}

// ---------------------
// ---- PLUS button ----
// ---------------------

void ctrlClickPlus(uint8_t btn)
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("PLUS button clicked in state "));
    debugPrintStateText(_state, true);
  }

  bool isOffState = false;

  switch (_state)
  {
  case START_TRANSIT_TO_NIGHT_LIGHT:
  case TRANSIT_TO_NIGHT_LIGHT:
  case NIGHT_LIGHT_ON:
  {
    // calculate new brightness, cap at half brightness
    uint8_t newVal = _nightLightBrightness + stepBrightness();
    uint8_t newNightLightBrightness = (newVal > 128) ? _nightLightBrightness : newVal;
    if (newNightLightBrightness > _nightLightBrightness)
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->print(F(" => increase night light brightness from "));
        debug_uart_main->print(_nightLightBrightness);
        debug_uart_main->print(F(" to "));
        debug_uart_main->println(newNightLightBrightness);
        debug_uart_main->print(F(" => set state to "));
        debugPrintStateText(START_TRANSIT_TO_NIGHT_LIGHT, true);
      }
      _nightLightBrightness = newNightLightBrightness;
      _state = START_TRANSIT_TO_NIGHT_LIGHT;
    }
    else
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->println(F(" night light brightness is already at maximum => ignored"));
      }
    }
    break;
  }
  case START_TRANSIT_TO_OFF:
  case TRANSIT_TO_OFF:
  case OFF:
    isOffState = true;
    // fall through
  case START_TRANSIT_TO_ON:
  case TRANSIT_TO_ON:
  case ON:
  {
    // calculate new brightness, avoid overflow
    uint8_t newTargetBrightness;
    if (isOffState)
    {
      newTargetBrightness = stepBrightness();
    }
    else
    {
      uint16_t a = (uint16_t)_targetBrightness + stepBrightness();
      newTargetBrightness = (a > maxBrightness()) ? _targetBrightness : (uint8_t)a;
    }
    if (newTargetBrightness > _targetBrightness)
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->print(F(" => increase brightness from "));
        debug_uart_main->print(_targetBrightness);
        debug_uart_main->print(F(" to "));
        debug_uart_main->println(newTargetBrightness);
        debug_uart_main->print(F(" => set state to "));
        debugPrintStateText(START_TRANSIT_TO_ON, true);
      }
      _targetBrightness = newTargetBrightness;
      _onBrightness = newTargetBrightness;
      _state = START_TRANSIT_TO_ON;
    }
    else
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->println(F(" brightness is already at maximum => ignored"));
      }
    }
  }
  }
}

// ----------------------
// ---- MINUS button ----
// ----------------------

void ctrlClickMinus(uint8_t btn)
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("MINUS button clicked in state "));
    debugPrintStateText(_state, true);
  }

  switch (_state)
  {
  case START_TRANSIT_TO_NIGHT_LIGHT:
  case TRANSIT_TO_NIGHT_LIGHT:
  case NIGHT_LIGHT_ON:
  {
    // calculate new brightness, cap at half brightness
    uint8_t step = stepBrightness();
    uint8_t newVal = _nightLightBrightness - step;
    uint8_t newNightLightBrightness = (newVal < step) ? _nightLightBrightness : newVal;
    if (newNightLightBrightness < _nightLightBrightness)
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->print(F(" => decrease night light brightness from "));
        debug_uart_main->print(_nightLightBrightness);
        debug_uart_main->print(F(" to "));
        debug_uart_main->println(newNightLightBrightness);
        debug_uart_main->print(F(" => set state to "));
        debugPrintStateText(START_TRANSIT_TO_NIGHT_LIGHT, true);
      }
      _nightLightBrightness = newNightLightBrightness;
      _state = START_TRANSIT_TO_NIGHT_LIGHT;
    }
    else
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->println(F(" night light brightness is already at minimum => ignored"));
      }
    }
    break;
  }
  case START_TRANSIT_TO_OFF:
  case TRANSIT_TO_OFF:
  case OFF:
    if (debug_uart_main != nullptr)
    {
      debug_uart_main->println(F("  light is already off => ignored"));
    }
    break;
  case START_TRANSIT_TO_ON:
  case TRANSIT_TO_ON:
  case ON:
    // calculate new brightness
    uint8_t step = stepBrightness();
    int16_t a = (int16_t)_targetBrightness - step;

    // avoid underflow
    if (a < step)
    {
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->print(F(" => decrease brightness to "));
        debug_uart_main->print(a);
        debug_uart_main->print(F(", min value is "));
        debug_uart_main->println(step);
        debug_uart_main->println(F(" => set state to START_TRANSIT_TO_OFF"));
      }
      _state = START_TRANSIT_TO_OFF;
    }
    else
    {
      uint8_t newTargetBrightness = (uint8_t)a;
      if (debug_uart_main != nullptr)
      {
        debug_uart_main->print(F(" => decrease brightness from "));
        debug_uart_main->print(_targetBrightness);
        debug_uart_main->print(F(" to "));
        debug_uart_main->println(newTargetBrightness);
        debug_uart_main->print(F(" => set state to "));
        debugPrintStateText(START_TRANSIT_TO_ON, true);
      }
      _targetBrightness = newTargetBrightness;
      _onBrightness = newTargetBrightness;
      _state = START_TRANSIT_TO_ON;
    }
  }
}

// --------------------
// ---- OFF button ----
// --------------------

// switch off LED strip when it is on, OR
// disallow night light if lamp is in night light mode OR
// enable night light if lamp is off
void ctrlClickOff(uint8_t btn)
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("OFF button clicked in state "));
    debugPrintStateText(_state, true);
  }

  switch (_state)
  {
  case NIGHT_LIGHT_ON:
    setNightLightEnabled(false);
    break;

  case OFF:
  {
    bool changed = setNightLightEnabled(true);
    if (changed)
    {
      confirmPreferenceChanged();
    }
    if (debug_uart_main != nullptr)
    {
      debug_uart_main->println(changed ? F("enabled night light mode") : F("night light mode not changed, stays enabled"));
    }
    break;
  }

  case ON:
    _state = enableNightLight() ? START_TRANSIT_TO_NIGHT_LIGHT : START_TRANSIT_TO_OFF;
    if (debug_uart_main != nullptr)
    {
      debug_uart_main->print(F(" => set next state to "));
      debugPrintStateText(_state, true);
    }
    break;

  default:
    break;
  }
}

// toggle "allow night light"
void ctrlDblClickOff(uint8_t btn)
{
  setNightLightEnabled(!isNightLightEnabled());
  confirmPreferenceChanged();
}

// save state of "allow night light"
void ctrlLongClickOff(int8_t btn)
{
  if (isNightLightEnabled() != allowNightLight())
  {
    setAllowNightLight(isNightLightEnabled());
  }
  // confirm whether or not the preference has actually changed
  confirmPreferenceChanged();
}

void setup()
{
  MONITOR_SERIAL.begin(115200);
  delay(50);
  MONITOR_SERIAL.println(F("Start setup"));

  debug_uart_main = &MONITOR_SERIAL;

  configSetup();
  _nightLightOnDuration = nightLightOnDuration() * 1000;
  _nightLightBrightness = nightLightBrightness();
  _onBrightness = onBrightness();
  setNightLightEnabled(allowNightLight());

  // touchDebug(MONITOR_SERIAL);
  touchSetup();

  ldrSetup();

  // presenceDebug(MONITOR_SERIAL);
  presenceSetup();

  ledStripDebug(MONITOR_SERIAL);
  ledStripSetup();

  // --- ON button ---
  // normal click: switch LED strip on
  setClickHandler(OnButton, ctrlClickOn);
  // normal click: switch LED strip on AND set max brightness
  setDoubleClickHandler(OnButton, ctrlDblClickOn);
  // long click: save current brightness as default
  // setLongClickHandler(OnButton, ctrlLongClickOn);

  // --- PLUS button ---
  // normal click: increase brightness one step
  setClickHandler(PlusButton, ctrlClickPlus);
  // long click: increase brightness one step as long as the button is pressed
  setLongClickHandler(PlusButton, ctrlClickPlus);

  // --- MINUS button ---
  // normal click: decrease brightness one step
  setClickHandler(MinusButton, ctrlClickMinus);
  // long click: decrease brightness one step as long as the button is pressed
  setLongClickHandler(MinusButton, ctrlClickMinus);

  // --- OFF button ---
  // normal click: switch LED strip off
  setClickHandler(OffButton, ctrlClickOff);
  // setDoubleClickHandler(OffButton, ctrlDblClickOff);
  // setLongClickHandler(OffButton, ctrlLongClickOff);

  MONITOR_SERIAL.println(F("setup done"));
}

// when lamp is off or turning off: check whether the night light should be switched on
State nightLightCheckOff()
{
  return enableNightLight() ? START_TRANSIT_TO_NIGHT_LIGHT : OFF;
}

// when night light is on: check whether the night light can be switched off again
State nightLightCheckOn()
{
  // update the timestamp of the last presence detection
  if (_prsInfo.presenceDetected)
  {
    _nightLightEnabledTs = millis();
  }

  // calculate the duration with no presence detection since switching on the night light
  unsigned long noPresenceDuration = millis() - _nightLightEnabledTs;

  // switch off the night light when no presence has been detected for long enough, otherwise leave it turned on
  return (!isNightLightEnabled() || noPresenceDuration > _nightLightOnDuration) ? START_TRANSIT_TO_OFF : NIGHT_LIGHT_ON;
}

// set the new target brightness and
// trigger the brightness change
State startTransitToOn()
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("set target brightness "));
    debug_uart_main->print(_targetBrightness);
    debug_uart_main->println(F(" on LED strip"));
  }
  ledStripSetTargetBrightness(_targetBrightness);
  return TRANSIT_TO_ON;
}

// stay in TRANSIT_TO_ON as long as the target brightness has not been reached, then switch to ON
State transitToOn()
{
  uint8_t ledBrightness = ledStripCurrentBrightness();
  uint8_t target = ledStripTargetBrightness();
  bool onBrightnessReached = (ledBrightness == target);
  /*
  if ((debug_uart_main != nullptr) && (_previousBrightness != ledBrightness))
  {
    debug_uart_main->print(F("brightness on LED strip: "));
    debug_uart_main->print(ledBrightness);
    debug_uart_main->print(F(" / "));
    debug_uart_main->println(target);
    _previousBrightness = onBrightnessReached ? 0 : ledBrightness;
  }
  */
  return onBrightnessReached ? ON : _state;
}

// when lamp is on and to be switched "off", check whether "off" means really off or night light
State offCheck()
{
  return enableNightLight() ? START_TRANSIT_TO_NIGHT_LIGHT : START_TRANSIT_TO_OFF;
}

// trigger the brightness change to 0
State startTransitToOff()
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("set target brightness 0 on LED strip"));
  }
  _targetBrightness = 0;
  ledStripSetTargetBrightness(0);
  return TRANSIT_TO_OFF;
}

// stay in TRANSIT_TO_OFF as long as the target brightness (0) has not been reached, then switch to OFF
State transitToOff()
{
  uint8_t ledBrightness = ledStripCurrentBrightness();
  if ((debug_uart_main != nullptr) && (_previousBrightness != ledBrightness))
  {
    debug_uart_main->print(F("brightness on LED strip: "));
    debug_uart_main->print(ledBrightness);
    debug_uart_main->println(F(" / 0"));
    _previousBrightness = ledBrightness;
  }
  return (ledBrightness == 0) ? OFF : _state;
}

// trigger the brightness change to night light brightness
State startTransitToNightLight()
{
  if (debug_uart_main != nullptr)
  {
    debug_uart_main->print(F("set target brightness "));
    debug_uart_main->print(_nightLightBrightness);
    debug_uart_main->println(F(" on LED strip"));
  }
  ledStripSetTargetBrightness(_nightLightBrightness);
  return TRANSIT_TO_NIGHT_LIGHT;
}

// stay in TRANSIT_TO_NIGHT_LIGHT as long as the target brightness has not been reached, then switch to NIGHT_LIGHT_ON
State transitToNightLight()
{
  return (ledStripCurrentBrightness() == _nightLightBrightness) ? NIGHT_LIGHT_ON : _state;
}

void handleState()
{
  State nextState;

  switch (_state)
  {
  case OFF:
    nextState = nightLightCheckOff();
    break;
  case START_TRANSIT_TO_ON:
    nextState = startTransitToOn();
    break;
  case TRANSIT_TO_ON:
    nextState = transitToOn();
    break;
  case ON:
    // nothing to do, wait for a pressed button
    return;
  case OFF_CHECK:
    nextState = offCheck();
    break;
  case START_TRANSIT_TO_OFF:
    nextState = startTransitToOff();
    break;
  case TRANSIT_TO_OFF:
    nextState = transitToOff();
    break;
  case START_TRANSIT_TO_NIGHT_LIGHT:
    nextState = startTransitToNightLight();
    break;
  case TRANSIT_TO_NIGHT_LIGHT:
    nextState = transitToNightLight();
    break;
  case NIGHT_LIGHT_ON:
    nextState = nightLightCheckOn();
    break;
  default:
    break;
  }
  bool confirm = false;
  if (debug_uart_main != nullptr && _state != nextState)
  {
    debug_uart_main->print(F("change state from "));
    debugPrintStateText(_state);
    debug_uart_main->print(F(" to "));
    debugPrintStateText(nextState, true);
    confirm = true;
  }
  _state = nextState;
  if (confirm)
  {
    debug_uart_main->print(F("state is now "));
    debugPrintStateText(_state, true);
  }
}

void loop()
{
  configLoop();

  // read ldr
  ldrLoop();
  _ldrValue = averagedBrightness();

  // read radar presence sensor
  presenceLoop();
  _prsInfo = presenceInfo();

  // check touch buttons
  touchLoop();

  // see whether buttons or ldr or presence sensor require a state change
  handleState();

  // set LED strip accordingly
  ledStripLoop();

  // debug
  // reportSerial();
}

// emit a status report
void reportSerial()
{
  unsigned long now = millis();
  if (now - _lastReport > 5000)
  {
    _lastReport = now;
    bool darkEnoughForNightLight = isDarkEnoughForNightLight();
    bool enableNightLight = darkEnoughForNightLight && _prsInfo.presenceDetected;

    MONITOR_SERIAL.print(F("ldr: "));
    MONITOR_SERIAL.print(_ldrValue);
    MONITOR_SERIAL.print(F("dark enough for night light: "));
    MONITOR_SERIAL.println(darkEnoughForNightLight);

    MONITOR_SERIAL.print(F("enable night light: "));
    MONITOR_SERIAL.println(enableNightLight);

    if (_prsInfo.presenceDetected)
    {
      if (_prsInfo.stationaryTargetDetected)
      {
        MONITOR_SERIAL.print(F("Stationary target: "));
        MONITOR_SERIAL.print(_prsInfo.stationaryTargetDistance);
        MONITOR_SERIAL.print(F("cm energy:"));
        MONITOR_SERIAL.print(_prsInfo.stationaryTargetEnergy);
        MONITOR_SERIAL.print(' ');
      }
      if (_prsInfo.movingTargetDetected)
      {
        MONITOR_SERIAL.print(F("Moving target: "));
        MONITOR_SERIAL.print(_prsInfo.movingTargetDistance);
        MONITOR_SERIAL.print(F("cm energy:"));
        MONITOR_SERIAL.print(_prsInfo.movingTargetEnergy);
      }
      MONITOR_SERIAL.println();
    }
    else
    {
      MONITOR_SERIAL.println(F("No target"));
    }

    if (!_cfgDone)
    {
      LD2410Config presenceCfg = currentConfig();
      if (presenceCfg.Valid)
      {
        MONITOR_SERIAL.print(F("LD2410 config: \nmax_gate:"));
        MONITOR_SERIAL.println(presenceCfg.max_gate);
        MONITOR_SERIAL.print(F("max_moving_gate: "));
        MONITOR_SERIAL.println(presenceCfg.max_moving_gate);
        MONITOR_SERIAL.print(F("max_stationary_gate: "));
        MONITOR_SERIAL.println(presenceCfg.max_stationary_gate);
        MONITOR_SERIAL.print(F("sensor_idle_time: "));
        MONITOR_SERIAL.println(presenceCfg.sensor_idle_time);

        MONITOR_SERIAL.print(F("motion_sensitivity: "));
        for (uint8_t i = 0; i < presenceCfg.max_moving_gate; i++)
        {
          MONITOR_SERIAL.print(presenceCfg.motion_sensitivity[i]);
          if (i < presenceCfg.max_moving_gate - 1)
            MONITOR_SERIAL.print(F(", "));
        }
        MONITOR_SERIAL.println();

        MONITOR_SERIAL.print(F("motion_sensitivity: "));
        for (uint8_t i = 0; i < presenceCfg.max_moving_gate; i++)
        {
          MONITOR_SERIAL.print(presenceCfg.stationary_sensitivity[i]);
          if (i < presenceCfg.max_moving_gate - 1)
            MONITOR_SERIAL.print(F(", "));
        }
        MONITOR_SERIAL.println();
        _cfgDone = true;
      }
    }
    else
    {
      MONITOR_SERIAL.println(F("No config yet"));
    }

    if (!_fwDone)
    {
      LD2410Firmware fwInfo = firmwareInfo();
      if (fwInfo.Valid)
      {
        _fwDone = true;
        MONITOR_SERIAL.print(F("LD2410 firmware: v"));
        MONITOR_SERIAL.print(fwInfo.Major);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.print(fwInfo.Minor);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.println(fwInfo.Bugfix);
      }
    }
    else
    {
      MONITOR_SERIAL.println(F("No firmware info yet"));
    }
  }
}