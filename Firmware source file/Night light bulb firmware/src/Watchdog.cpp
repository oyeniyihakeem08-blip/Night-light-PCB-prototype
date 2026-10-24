// ============================================================
// Watchdog.cpp — Bulb
// ============================================================
#include "Watchdog.h"
#include "Config.h"
#include <IWatchdog.h>
#include <STM32LowPower.h>

void Watchdog::begin() {
    // 8,000,000 µs = 8 second IWDG timeout
    IWatchdog.begin(8000000);
    LowPower.begin();
}

void Watchdog::refresh() {
    static uint32_t _lastRefresh = 0;
    uint32_t now = millis();
    if (now - _lastRefresh >= WATCHDOG_REFRESH_MS) {
        IWatchdog.reload();
        _lastRefresh = now;
    }
}

void Watchdog::sleepUntilNextTick(uint32_t ms) {
    // STOP mode: CPU and most peripherals off.
    // Wakes on: SysTick expiry, nRF24 IRQ falling edge, motion sensor edge.
    // STM32LowPower handles re-enabling SysTick on wake automatically.
    LowPower.sleep(ms);
}
