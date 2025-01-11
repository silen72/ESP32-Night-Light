#include "device_state.h"

#include <WebServer.h>
#include <Button2.h>

#include <device_common.h>
#include <device_state.h>
#include <touch.h>
#include <wifi_handler.h>
#include <ldr.h>
#include <led_strip.h>
#include <config.h>
#include <presence.h>
#include <webapi.h>
#include <webinterface.h>

State _state = State::OFF; // initial state is always OFF -> no light

bool _allowNightLightMode = true;        // Whether night light should be turned on when it is dark enough and presence is detected
unsigned long _nightLightEnabledTs = 0;  // Timestamp, when night light was switched on
unsigned long _nightLightOnDuration = 0; // The duration the night light stays on even when presence is no longer detected
uint8_t _nightLightBrightness = 0;       // Night light will be set to this brightness when enabled
uint8_t _maxNightLightBrightness = 0;    // Maximum allowed night light brightness
uint8_t _onBrightness = 0;               // Light will be set to this brightness when enabled
uint8_t _maxBrightness = 0;              // Maximum allowed brightness
uint8_t _stepBrightness = 0;             // Brightness will be in/decreased by this step value
uint8_t _targetBrightness = 0;           // The target brightness. When it differs from the actual brightness, the actual brightness will be gradually modified to be equal.
bool _ignoreMinusLongClick = false;      // To avoid unintentionally decreasing the night light brightness after decreasing regular brightness.

unsigned long _lastTripleClickTs = 0;
uint8_t _factoryResetCount = 0;
uint8_t _forceApModeCount = 0;

uint16_t _transitionDurationMs = 0; // A smooth transition between the current and a target brightness will be finished within this duration.

uint16_t _ldrValue = 0;            // The current measurement coming from the ldr (dark 0 .. 4095 bright). Gets updated every loop cycle.
uint16_t _nightLightThreshold = 0; // LDR values equal or less than this value warrant switching the night light on.

LD2410Detection _prsInfo; // The current measurement coming from the radar presence sensor. Gets updated every loop cycle.
uint16_t _maxMovingTargetDistance = 0;
uint16_t _minMovingTargetDistance = 0;
uint8_t _maxMovingTargetEnergy = 0;
uint8_t _minMovingTargetEnergy = 0;
uint16_t _maxStationaryTargetDistance = 0;
uint16_t _minStationaryTargetDistance = 0;
uint8_t _maxStationaryTargetEnergy = 0;
uint8_t _minStationaryTargetEnergy = 0;

// Debugging
Stream *_debugUartMain = nullptr;     // The stream used for the debugging
uint8_t _debugPreviousBrightness = 0; // Used for detecting a change in brightness on the led strip to report a new value only once.

const ButtonNumber OnButton = ButtonNumber::FOUR;    // The ON touch button.
const ButtonNumber PlusButton = ButtonNumber::THREE; // The PLUS touch button.
const ButtonNumber MinusButton = ButtonNumber::TWO;  // The MINUS touch button.
const ButtonNumber OffButton = ButtonNumber::ONE;    // The OFF touch button.

/*
  -------------------------
  ---- DEBUG functions ----
  -------------------------
*/

// debug only: send the name of a button to the debug stream
void debugPrintButtonName(ButtonNumber btn, bool addPrintln = false)
{
  if (_debugUartMain != nullptr)
  {
    switch (btn)
    {
    case OffButton:
      _debugUartMain->print(F("OFF"));
      break;
    case MinusButton:
      _debugUartMain->print(F("MINUS"));
      break;
    case PlusButton:
      _debugUartMain->print(F("PLUS"));
      break;
    case OnButton:
      _debugUartMain->print(F("ON"));
      break;
    default:
      _debugUartMain->print(F("ERROR! unknown button"));
      break;
    }
    if (addPrintln)
      _debugUartMain->println();
  }
}

