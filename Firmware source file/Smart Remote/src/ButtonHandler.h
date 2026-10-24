// ============================================================
// ButtonHandler.h / .cpp — Remote (ATtiny1616-s)
//
// Identifies which button woke the MCU and, for the POWER button,
// measures how long it was held before release.
// All timing is synchronous — we are awake for the whole hold.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

enum ButtonEvent : uint8_t {
    BTN_NONE,
    BTN_POWER_TAP,          // POWER pressed and released < 3s
    BTN_POWER_PAIR,         // POWER held 3–6s
    BTN_POWER_RESET,        // POWER held > 6s
    BTN_SENSOR_ON,
    BTN_SENSOR_OFF,
};

class ButtonHandler {
public:
    void begin();

    // Call once after waking from sleep.
    // Blocks until button is released (for POWER hold detection).
    // Returns the event that occurred.
    ButtonEvent detect();

private:
    bool isPressed(uint8_t pin) const;
    void waitForRelease(uint8_t pin) const;
};
