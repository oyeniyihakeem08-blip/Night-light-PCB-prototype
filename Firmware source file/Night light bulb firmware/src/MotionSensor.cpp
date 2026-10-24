// ============================================================
// MotionSensor.cpp — Bulb
// ============================================================
#include "MotionSensor.h"

void MotionSensor::begin() {
    pinMode(PIN_MOTION, INPUT);
    _timerActive = false;
    _timerPaused = false;
    _timerStart  = 0;
}

bool MotionSensor::isMotionDetected() const {
    return digitalRead(PIN_MOTION) == HIGH;
}

bool MotionSensor::updateTimer() {
    if (!_timerActive || _timerPaused) return false;

    if (isMotionDetected()) {
        // Motion seen — reset countdown
        _timerStart = millis();
        return false;
    }

    // No motion — check if 60s has elapsed since last motion
    return (millis() - _timerStart >= MOTION_TIMEOUT_MS);
}

void MotionSensor::resetTimer() {
    _timerStart  = millis();
    _timerActive = true;
    _timerPaused = false;
}

void MotionSensor::pauseTimer() {
    _timerPaused = true;
}

void MotionSensor::resumeTimer() {
    // Resume from now — treat current moment as last motion
    _timerStart  = millis();
    _timerPaused = false;
}

void MotionSensor::disableTimer() {
    _timerActive = false;
    _timerPaused = false;
}
