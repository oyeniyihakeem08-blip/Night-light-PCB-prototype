// ============================================================
// LightSensor.cpp — Bulb
// ============================================================
#include "LightSensor.h"

void LightSensor::begin() {
    pinMode(PIN_LDR, INPUT_ANALOG);
    _settling    = false;
    _settleStart = 0;
}

void LightSensor::startSettle() {
    _settling    = true;
    _settleStart = millis();
}

bool LightSensor::isDark() {
    // Still in settle window — do not trust the ADC reading yet
    if (_settling) {
        if (millis() - _settleStart < LDR_SETTLE_MS) {
            return false;  // Report not-dark during settle (conservative)
        }
        _settling = false;  // Settle complete — now safe to sample
    }

    int reading = analogRead(PIN_LDR);
    // ADC < threshold means low voltage → dark room (pull-down divider)
    return (reading < LDR_DARK_THRESHOLD);
}