// debug only: send the name of a clickType to the debug stream
void debugPrintClickType(clickType type, bool addPrintln = false)
{
  if (_debugUartMain != nullptr)
  {
    switch (type)
    {
    case single_click:
      _debugUartMain->print(F("click"));
      break;
    case double_click:
      _debugUartMain->print(F("double click"));
      break;
    case triple_click:
      _debugUartMain->print(F("triple click"));
      break;
    case long_click:
      _debugUartMain->print(F("long click"));
      break;
    case empty:
      _debugUartMain->print(F("empty"));
      break;
    default:
      _debugUartMain->print(F("ERROR! unknown click type"));
      break;
    }
    if (addPrintln)
      _debugUartMain->println();
  }
}

void debugPrintStateText(Stream *stream, State state, bool addPrintln)
{
  if (stream != nullptr)
  {
    switch (state)
    {
    case OFF:
      stream->print(F("OFF"));
      break;
    case START_TRANSIT_TO_ON:
      stream->print(F("START_TRANSIT_TO_ON"));
      break;
    case TRANSIT_TO_ON:
      stream->print(F("TRANSIT_TO_ON"));
      break;
    case ON:
      stream->print(F("ON"));
      break;
    case START_TRANSIT_TO_OFF:
      stream->print(F("START_TRANSIT_TO_OFF"));
      break;
    case TRANSIT_TO_OFF:
      stream->print(F("TRANSIT_TO_OFF"));
      break;
    case START_TRANSIT_TO_NIGHT_LIGHT:
      stream->print(F("START_TRANSIT_TO_NIGHT_LIGHT"));
      break;
    case TRANSIT_TO_NIGHT_LIGHT:
      stream->print(F("TRANSIT_TO_NIGHT_LIGHT"));
      break;
    case NIGHT_LIGHT_ON:
      stream->print(F("NIGHT_LIGHT_ON"));
      break;
    default:
      stream->print(F("ERROR! unknown state"));
      break;
    }
    if (addPrintln)
      stream->println();
  }
}

// debug only: send the name of a state to the debug stream
void debugPrintStateText(State state, bool addPrintln = false) { debugPrintStateText(_debugUartMain, state, addPrintln); }

void debugPrintBool(bool value, bool addPrintln = false)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(value ? F("true") : F("false"));
    if (addPrintln)
      _debugUartMain->println();
  }
}

void debugClickIgnored()
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->println(F(" => ignored"));
  }
}

void debugClickIgnored(const char *reason)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(F(" "));
    _debugUartMain->print(reason);
    _debugUartMain->println(F(" => ignored"));
  }
}

void debugClickIgnored(const __FlashStringHelper *ifsh) { debugClickIgnored(reinterpret_cast<const char *>(ifsh)); }

void debugPlusMinus(bool isIncrease, bool isNightLight, uint8_t from, uint8_t to)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(isIncrease ? F(" => increase") : F(" => decrease"));
    if (isNightLight)
    {
      _debugUartMain->print(F(" night light"));
    }
    _debugUartMain->print(F(" brightness from "));
    _debugUartMain->print(from);
    _debugUartMain->print(F(" to "));
    _debugUartMain->println(to);
  }
}
void debugPlus(bool isNightLight, uint8_t from, uint8_t to) { debugPlusMinus(true, isNightLight, from, to); }
void debugMinus(bool isNightLight, uint8_t from, uint8_t to) { debugPlusMinus(false, isNightLight, from, to); }

void debugButtonAndState(ButtonNumber btn, clickType type)
{
  if (_debugUartMain != nullptr)
  {
    debugPrintButtonName(btn);
    _debugUartMain->print(F(" "));
    debugPrintClickType(type);
    _debugUartMain->print(F("ed in state "));
    debugPrintStateText(_state, true);
  }
}

void debugNightLightMode(bool changed, bool state)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->println(changed ? F("enabled night light mode") : F("night light mode not changed, stays enabled"));
  }
}

/*
  ---------------------------
  ---- Wrapper functions ----
  ---------------------------
*/

// Enable or disable the night light mode. When disabled, night light will not be switched on.
// Returns true, when the mode has changed.
bool setNightLightModeAllowed(bool value = true)
{
  bool changed = value != _allowNightLightMode;
  if (changed)
  {
    if (_debugUartMain != nullptr)
    {
      _debugUartMain->print(F("set allow night light mode to "));
      debugPrintBool(value, true);
    }
    _allowNightLightMode = value;
  }
  else if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(F("allow night light mode not changed, stays "));
    debugPrintBool(value, true);
  }
  return changed;
}

