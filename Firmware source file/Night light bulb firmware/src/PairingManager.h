// ============================================================
// PairingManager.h — Bulb (STM32G031K8T6)
//
// Owns the system-level state machine:
//   SYS_UNPAIRED  — no pairing data; only PAIR or FACTORY_RESET accepted
//   SYS_PAIRING   — 60s window waiting for PAIR on pipe 0
//   SYS_NORMAL    — paired and operational; hands commands to BulbController
//
// Pipe 0 (public) is always monitored here, even in SYS_NORMAL,
// so FACTORY_RESET can arrive at any time without a token.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Storage.h"
#include "Radio.h"
#include "BulbController.h"
#include "Protocol.h"

enum SystemState : uint8_t {
    SYS_UNPAIRED,
    SYS_PAIRING,
    SYS_NORMAL
};

class PairingManager {
public:
    void begin(Storage* stor, Radio* radio, BulbController* ctrl);

    // Call every main loop tick
    void update();

private:
    void enterPairingMode();
    void handlePairingWindow();
    void handleNormalMode();
    void handlePublicPacket(const Packet& pkt);
    void processFactoryReset();
    uint32_t generateToken();
    void updatePairingLED();   // Non-blocking 1s/2s blink pattern

    Storage*        _stor;
    Radio*          _radio;
    BulbController* _ctrl;

    SystemState _state;
    uint32_t    _pairingWindowStart;

    // Non-blocking pairing LED state
    bool     _ledOn;
    uint32_t _ledPhaseStart;
};
