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
    CONFIRM_INPUT_DONE,
    TEST_START,
    TEST_INCREASE_BRIGHTNESS,
    TEST_DECREASE_BRIGHTNESS,
    TEST_DONE
};

Stream *_debug_uart = nullptr;

LEDStripState _currentState = TARGET_BRIGHTNESS_REACHED; // the state the LED strip hardware is actually in
LEDStripState _savedState = TARGET_BRIGHTNESS_REACHED;   // for storing the state when confirming or testing
LEDStripState _desiredState = TARGET_BRIGHTNESS_REACHED; // the state the LED strip hardware is to move to
LEDStripState _nextState = TARGET_BRIGHTNESS_REACHED;

uint8_t _transitionStartBrightness = 0;
uint8_t _currentBrightness = 0; // the actual LED strip hardware brightness
uint8_t _desiredBrightness = 0; // the desired LED strip hardware brightness
int16_t _brightnessGap = 0;     // brightness gap to bridge between brightness on transition start and desired brightness (might be negative)

LEDStripState currentState() { return _currentState; }
uint8_t currentBrightness() { return _currentBrightness; }
LEDStripState desiredState() { return _desiredState; }
LEDStripState nextState() { return _nextState; }

unsigned long _loopTimeStamp = 0;
unsigned long _transitionStartTs = 0;   // the moment the current transition started (0 when there is no current transition)
unsigned long _transitionDuration = 0;  // how much time the current transition has taken (0 when there is no current transition)
unsigned long _lastConfirmTs = 0;       // for delaying the led on/off cycles

void setDebugStream(Stream &terminalStream)
{
    _debug_uart = &terminalStream;
}

void ledStripSetup()
{
    pinMode(LED_STRIP_PIN, OUTPUT);
    analogWrite(LED_STRIP_PIN, _desiredBrightness);
}

void startTransitionToBrightness()
{
    if (_debug_uart != nullptr)
    {
        _debug_uart->print(F("startTransitionToBrightness from "));
        _debug_uart->print((int16_t)_currentBrightness);
        _debug_uart->print(F(" to "));
        _debug_uart->println((int16_t)_desiredBrightness);
    }
    _transitionStartBrightness = _currentBrightness;
    _brightnessGap = (int16_t)_desiredBrightness - (int16_t)_transitionStartBrightness;
    _transitionStartTs = _loopTimeStamp;
    _currentState = LEDStripState::TRANSITION_TO_BRIGHTNESS;
}

void transitionToBrightness()
{
    // check time left to reach desired brightness (in ms)
    _transitionDuration = _loopTimeStamp - _transitionStartTs;
    if (_transitionDuration > transitionDurationMs())
    {
        // time's up
        if (_debug_uart != nullptr)
        {
            _debug_uart->println(F("transitionToBrightness: transition duration's up"));
        }
        _transitionDuration = 0;
        _currentState = LEDStripState::TRANSITION_TO_BRIGHTNESS_DONE;
    }
    else
    {
        // time left, make transition smooth
        uint32_t newBrightness = (uint32_t)_transitionStartBrightness + (uint32_t)_brightnessGap * (uint32_t)_transitionDuration / (uint32_t)transitionDurationMs();
        if (newBrightness != _currentBrightness)
        {
            if (_debug_uart != nullptr)
            {
                _debug_uart->print(F("transitionToBrightness from "));
                _debug_uart->print((int16_t)_currentBrightness);
                _debug_uart->print(F(" to "));
                _debug_uart->println(newBrightness);
            }
            _currentBrightness = newBrightness & 255;
            analogWrite(LED_STRIP_PIN, _currentBrightness);
        }
    }
}

void transitionToBrightnessDone()
{
    if (_debug_uart != nullptr)
    {
        _debug_uart->println(F("transitionToBrightnessDone"));
    }
    analogWrite(LED_STRIP_PIN, _desiredBrightness);
    _currentState = LEDStripState::TARGET_BRIGHTNESS_REACHED;
}

void doTest()
{

}

