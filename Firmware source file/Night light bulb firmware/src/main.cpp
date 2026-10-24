// ============================================================
// SmartBulb.ino — Bulb (STM32G031K8T6)
//
// Entry point only. All logic lives in the modules below.
// setup() wires modules together. loop() just ticks them.
//
// Required libraries (install via Arduino Library Manager):
//   • RF24 by TMRh20
//   • STM32duino Low Power  (part of STM32duino board package)
//   • STM32duino IWatchdog  (part of STM32duino board package)
//
// Board: STM32G031K8Tx (Generic STM32G0 series)
// Board package: STM32 by STMicroelectronics (STM32duino)
// ============================================================
#include <Arduino.h>
#include "Config.h"
#include "Storage.h"
#include "Watchdog.h"
#include "Radio.h"
#include "MotionSensor.h"
#include "LightSensor.h"
#include "BulbController.h"
#include "PairingManager.h"

// ── Module instances ─────────────────────────────────────────
static Storage       storage;
static Watchdog      watchdog;
static Radio         radio;
static MotionSensor  motionSensor;
static LightSensor   lightSensor;
static BulbController bulbCtrl;
static PairingManager pairingMgr;

// ── Setup ────────────────────────────────────────────────────
void setup() {
    storage.begin();
    watchdog.begin();
    radio.begin();
    motionSensor.begin();
    lightSensor.begin();

    // Wire dependencies together before activating state machines
    bulbCtrl.begin(&storage, &radio, &motionSensor, &lightSensor);
    pairingMgr.begin(&storage, &radio, &bulbCtrl);
    // PairingManager.begin() decides whether to pair or go normal
}

// ── Main loop ────────────────────────────────────────────────
void loop() {
    static uint32_t _lastTick = 0;
    uint32_t now = millis();

    // Sleep until next 5ms tick (STOP mode — wakes on IRQ or SysTick)
    if (now - _lastTick < MAIN_LOOP_INTERVAL_MS) {
        watchdog.sleepUntilNextTick(MAIN_LOOP_INTERVAL_MS - (now - _lastTick));
        return;
    }
    _lastTick = now;

    watchdog.refresh();    // Pet IWDG every 4s
    pairingMgr.update();   // Handle radio + system state
    bulbCtrl.update();     // Run sensor logic
}
