// ============================================================
// PairingManager.cpp — Bulb
// ============================================================
#include "PairingManager.h"
#include "Config.h"

void PairingManager::begin(Storage* stor, Radio* radio, BulbController* ctrl) {
    _stor  = stor;
    _radio = radio;
    _ctrl  = ctrl;

    _ledOn         = false;
    _ledPhaseStart = 0;

    if (_stor->isPaired()) {
        // Already paired — go straight to normal operation
        PairingData data;
        _stor->load(data);
        _ctrl->activate(data);
        _state = SYS_NORMAL;
    } else {
        // First boot (or post-factory-reset) — start pairing window
        enterPairingMode();
    }
}

void PairingManager::update() {
    switch (_state) {
        case SYS_UNPAIRED:
        case SYS_PAIRING:
            handlePairingWindow();
            break;
        case SYS_NORMAL:
            handleNormalMode();
            break;
    }
}

// ── Pairing state ────────────────────────────────────────────

void PairingManager::enterPairingMode() {
    _state              = SYS_PAIRING;
    _pairingWindowStart = millis();
    _ledOn              = false;
    _ledPhaseStart      = millis();
    digitalWrite(PIN_BULB_LED, LOW);
}

void PairingManager::handlePairingWindow() {
    updatePairingLED();

    // Check for timeout
    if (millis() - _pairingWindowStart >= PAIRING_TIMEOUT_MS) {
        // Timeout: enter unpaired fallback — accept only PAIR/FACTORY_RESET
        digitalWrite(PIN_BULB_LED, LOW);
        _state = SYS_UNPAIRED;
        return;
    }

    // Poll radio for incoming public packet
    Packet pkt;
    uint8_t pipe;
    if (!_radio->receive(pkt, pipe)) return;
    if (pipe != 0) return;  // Ignore anything not on public pipe

    handlePublicPacket(pkt);
}

void PairingManager::handlePublicPacket(const Packet& pkt) {
    if (pkt.cmd == CMD_FACTORY_RESET) {
        processFactoryReset();
        return;
    }

    // Only accept PAIR if still in pairing mode (not when already paired)
    if (pkt.cmd == CMD_PAIR && _state == SYS_PAIRING) {
        // Generate token and store pairing data
        PairingData data;
        data.token      = generateToken();
        memcpy(data.remote_addr, pkt.remote_addr, 5);
        data.sensor_mode = MODE_AUTO;
        _stor->save(data);

        // Send ACK to remote on public pipe
        Packet ack;
        ack.cmd   = CMD_PAIR_ACK;
        ack.token = data.token;
        memcpy(ack.remote_addr, data.remote_addr, 5);
        ack.nonce = (uint8_t)(millis() & 0xFF);
        _radio->sendTo(pkt.remote_addr, ack);

        // Activate normal operation
        _ctrl->activate(data);
        _state = SYS_NORMAL;
        return;
    }
    // All other commands on pipe 0 are silently discarded
}

// ── Normal state — only watching pipe 0 for FACTORY_RESET ────

void PairingManager::handleNormalMode() {
    Packet  pkt;
    uint8_t pipe;
    if (!_radio->receive(pkt, pipe)) return;

    if (pipe == 0) {
        // Only FACTORY_RESET accepted on public pipe after pairing
        if (pkt.cmd == CMD_FACTORY_RESET) {
            processFactoryReset();
        }
        // All other pipe-0 commands (including PAIR) silently ignored
        return;
    }

    if (pipe == 1) {
        // Validated commands go to BulbController
        _ctrl->handleCommand(pkt);
    }
}

// ── Factory reset ─────────────────────────────────────────────

void PairingManager::processFactoryReset() {
    _stor->clear();
    _radio->closePrivatePipe();
    enterPairingMode();
}

// ── Token generation from ADC noise ──────────────────────────

uint32_t PairingManager::generateToken() {
    uint32_t token = 0;
    // Sample the LSB of 32 consecutive ADC reads on LDR pin.
    // ADC noise provides adequate entropy for a 32-bit pairing token.
    for (uint8_t i = 0; i < 32; i++) {
        token = (token << 1) | (analogRead(PIN_LDR) & 0x01);
        delayMicroseconds(50);
    }
    // XOR with millis() for extra entropy
    return token ^ millis();
}

// ── Non-blocking pairing LED: 1s ON, 2s OFF repeating ────────

void PairingManager::updatePairingLED() {
    uint32_t elapsed = millis() - _ledPhaseStart;

    if (_ledOn && elapsed >= PAIR_BLINK_ON_MS) {
        _ledOn         = false;
        _ledPhaseStart = millis();
        digitalWrite(PIN_BULB_LED, LOW);
    } else if (!_ledOn && elapsed >= PAIR_BLINK_OFF_MS) {
        _ledOn         = true;
        _ledPhaseStart = millis();
        digitalWrite(PIN_BULB_LED, HIGH);
    }
}