void doConfirm()
{
    unsigned long now = millis();
    if (now - _lastConfirmTs < 200)
        return;

    switch (_currentState)
    {
    case LEDStripState::CONFIRM_INPUT_DONE:
    {
        _currentState = _savedState;
    }
    case LEDStripState::CONFIRM_INPUT_OFF_1:
    {
        analogWrite(LED_STRIP_PIN, 0);
        _lastConfirmTs = now;
        _currentState = CONFIRM_INPUT_ON_1;
    }
    case LEDStripState::CONFIRM_INPUT_OFF_2:
    {
        analogWrite(LED_STRIP_PIN, 0);
        _lastConfirmTs = now;
        _currentState = CONFIRM_INPUT_ON_2;
    }
    case LEDStripState::CONFIRM_INPUT_ON_1:
    {
        analogWrite(LED_STRIP_PIN, 255);
        _lastConfirmTs = now;
        _currentState = CONFIRM_INPUT_OFF_2;
    }
    case LEDStripState::CONFIRM_INPUT_ON_2:
    {
        analogWrite(LED_STRIP_PIN, 255);
        _lastConfirmTs = now;
        _currentState = CONFIRM_INPUT_DONE;
    }
    case LEDStripState::CONFIRM_INPUT_START:
    {
        _currentState = LEDStripState::CONFIRM_INPUT_OFF_1;
        _lastConfirmTs = 0;
    }
    default:
    {
        _currentState = LEDStripState::CONFIRM_INPUT_DONE;
        _lastConfirmTs = 0;
    }
    }
}

void ledStripLoop()
{
    if (_currentState == _desiredState)
        return;

    _loopTimeStamp = millis();

    switch (_currentState)
    {
    case LEDStripState::START_TRANSITION_TO_BRIGHTNESS:
    {
        startTransitionToBrightness();
        break;
    }
    case LEDStripState::TRANSITION_TO_BRIGHTNESS:
    {
        transitionToBrightness();
        break;
    }
    case LEDStripState::TRANSITION_TO_BRIGHTNESS_DONE:
    {
        transitionToBrightnessDone();
        break;
    }
    case LEDStripState::TEST_DECREASE_BRIGHTNESS:
    case LEDStripState::TEST_DONE:
    case LEDStripState::TEST_INCREASE_BRIGHTNESS:
    case LEDStripState::TEST_START:
    {
        doTest();
    }
    case LEDStripState::CONFIRM_INPUT_DONE:
    case LEDStripState::CONFIRM_INPUT_OFF_1:
    case LEDStripState::CONFIRM_INPUT_OFF_2:
    case LEDStripState::CONFIRM_INPUT_ON_1:
    case LEDStripState::CONFIRM_INPUT_ON_2:
    case LEDStripState::CONFIRM_INPUT_START:
    {
        doConfirm();
    }
    }
}

void ledStripSetTargetBrightness(uint8_t brightness)
{
    if (_desiredBrightness != brightness)
    {
        _desiredBrightness = brightness;
        _currentState = START_TRANSITION_TO_BRIGHTNESS;
    }
}

void ledStripConfirm()
{
    if (_debug_uart != nullptr)
    {
        _debug_uart->print(F("ledStripConfirm: "));
    }

    switch (_currentState)
    {
    case LEDStripState::START_TRANSITION_TO_BRIGHTNESS:
    case LEDStripState::TRANSITION_TO_BRIGHTNESS:
    case LEDStripState::TRANSITION_TO_BRIGHTNESS_DONE:
    {
        unsigned long tsDiffMs = _loopTimeStamp - _transitionStartTs;
        if (_debug_uart != nullptr)
        {
            _debug_uart->print(F("interrupting transition, remaining time (ms): "));
            _debug_uart->println(tsDiffMs);
        }
        _savedState = _currentState;
        _desiredState = LEDStripState::CONFIRM_INPUT_START;
        break;
    }
    case LEDStripState::TARGET_BRIGHTNESS_REACHED:
    {
        if (_debug_uart != nullptr)
        {
            _debug_uart->println(F("confirm while target brightness reached"));
        }
        _savedState = _currentState;
        _desiredState = LEDStripState::CONFIRM_INPUT_START;
        break;
    }    
    case LEDStripState::TEST_DECREASE_BRIGHTNESS:
    case LEDStripState::TEST_DONE:
    case LEDStripState::TEST_INCREASE_BRIGHTNESS:
    case LEDStripState::TEST_START:
    {
        if (_debug_uart != nullptr)
        {
            _debug_uart->println(F("interrupting test"));
        }
        _savedState = _currentState;
        _desiredState = LEDStripState::CONFIRM_INPUT_START;
        break;
    }
    default:
    {
        if (_debug_uart != nullptr)
        {
            _debug_uart->println(F("already confirming (request ignored)"));
        }
        return;
    }
    }
}

void ledStripTest()
{
}