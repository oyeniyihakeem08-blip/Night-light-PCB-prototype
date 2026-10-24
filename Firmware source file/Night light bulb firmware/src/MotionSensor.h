// ============================================================
// MotionSensor.h / .cpp — Bulb (STM32G031K8T6)
// Reads BISS0001 digital output. Owns the 60s motion timer.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class MotionSensor {
public:
    void begin();

    // Returns true if BISS0001 is currently asserting motion
    bool isMotionDetected() const;

    // Called every loop tick when bulb is ON and in AUTO mode.
    // Resets the 60s countdown when motion is seen.
    // Returns true when timeout has expired (time to turn OFF).
    bool updateTimer();

    // Restart the 60s countdown from now
    void resetTimer();

    // Pause/resume the timer (used during FORCED_ON state)
    void pauseTimer();
    void resumeTimer();

    // Disable timer entirely (MANUAL mode)
    void disableTimer();

private:
    bool     _timerActive;
    bool     _timerPaused;
    uint32_t _timerStart;   // millis() snapshot when motion last seen
};
