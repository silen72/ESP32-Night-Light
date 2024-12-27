#include "ldr.h"

uint16_t _averageBrightness = 4096;
uint32_t _measurementDelayMs = 500;

unsigned long _lastMeasureTs = 0;

uint8_t const RINGBUFFER_SIZE = 2 << 4; // must be 2, 4, 8, 16, 32 or 64
uint8_t const RINGPOS_MASK = RINGBUFFER_SIZE - 1;
uint8_t _ringbuffer[RINGBUFFER_SIZE] = {0};
uint8_t _ringfill = 0;
uint8_t _ringpos_current = 0;
uint16_t _ring_sum = 0;
uint8_t _ring_avg = 0;

void ring_add(uint8_t value)
{
    // calculate average
    _ring_sum = _ring_sum - _ringbuffer[_ringpos_current] + value;
    if (_ringfill < RINGBUFFER_SIZE) _ringfill++;
    _ring_avg = _ring_sum / _ringfill;

    // set value in ring
    _ringbuffer[_ringpos_current] = value;

    // increment ring pointer, adjust overflows
    _ringpos_current++;
    _ringpos_current &= RINGPOS_MASK;
}

uint8_t ring_avg() { return _ring_avg; }

void ldrSetup()
{
    pinMode(LDR_PIN, INPUT);
    _averageBrightness = measuredBrightness();
}

void ldrLoop()
{
    unsigned long timeStamp = millis();
    if (timeStamp - _lastMeasureTs >= _measurementDelayMs)
    {
        _averageBrightness = (_averageBrightness + measuredBrightness()) / 2;
        _lastMeasureTs = timeStamp;
    }
}

uint16_t averagedBrightness() { return _averageBrightness; }
uint16_t measuredBrightness() { return analogRead(LDR_PIN); }

uint32_t measurementDelayMs() { return _measurementDelayMs; }
void setMeasurementDelayMs(uint32_t value) { _measurementDelayMs = value; }

unsigned long lastMeasureTs() { return _lastMeasureTs; }
