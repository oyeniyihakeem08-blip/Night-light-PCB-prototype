// ============================================================
// BulbController.h — Bulb (STM32G031K8T6)
//
// Owns the bulb's operational state machine.
// Called only after pairing is confirmed (SYS_NORMAL).
//
// States:
//   MODE_AUTO   → sub-states: AUTO_OFF, AUTO_ON, AUTO_FORCED_ON
//   MODE_MANUAL → bulb ON or OFF, no sensor logic
// ============================================================
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Storage.h"
#include "Radio.h"
#include "MotionSensor.h"
#include "LightSensor.h"
#include "Protocol.h"

// Sensor mode — matches EEPROM values
enum SensorMode : uint8_t {
    MODE_AUTO   = 0,
    MODE_MANUAL = 1
};

// Internal sub-states for AUTO mode
enum AutoState : uint8_t {
    AUTO_OFF,        // Bulb off, monitoring sensors
    AUTO_ON,         // Bulb on, motion timer running
    AUTO_FORCED_ON   // Locked on by remote, sensors paused
};

class BulbController {
public:
    // Call after pairing data is available
    void begin(Storage* stor, Radio* radio,
               MotionSensor* motion, LightSensor* light);

    // Load mode from storage and activate private radio pipe
    void activate(const PairingData& data);

    // Call every main loop tick (5ms)
    void update();

    // Called by PairingManager when a validated packet arrives on pipe 1
    void handleCommand(const Packet& pkt);

    // Current mode (used by PairingManager for EEPROM saves)
    SensorMode currentMode() const { return _mode; }

private:
    void  setBulb(bool on);
    void  switchToAuto();
    void  switchToManual();
    void  flashLED(uint8_t count);     // n × SHORT_FLASH_MS blinks
    bool  tokenValid(const Packet& p);

    Storage*      _stor;
    Radio*        _radio;
    MotionSensor* _motion;
    LightSensor*  _light;

    SensorMode  _mode;
    AutoState   _autoState;
    bool        _bulbOn;
    uint32_t    _pairingToken;

    // Pair-success LED sequence state
    bool     _playingSuccessSeq;
    uint8_t  _successPhase;
    uint32_t _successPhaseStart;
};