// Checks whether the ldr measurement is low enough to warrant switching the night light on.
bool isDarkEnoughForNightLight()
{
  return _ldrValue <= _nightLightThreshold;
}

bool isMovingTargetDetected()
{
  if (!_prsInfo.movingTargetDetected)
    return false;
  uint16_t dist = _prsInfo.movingTargetDistance;
  uint8_t energy = _prsInfo.movingTargetEnergy;
  return dist <= _maxMovingTargetDistance &&
         dist >= _minMovingTargetDistance &&
         energy <= _maxMovingTargetEnergy &&
         energy >= _minMovingTargetEnergy;
}

bool isStationaryTargetDetected()
{
  if (!_prsInfo.stationaryTargetDetected)
    return false;
  uint16_t dist = _prsInfo.stationaryTargetDistance;
  uint8_t energy = _prsInfo.stationaryTargetEnergy;
  return dist <= _maxStationaryTargetDistance &&
         dist >= _minStationaryTargetDistance &&
         energy <= _maxStationaryTargetEnergy &&
         energy >= _minStationaryTargetEnergy;
}

// Checks whether presence has been detected within the preferred confinements
bool isPresenceDetected()
{
  return (_prsInfo.presenceDetected && (isMovingTargetDetected() || isStationaryTargetDetected()));
}

// Checks whether the night light should actually switched on.
bool enableNightLight()
{
  return _allowNightLightMode && isDarkEnoughForNightLight() && isPresenceDetected();
}

// ToDo: add a function that gives distinguishable confirmations for "on" and "off"

// Let the led strip blink briefly to give a visual confirmation
void confirmViaLEDStrip(bool alsoConfirmValue = false, bool value = false)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->println(F("confirm otherwise invisible change via LED strip"));
  }
  ledStripConfirm(alsoConfirmValue, value);
}

// Set a target brightness
void setTargetBrightness(uint8_t newTargetBrightness)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(F(" => change target brightness from "));
    _debugUartMain->println(_targetBrightness);
    _debugUartMain->print(F(" to "));
    _debugUartMain->println(newTargetBrightness);
  }
  _targetBrightness = newTargetBrightness;
}

// Set a new state for the state machine
void setState(State newState)
{
  if (_debugUartMain != nullptr && _state != newState)
  {
    _debugUartMain->print(F(" => change state from "));
    debugPrintStateText(_state);
    _debugUartMain->print(F(" to "));
    debugPrintStateText(newState, true);
  }
  _state = newState;
}

// Set _targetBrightness as actual brightness on the led strip
void setLEDStripBrightness()
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(F("set target brightness "));
    _debugUartMain->print(_targetBrightness);
    _debugUartMain->println(F(" on LED strip"));
  }
  ledStripSetTargetBrightness(_targetBrightness);
}

// Enable or disable ignoring a long click on the MINUS button.
// After decreasing the brightness to OFF the night light might kick in immediately.
// Without ignoring the then still pressed MINUS button, the user would start decreasing the
// night light brightness, which is not intended.
void setIgnoreMinusLongClick(bool value)
{
  if (_debugUartMain != nullptr)
  {
    _debugUartMain->print(F(" => change ignore MINUS long click to "));
    debugPrintBool(value, true);
  }
  _ignoreMinusLongClick = value;
}

/*
 -------------------
 ---- ON button ----
 -------------------
 */

// Handle single click on the ON button:
// switch on LED strip with previous brightness
void ctrlClickOn(uint8_t btn)
{
  debugButtonAndState(OnButton, single_click);

  switch (_state)
  {
  // do nothing when transitioning to on
  case START_TRANSIT_TO_ON:
  case TRANSIT_TO_ON:
    debugClickIgnored();
    break;
  // when on toggle between max brightness and configured brightness
  case ON:
    setTargetBrightness(_targetBrightness < _maxBrightness ? _maxBrightness : _onBrightness);
    setState(START_TRANSIT_TO_ON);
    return;
  }
  setTargetBrightness(_onBrightness);
  setState(START_TRANSIT_TO_ON);
}

