// ============================================================
// ButtonHandler.cpp — Remote
// ============================================================
#include "ButtonHandler.h"

void ButtonHandler::begin() {
    pinMode(PIN_BTN_POWER,     INPUT_PULLUP);
    pinMode(PIN_BTN_SENSOR_ON, INPUT_PULLUP);
    pinMode(PIN_BTN_SENSOR_OFF,INPUT_PULLUP);
}

ButtonEvent ButtonHandler::detect() {
    // Debounce — ignore sub-50ms noise on wake
    delay(DEBOUNCE_MS);

    if (isPressed(PIN_BTN_SENSOR_ON)) {
        waitForRelease(PIN_BTN_SENSOR_ON);
        return BTN_SENSOR_ON;
    }

    if (isPressed(PIN_BTN_SENSOR_OFF)) {
        waitForRelease(PIN_BTN_SENSOR_OFF);
        return BTN_SENSOR_OFF;
    }

    if (isPressed(PIN_BTN_POWER)) {
        uint32_t pressStart = millis();
        // Block here while button is held — timing the hold
        while (isPressed(PIN_BTN_POWER)) {
            delay(10);
            // Nothing else to do — remote is awake, holding is intentional
        }
        uint32_t held = millis() - pressStart;

        if (held >= HOLD_RESET_MS) return BTN_POWER_RESET;
        if (held >= HOLD_PAIR_MS)  return BTN_POWER_PAIR;
        return BTN_POWER_TAP;
    }

    return BTN_NONE;
}

bool ButtonHandler::isPressed(uint8_t pin) const {
    return digitalRead(pin) == LOW;  // Active low (pull-up)
}

void ButtonHandler::waitForRelease(uint8_t pin) const {
    while (digitalRead(pin) == LOW) {
        delay(10);
    }
}
