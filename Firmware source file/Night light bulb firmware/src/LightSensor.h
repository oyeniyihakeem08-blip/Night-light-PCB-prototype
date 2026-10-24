// ============================================================
// LightSensor.h / .cpp — Bulb (STM32G031K8T6)
// LDR sampling with OFF-state gating and 500ms settle delay.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class LightSensor {
public:
    void begin();

    // Call when bulb turns OFF to start the 500ms settle window.
    // LDR will not be read until settle completes.
    void startSettle();

    // Returns true if room is currently dark AND the settle window
    // has completed. Returns false while settling.
    // NEVER call this when the bulb is ON — see spec section 4A.
    bool isDark();

private:
    bool     _settling;
    uint32_t _settleStart;
};