// Handle double click on the ON button:
// switch on LED strip with maximum brightness
void ctrlDblClickOn(uint8_t btn)
{
  debugButtonAndState(OnButton, double_click);
  setTargetBrightness(_targetBrightness < _maxBrightness ? _maxBrightness : _onBrightness);
  setState(START_TRANSIT_TO_ON);
}

// Handle long click (2000ms+) on the ON button:
// save currently selected brightness as default brightness
// dependend on current state either as default brightness for normal mode or
// as default brightness for night light mode or
void ctrlLongClickOn(int8_t btn)
{
  debugButtonAndState(OnButton, long_click);
  // do not save "off" as default brightness
  if (_targetBrightness == 0)
  {
    debugClickIgnored(F("off cannot be set as default brightness"));
    return;
  }

  // either save night light brightness or on brightness
  // but only, when the final brightness has been reached
  switch (_state)
  {
  case NIGHT_LIGHT_ON:
    if (_targetBrightness != nightLightBrightness())
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
  // saving the current brightness gives no visual feedback => blink as confirmation
  // confirm whether or not the preference has actually changed
  confirmViaLEDStrip();
}

// ----------------------------------------------
// ---- PLUS button, single and long click ----
// ----------------------------------------------

// Increase the brightness of the current state
// ON or OFF: increase the brightness one step (up to maxBrightness)
// NIGHT_LIGHT: increase the night light brightness one step (up to maxNightLightBrightness)
void handlePlus(uint8_t btn)
{
  bool isOffState = false;

  switch (_state)
  {
  case START_TRANSIT_TO_NIGHT_LIGHT:
  case TRANSIT_TO_NIGHT_LIGHT:
  case NIGHT_LIGHT_ON:
  {
    // calculate new brightness, cap at half brightness
    uint8_t newVal = _nightLightBrightness + _stepBrightness;
    uint8_t newNightLightBrightness = (newVal > _maxNightLightBrightness) ? _nightLightBrightness : newVal;
    if (newNightLightBrightness > _nightLightBrightness)
    {
      debugPlus(true, _nightLightBrightness, newNightLightBrightness);
      _nightLightBrightness = newNightLightBrightness;
      setState(START_TRANSIT_TO_NIGHT_LIGHT);
    }
    else
    {
      debugClickIgnored(F("night light brightness is already at maximum"));
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
      newTargetBrightness = _stepBrightness;
    }
    else
    {
      uint16_t a = (uint16_t)_targetBrightness + _stepBrightness;
      newTargetBrightness = (a > _maxBrightness) ? _targetBrightness : (uint8_t)a;
    }

    if (newTargetBrightness > _targetBrightness)
    {
      debugPlus(false, _targetBrightness, newTargetBrightness);
      setTargetBrightness(newTargetBrightness);
      _onBrightness = newTargetBrightness;
      setState(START_TRANSIT_TO_ON);
    }
    else
    {
      debugClickIgnored(F("brightness is already at maximum"));
    }
  }
  }
}

// Handle single click on the PLUS button:
// Increase the brightness of the current mode one step
// ON or OFF: increase the brightness (up to maxBrightness)
// NIGHT_LIGHT: increase the night light brightness (up to maxNightLightBrightness)
void ctrlClickPlus(uint8_t btn)
{
  debugButtonAndState(PlusButton, single_click);
  handlePlus(btn);
}

// Handle long click on the PLUS button:
// Increase the brightness of the current mode as long as the button is pressed
// ON or OFF: increase the brightness one step (up to maxBrightness)
// NIGHT_LIGHT: increase the night light brightness one step (up to maxNightLightBrightness)
void ctrlLongClickPlus(uint8_t btn)
{
  debugButtonAndState(PlusButton, long_click);
  handlePlus(btn);
}

void ctrlTripleClick(ButtonNumber btnn)
{
  debugButtonAndState(btnn, triple_click);

  // reset counter when last triple click was too long ago
  auto now = millis();
  if (now - _lastTripleClickTs > 2000)
  {
    if ((_factoryResetCount > 0 || _forceApModeCount > 0) && _debugUartMain != nullptr)
    {
      _debugUartMain->println(F("Resetting triple click counter."));
    }
    _factoryResetCount = 0;
    _forceApModeCount = 0;
  }

  // detect sequence for factory reset
  //  1.) triple click on OFF
  //  2.) triple click on PLUS
  //  3.) triple click on MINUS
  //  4.) triple click on MINUS
  bool isFactoryReset =
      ((_factoryResetCount == 0) && (btnn == OffButton)) ||
      ((_factoryResetCount == 1) && (btnn == PlusButton)) ||
      ((_factoryResetCount == 2) && (btnn == MinusButton)) ||
      ((_factoryResetCount == 3) && (btnn == MinusButton));

  // detect sequence for forced AP mode
  //  1.) triple click on PLUS
  //  2.) triple click on MINUS
  //  3.) triple click on PLUS
  bool isForceAp = !isFactoryReset && (((_forceApModeCount == 0) && (btnn == PlusButton)) ||
                                       ((_forceApModeCount == 1) && (btnn == MinusButton)) ||
                                       ((_forceApModeCount == 2) && (btnn == PlusButton)));

  _factoryResetCount = isFactoryReset ? _factoryResetCount + 1 : 0; // advance or reset
  if (isFactoryReset && _debugUartMain != nullptr)
  {
    _debugUartMain->print(F(" => Factory reset count increased to "));
    _debugUartMain->print(_factoryResetCount);
    _debugUartMain->println(F(" of 4"));
    _forceApModeCount = 0;
  }

  _forceApModeCount = isForceAp ? _forceApModeCount + 1 : 0; // advance or reset
  if (isForceAp && _debugUartMain != nullptr)
  {
    _debugUartMain->print(F(" => Force AP count increased to "));
    _debugUartMain->print(_forceApModeCount);
    _debugUartMain->println(F(" of 3"));
    _factoryResetCount = 0;
  }

  if (_factoryResetCount == 4)
  {
    if (_debugUartMain != nullptr)
    {
      _debugUartMain->println(F("Factory reset. Device will restart."));
    }
    factoryReset();
    ESP.restart();
  }
  if (_forceApModeCount == 3)
  {
    if (_debugUartMain != nullptr)
    {
      _debugUartMain->println(F("Forcing AP mode."));
    }
    confirmViaLEDStrip();
    requestAPMode();
  }

  _lastTripleClickTs = isFactoryReset || isForceAp ? now : 0;
}

void ctrlTripleClickPlus(uint8_t btn) { ctrlTripleClick(PlusButton); }
void ctrlTripleClickMinus(uint8_t btn) { ctrlTripleClick(MinusButton); }
void ctrlTripleClickOff(uint8_t btn) { ctrlTripleClick(OffButton); }

// ----------------------
// ---- MINUS button ----
// ----------------------

// Decrease the brightness of the current state
// ON: Decrease the brightness one step (down to stepBrightness, then to OFF)
// NIGHT_LIGHT: Decrease the night light brightness one step (down to stepBrightness)
//              Will not decrease night light brightness when immediately before
void handleMinus(uint8_t btn)
{
  switch (_state)
  {
  case START_TRANSIT_TO_NIGHT_LIGHT:
  case TRANSIT_TO_NIGHT_LIGHT:
  case NIGHT_LIGHT_ON:
  {
    // calculate new brightness, cap at half brightness
    uint8_t step = _stepBrightness;
    uint8_t newVal = _nightLightBrightness - step;
    uint8_t newNightLightBrightness = (newVal < step) ? _nightLightBrightness : newVal;
    if (_ignoreMinusLongClick)
    {
      debugClickIgnored(F("button still pressed after decreasing on brightness to OFF"));
    }
    else if (newNightLightBrightness < _nightLightBrightness)
    {
      debugMinus(true, _nightLightBrightness, newNightLightBrightness);
      _nightLightBrightness = newNightLightBrightness;
      setState(START_TRANSIT_TO_NIGHT_LIGHT);
    }
    else
    {
      debugClickIgnored(F("night light brightness is already at minimum"));
    }
    break;
  }
  case START_TRANSIT_TO_OFF:
  case TRANSIT_TO_OFF:
  case OFF:
    debugClickIgnored(F("light is already off"));
    break;
  case START_TRANSIT_TO_ON:
  case TRANSIT_TO_ON:
  case ON:
    // calculate new brightness
    uint8_t step = _stepBrightness;
    int16_t a = (int16_t)_targetBrightness - step;

    // avoid underflow
    if (a < step)
    {
      if (_debugUartMain != nullptr)
      {
        _debugUartMain->print(F(" => decrease brightness to "));
        _debugUartMain->print(a);
        _debugUartMain->print(F(", min value is "));
        _debugUartMain->println(step);
      }
      setIgnoreMinusLongClick(true); // avoid decreasing night light brightness also simply by holding MINUS too long
      setState(START_TRANSIT_TO_OFF);
    }
    else
    {
      uint8_t newTargetBrightness = (uint8_t)a;
      debugMinus(false, _targetBrightness, newTargetBrightness);
      setTargetBrightness(newTargetBrightness);
      _onBrightness = newTargetBrightness;
      setState(START_TRANSIT_TO_ON);
    }
  }
}

// ------------------------------------
// ---- MINUS button, single click ----
// ------------------------------------

// Handle single click on the MINUS button:
// Decrease the brightness of the current mode one step
// ON: Decrease the brightness down to stepBrightness, then to OFF
// NIGHT_LIGHT: Decrease the night light brightness down to stepBrightness
void ctrlClickMinus(uint8_t btn)
{
  debugButtonAndState(MinusButton, single_click);
  handleMinus(btn);
}

// ----------------------------------
// ---- MINUS button, long click ----
// ----------------------------------

// Handle long click on the MINUS button:
// Decrease the brightness of the current mode as long as the button is pressed
// ON: Decrease the brightness down to stepBrightness, then to OFF
// NIGHT_LIGHT: Decrease the night light brightness down to stepBrightness
void ctrlLongClickMinus(uint8_t btn)
{
  debugButtonAndState(MinusButton, long_click);
  handleMinus(btn);
}

// --------------------------------
// ---- MINUS button, released ----
// --------------------------------

// Handle releasing the MINUS button:
// Reset _ignoreMinusLongClick
void ctrlReleasedMinus(uint8_t btn)
{
  if (_ignoreMinusLongClick)
  {
    if (_debugUartMain != nullptr)
    {
      _debugUartMain->println(F("MINUS button released"));
    }
    setIgnoreMinusLongClick(false);
  }
}

// ----------------------------------
// ---- OFF button, single click ----
// ----------------------------------

// Switch off LED strip when it is on, OR
// disallow night light when the night light is switched on OR
// (re-)enable night light when lamp is off
void ctrlClickOff(uint8_t btn)
{
  debugButtonAndState(OffButton, single_click);

  switch (_state)
  {
  case NIGHT_LIGHT_ON:
    setNightLightModeAllowed(false);
    break;

  case OFF:
  {
    bool changed = setNightLightModeAllowed(true);
    if (changed)
    {
      confirmViaLEDStrip(true, true);
    }
    break;
  }

  case ON:
    // light is to be turned off
    // check whether "off" means OFF of night light
    setState(enableNightLight() ? START_TRANSIT_TO_NIGHT_LIGHT : START_TRANSIT_TO_OFF);
    break;

  default:
    break;
  }
}

// ----------------------------------
// ---- OFF button, double click ----
// ----------------------------------

// Toggle allowing night light mode
void ctrlDblClickOff(uint8_t btn)
{
  debugButtonAndState(OffButton, double_click);
  setNightLightModeAllowed(!_allowNightLightMode); // toggling always changes, so no need to handle the return value
  confirmViaLEDStrip(true, _allowNightLightMode);
}

// ----------------------------------
// ---- OFF button, long click ----
// ----------------------------------

// Handle long click (2000ms+) on the OFF button:
// Save the current state of allow night light mode.
void ctrlLongClickOff(int8_t btn)
{
  debugButtonAndState(OffButton, long_click);
  if (_allowNightLightMode != allowNightLight())
  {
    setAllowNightLight(_allowNightLightMode);
  }
  // confirm whether or not the preference has actually changed
  confirmViaLEDStrip();
}

DeviceStateInfo getDeviceState()
{
  DeviceStateInfo info;
  info.allowNightLightMode = _allowNightLightMode;
  info.ldrValue = _ldrValue;
  info.maxBrightness = _maxBrightness;
  info.maxNightLightBrightness = _maxNightLightBrightness;
  info.movingTargetDetected = isMovingTargetDetected();
  info.movingTargetDistance = _prsInfo.movingTargetDistance;
  info.movingTargetEnergy = _prsInfo.movingTargetEnergy;
  info.nightLightBrightness = _nightLightBrightness;
  info.nightLightOnDuration = _nightLightOnDuration;
  info.nightLightThreshold = _nightLightThreshold;
  info.onBrightness = _onBrightness;
  info.state = _state;
  info.stationaryTargetDetected = isStationaryTargetDetected();
  info.stationaryTargetDistance = _prsInfo.stationaryTargetDistance;
  info.stationaryTargetEnergy = _prsInfo.stationaryTargetEnergy;
  info.stepBrightness = _stepBrightness;
  info.brightness = _targetBrightness;
  info.transitionDurationMs = _transitionDurationMs;

  info.presenceDetected = info.movingTargetDetected && info.stationaryTargetDetected;
  return info;
}

/*

  State handlers

 */

// When the lamp is off or turning off: check whether the night light should be switched on.
State nightLightCheckOff()
{
  return enableNightLight() ? State::START_TRANSIT_TO_NIGHT_LIGHT : State::OFF;
}

// When the night light is on: check whether the night light can be switched off again.
State nightLightCheckOn()
{
  // update the timestamp of the last presence detection
  if (_prsInfo.presenceDetected)
  {
    _nightLightEnabledTs = millis();
  }

  // calculate the duration with no presence detection since switching on the night light
  unsigned long noPresenceDuration = millis() - _nightLightEnabledTs;

  // switch off the night light when night light mode is not allowed any more or no presence has been detected for long enough, otherwise leave it on
  return (!_allowNightLightMode || noPresenceDuration > _nightLightOnDuration) ? START_TRANSIT_TO_OFF : NIGHT_LIGHT_ON;
}

// Trigger the brightness change to target brightness.
// Hint: use setTargetBrightness(...) to modify target brightness before calling this.
State startTransitToOn()
{
  setLEDStripBrightness();
  return TRANSIT_TO_ON;
}

// Checks whether a target brightness has been reached. Returns onBrightnessReached when reached, current state otherwise.
State checkTransitToBrightness(uint8_t target, State onBrightnessReached)
{
  uint8_t ledBrightness = ledStripCurrentBrightness();
  bool brightnessReached = (ledBrightness == target);
  return brightnessReached ? onBrightnessReached : _state;
}

// Stay in TRANSIT_TO_ON as long as the target brightness has not been reached, then switch to ON.
State transitToOn()
{
  State next = checkTransitToBrightness(ledStripTargetBrightness(), ON);
  if (next == ON)
    MONITOR_SERIAL.println(F("Lamp is ON"));
  return next;
}

// Trigger the brightness change to 0.
State startTransitToOff()
{
  setTargetBrightness(0);
  setLEDStripBrightness();
  return TRANSIT_TO_OFF;
}

// Stay in TRANSIT_TO_OFF as long as the target brightness (0) has not been reached, then switch to OFF.
State transitToOff()
{
  State next = checkTransitToBrightness(0, State::OFF);
  if (next == State::OFF)
    MONITOR_SERIAL.println(F("Lamp is OFF"));
  return next;
}

// Trigger the brightness change to night light brightness.
State startTransitToNightLight()
{
  setTargetBrightness(_nightLightBrightness);
  setLEDStripBrightness();
  return TRANSIT_TO_NIGHT_LIGHT;
}

// Stay in TRANSIT_TO_NIGHT_LIGHT as long as the target brightness has not been reached, then switch to NIGHT_LIGHT_ON.
State transitToNightLight()
{
  State next = (ledStripCurrentBrightness() == _nightLightBrightness) ? NIGHT_LIGHT_ON : _state;
  if (next == NIGHT_LIGHT_ON)
    MONITOR_SERIAL.println(F("Night Light is ON"));
  return next;
}

// Take the necessary actions for the current state.
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
    return; // nothing to do, wait for a pressed button
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
    // ToDo? this should never happen -> emit an error message?
    return;
    break;
  }
  setState(nextState);
}

