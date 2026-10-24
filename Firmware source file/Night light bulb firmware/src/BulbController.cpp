// ============================================================
// BulbController.cpp — Bulb
// ============================================================
#include "BulbController.h"

void BulbController::begin(Storage* stor, Radio* radio,
                            MotionSensor* motion, LightSensor* light) {
    _stor   = stor;
    _radio  = radio;
    _motion = motion;
    _light  = light;

    _bulbOn           = false;
    _playingSuccessSeq = false;
    _successPhase     = 0;
    _mode             = MODE_AUTO;
    _autoState        = AUTO_OFF;
    _pairingToken     = 0;

    pinMode(PIN_BULB_LED, OUTPUT);
    setBulb(false);
}

void BulbController::activate(const PairingData& data) {
    _pairingToken = data.token;
    _radio->openPrivatePipe(data.remote_addr);

    // Restore persisted mode
    if (data.sensor_mode == MODE_MANUAL) {
        switchToManual();
    } else {
        switchToAuto();
    }

    // Play pair-success LED sequence: 3s ON → 2s OFF → 3s ON → dark
    _playingSuccessSeq = true;
    _successPhase      = 0;
    _successPhaseStart = millis();
    setBulb(true);  // Phase 0: 3s on
}

void BulbController::update() {
    // ── Pair-success LED sequence (non-blocking) ─────────────
    if (_playingSuccessSeq) {
        uint32_t elapsed = millis() - _successPhaseStart;
        switch (_successPhase) {
            case 0:  // 3s ON
                if (elapsed >= SUCCESS_BLINK_ON_MS) {
                    setBulb(false);
                    _successPhase     = 1;
                    _successPhaseStart = millis();
                }
                return;  // Block normal operation during sequence
            case 1:  // 2s OFF
                if (elapsed >= SUCCESS_BLINK_OFF_MS) {
                    setBulb(true);
                    _successPhase     = 2;
                    _successPhaseStart = millis();
                }
                return;
            case 2:  // 3s ON
                if (elapsed >= SUCCESS_BLINK_ON_MS) {
                    setBulb(false);
                    _playingSuccessSeq = false;
                    // Resume normal AUTO operation from OFF state
                    _autoState = AUTO_OFF;
                    _light->startSettle();
                }
                return;
        }
    }

    // ── Normal operation ─────────────────────────────────────
    if (_mode == MODE_AUTO) {
        switch (_autoState) {
            case AUTO_OFF:
                // LDR gated to OFF-state only (spec 4A)
                if (_light->isDark() && _motion->isMotionDetected()) {
                    setBulb(true);
                    _motion->resetTimer();
                    _autoState = AUTO_ON;
                }
                break;

            case AUTO_ON:
                // Check motion timeout
                if (_motion->updateTimer()) {
                    setBulb(false);
                    _light->startSettle();
                    _autoState = AUTO_OFF;
                }
                break;

            case AUTO_FORCED_ON:
                // Timer paused — nothing to do until remote presses POWER
                break;
        }
    }
    // MANUAL mode: no sensor logic — bulb stays where remote left it
}

void BulbController::handleCommand(const Packet& pkt) {
    // All pipe-1 commands require a valid token
    if (!tokenValid(pkt)) return;

    switch (pkt.cmd) {
        case CMD_POWER_ON:
            if (_mode == MODE_AUTO) {
                // Forced-on: lock bulb on, pause motion timer
                setBulb(true);
                _motion->pauseTimer();
                _autoState = AUTO_FORCED_ON;
            } else {
                // MANUAL mode: simple on
                setBulb(true);
            }
            break;

        case CMD_POWER_OFF:
            if (_mode == MODE_AUTO) {
                if (_autoState == AUTO_FORCED_ON) {
                    // Resume AUTO from OFF
                    setBulb(false);
                    _light->startSettle();
                    _motion->resumeTimer();
                    _autoState = AUTO_OFF;
                } else {
                    // Normal off from AUTO_ON
                    setBulb(false);
                    _light->startSettle();
                    _motion->disableTimer();
                    _autoState = AUTO_OFF;
                    _motion->resetTimer();  // Arm timer for next AUTO_ON
                }
            } else {
                setBulb(false);
            }
            break;

        case CMD_SENSOR_ON:
            switchToAuto();
            _stor->saveSensorMode(MODE_AUTO);
            flashLED(1);  // 1 flash = AUTO mode confirmed
            break;

        case CMD_SENSOR_OFF:
            switchToManual();
            _stor->saveSensorMode(MODE_MANUAL);
            flashLED(3);  // 3 flashes = MANUAL mode confirmed
            break;

        default:
            break;
    }
}

// ── Private helpers ──────────────────────────────────────────

void BulbController::setBulb(bool on) {
    _bulbOn = on;
    digitalWrite(PIN_BULB_LED, on ? HIGH : LOW);
}

void BulbController::switchToAuto() {
    _mode      = MODE_AUTO;
    _autoState = AUTO_OFF;
    _motion->disableTimer();
    _light->startSettle();  // Safe settle before first LDR read
}

void BulbController::switchToManual() {
    _mode = MODE_MANUAL;
    _motion->disableTimer();
    // Bulb state unchanged — stays wherever it is
}

void BulbController::flashLED(uint8_t count) {
    // Blocking flash — only called on mode change, not in normal loop
    bool wasOn = _bulbOn;
    for (uint8_t i = 0; i < count; i++) {
        digitalWrite(PIN_BULB_LED, HIGH);
        delay(SHORT_FLASH_MS);
        digitalWrite(PIN_BULB_LED, LOW);
        delay(SHORT_FLASH_MS);
    }
    // Restore bulb state after flash
    digitalWrite(PIN_BULB_LED, wasOn ? HIGH : LOW);
}

bool BulbController::tokenValid(const Packet& pkt) {
    return (pkt.token == _pairingToken);
}
