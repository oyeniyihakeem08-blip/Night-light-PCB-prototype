// ============================================================
// Watchdog.h — Bulb (STM32G031K8T6)
// IWDG + STOP mode sleep in one place.
// ============================================================
#pragma once
#include <Arduino.h>

class Watchdog {
public:
    // Start IWDG (8s timeout) and initialise low-power library
    void begin();

    // Call every WATCHDOG_REFRESH_MS to prevent reset.
    // Safe to call every loop iteration — throttled internally.
    void refresh();

    // Enter STOP mode for up to 'ms' milliseconds.
    // Wakes early on: SysTick, nRF24 IRQ pin, or motion sensor edge.
    void sleepUntilNextTick(uint32_t ms);
};