void deviceSetup()
{
  MONITOR_SERIAL.begin(115200);
  _debugUartMain = &MONITOR_SERIAL;

  delay(500);
  MONITOR_SERIAL.println(F("ESP32 LED Night Light initializing ..."));

  configSetup();
  _allowNightLightMode = allowNightLight();
  _nightLightOnDuration = nightLightOnDuration() * 1000;
  _nightLightBrightness = nightLightBrightness();
  _nightLightThreshold = nightLightThreshold();
  _maxNightLightBrightness = maxNightLightBrightness();

  _transitionDurationMs = transitionDurationMs();

  _onBrightness = onBrightness();
  _maxBrightness = maxBrightness();
  _stepBrightness = stepBrightness();

  _maxMovingTargetDistance = maxMovingTargetDistance();
  _minMovingTargetDistance = minMovingTargetDistance();
  _maxMovingTargetEnergy = maxMovingTargetEnergy();
  _minMovingTargetEnergy = minMovingTargetEnergy();

  _maxStationaryTargetDistance = maxStationaryTargetDistance();
  _minStationaryTargetDistance = minStationaryTargetDistance();
  _maxStationaryTargetEnergy = maxStationaryTargetEnergy();
  _minStationaryTargetEnergy = minStationaryTargetEnergy();

  wifiDebug(MONITOR_SERIAL);
  wifiSetup();

  // touchDebug(MONITOR_SERIAL);
  touchSetup();

  ldrSetup();

  // presenceDebug(MONITOR_SERIAL);
  presenceSetup();

  // ledStripDebug(MONITOR_SERIAL);
  ledStripSetup();

  // --- ON button ---
  // normal click: switch LED strip on
  setClickHandler(OnButton, ctrlClickOn);
  // normal click: switch LED strip on AND set max brightness
  setDoubleClickHandler(OnButton, ctrlDblClickOn);
  // long click: save current brightness as default
  setLongClickHandler(OnButton, ctrlLongClickOn);

  // --- PLUS button ---
  // normal click: increase brightness one step
  setClickHandler(PlusButton, ctrlClickPlus);
  // long click: increase brightness one step as long as the button is pressed
  setLongClickHandler(PlusButton, ctrlLongClickPlus);
  // triple click: detect 'force AP' and 'factory reset' command
  setTripleClickHandler(PlusButton, ctrlTripleClickPlus);

  // --- MINUS button ---
  // normal click: decrease brightness one step
  setClickHandler(MinusButton, ctrlClickMinus);
  // long click: decrease brightness one step as long as the button is pressed
  setLongClickHandler(MinusButton, ctrlLongClickMinus);
  // release: correctly handle decreasing brightness
  setReleasedHandler(MinusButton, ctrlReleasedMinus);
  // triple click: detect 'force AP' and 'factory reset' command
  setTripleClickHandler(MinusButton, ctrlTripleClickMinus);

  // --- OFF button ---
  // normal click: switch LED strip off
  setClickHandler(OffButton, ctrlClickOff);
  // double click: Toggle allowing night light mode
  setDoubleClickHandler(OffButton, ctrlDblClickOff);
  // long click: save current choice for allowing night light mode
  setLongClickHandler(OffButton, ctrlLongClickOff);
  // triple click: detect 'force AP' and 'factory reset' command
  setTripleClickHandler(OffButton, ctrlTripleClickOff);

  webApiDebug(MONITOR_SERIAL);
  webApiSetup();

  webInterfaceDebug(MONITOR_SERIAL);
  webInterfaceSetup();

  MONITOR_SERIAL.println(F("ESP32 LED Night Light initialized, light is OFF"));
}

void deviceLoop()
{
  wifiLoop();

  // handle preferences
  configLoop();
  // read ldr
  ldrLoop();
  _ldrValue = averagedBrightness();
  // read radar presence sensor
  presenceLoop();
  _prsInfo = presenceInfo();
  // check touch buttons
  touchLoop();
  // check whether buttons or ldr or presence sensor require a state change
  handleState();
  // set LED strip accordingly
  ledStripLoop();

  webApiLoop();

  webInterfaceLoop();
}