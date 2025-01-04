#include "config.h"
#include "led_strip.h"

enum LEDStripState
{
    TARGET_BRIGHTNESS_REACHED,
    START_TRANSITION_TO_BRIGHTNESS,
    TRANSITION_TO_BRIGHTNESS,
    TRANSITION_TO_BRIGHTNESS_DONE,
    CONFIRM_INPUT_START,
    CONFIRM_INPUT_OFF_1,
    CONFIRM_INPUT_ON_1,
    CONFIRM_INPUT_OFF_2,
    CONFIRM_INPUT_ON_2,
    CONFIRM_INPUT_DONE
};

Stream *debug_uart_led_strip = nullptr;

LEDStripState _ledCurrentState = TARGET_BRIGHTNESS_REACHED; // the state the LED strip hardware is actually in
LEDStripState _ledSavedState = TARGET_BRIGHTNESS_REACHED;   // for storing the state when confirming or testing
uint8_t _ledSavedBrightness = 0;


uint8_t _ledTransitionStartBrightness = 0; // the brightness the LED strip hardware was set to at the start of the transition
uint8_t _ledCurrentBrightness = 0;         // the actual LED strip hardware brightness
uint8_t _ledTargetBrightness = 0;          // the target LED strip hardware brightness to be reached in the transition
int16_t _ledBrightnessGap = 0;             // brightness gap to bridge between brightness on transition start and desired brightness (might be negative)

uint8_t ledStripCurrentBrightness() { return _ledCurrentBrightness; }
uint8_t ledStripTargetBrightness() { return _ledTargetBrightness; }

unsigned long _loopTimeStamp = 0;
unsigned long _transitionStartTs = 0;  // the moment the current transition started (0 when there is no current transition)
unsigned long _transitionDuration = 0; // how much time the current transition has taken (0 when there is no current transition)
unsigned long _lastConfirmTs = 0;      // for delaying the led on/off cycles

void setBrightness(uint8_t value);
void startTransitionToBrightness();
void transitionToBrightness();
void transitionToBrightnessDone();
void doConfirm();

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
    case CONFIRM_INPUT_DONE:
    case CONFIRM_INPUT_OFF_1:
    case CONFIRM_INPUT_OFF_2:
    case CONFIRM_INPUT_ON_1:
    case CONFIRM_INPUT_ON_2:
    case CONFIRM_INPUT_START:
    {
        doConfirm();
    }
    }
}

void ledStripConfirm()
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
            debug_uart_led_strip->print(F("interrupting transition, remaining time (ms): "));
            debug_uart_led_strip->println(tsDiffMs);
        }
        _ledSavedState = _ledCurrentState;
        _ledCurrentState = CONFIRM_INPUT_START;
        break;
    }
    case TARGET_BRIGHTNESS_REACHED:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("confirm while target brightness reached"));
        }
        _ledSavedState = _ledCurrentState;
        _ledCurrentState = CONFIRM_INPUT_START;
        break;
    }
    default:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("already confirming (request ignored)"));
        }
        return;
    }
    }
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
        case CONFIRM_INPUT_START:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_START"));
            break;
        case CONFIRM_INPUT_OFF_1:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_OFF_1"));
            break;
        case CONFIRM_INPUT_ON_1:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_ON_1"));
            break;
        case CONFIRM_INPUT_OFF_2:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_OFF_2"));
            break;
        case CONFIRM_INPUT_ON_2:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_ON_2"));
            break;
        case CONFIRM_INPUT_DONE:
            debug_uart_led_strip->print(F("CONFIRM_INPUT_DONE"));
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
        debug_uart_led_strip->print(F("LED brightness: "));
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
            debug_uart_led_strip->print(F("transitionToBrightness: duration "));
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
                debug_uart_led_strip->print(F("transitionToBrightness: duration "));
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
    if (_loopTimeStamp - _lastConfirmTs < 200)
        return;

    LEDStripState nextState;

    switch (_ledCurrentState)
    {
    case CONFIRM_INPUT_DONE:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_DONE"));
            debug_uart_led_strip->print(F("  restore state: "));
            debugPrintStateText(_ledSavedState, true);
        }
        nextState = _ledSavedState;
        setBrightness(_ledSavedBrightness);
        break;
    }
    case CONFIRM_INPUT_OFF_1:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_OFF_1"));
            debug_uart_led_strip->println(F("  set brightness to 0"));
        }
        setBrightness(0);
        _lastConfirmTs = _loopTimeStamp;
        nextState = CONFIRM_INPUT_ON_1;
        break;
    }
    case CONFIRM_INPUT_OFF_2:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_OFF_2"));
            debug_uart_led_strip->println(F("  set brightness to 0"));
        }
        setBrightness(0);
        _lastConfirmTs = _loopTimeStamp;
        nextState = CONFIRM_INPUT_ON_2;
        break;
    }
    case CONFIRM_INPUT_ON_1:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_ON_1"));
            debug_uart_led_strip->println(F("  set brightness to 255"));
        }
        setBrightness(255);
        _lastConfirmTs = _loopTimeStamp;
        nextState = CONFIRM_INPUT_OFF_2;
        break;
    }
    case CONFIRM_INPUT_ON_2:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_ON_2"));
            debug_uart_led_strip->println(F("  set brightness to 255"));
        }
        setBrightness(255);
        _lastConfirmTs = _loopTimeStamp;
        nextState = CONFIRM_INPUT_DONE;
        break;
    }
    case CONFIRM_INPUT_START:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("CONFIRM_INPUT_START"));
        }
        _ledSavedBrightness = _ledCurrentBrightness;
        _lastConfirmTs = 0;
        nextState = CONFIRM_INPUT_OFF_1;
        break;
    }
    default:
    {
        if (debug_uart_led_strip != nullptr)
        {
            debug_uart_led_strip->println(F("DEFAULT - you're not supposed to see this!"));
            debug_uart_led_strip->print(F("  state: "));
            debugPrintStateText(_ledCurrentState, true);
        }
        _lastConfirmTs = 0;
        nextState = CONFIRM_INPUT_DONE;
    }
    }

    if (debug_uart_led_strip != nullptr)
    {
        debug_uart_led_strip->print(F("switching state from "));
        debugPrintStateText(_ledCurrentState);
        debug_uart_led_strip->print(F(" to "));
        debugPrintStateText(nextState, true);
    }
    _ledCurrentState = nextState;
}
