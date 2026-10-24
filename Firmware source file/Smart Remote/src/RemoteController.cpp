// ============================================================
// RemoteController.cpp — Remote
// ============================================================
#include "RemoteController.h"

void RemoteController::begin(Storage* stor, Radio* radio,
                              ButtonHandler* btn, LEDFeedback* led) {
    _stor  = stor;
    _radio = radio;
    _btn   = btn;
    _led   = led;
}

void RemoteController::handleWake() {
    ButtonEvent evt = _btn->detect();

    switch (evt) {
        case BTN_POWER_TAP:     handlePowerTap();     break;
        case BTN_POWER_PAIR:    handlePairRequest();  break;
        case BTN_POWER_RESET:   handleFactoryReset(); break;
        case BTN_SENSOR_ON:     handleSensorOn();     break;
        case BTN_SENSOR_OFF:    handleSensorOff();    break;
        case BTN_NONE:
        default:
            // Spurious wake — go back to sleep with no feedback
            break;
    }
}

// ── Button handlers ──────────────────────────────────────────

void RemoteController::handlePowerTap() {
    if (!_stor->isPaired()) {
        _led->error();  // 3 blinks = not paired
        return;
    }
    // Power toggle: bulb controller tracks state — we send ON then OFF alternately.
    // Simplification: always send CMD_POWER_ON. Bulb implements toggle internally.
    // If bulb is already ON it processes as the second press (OFF).
    // NOTE: To implement proper toggle here, store bulb state in EEPROM.
    // For now: single POWER command, bulb handles toggling.
    Packet pkt;
    buildPacket(pkt, CMD_POWER_ON, true);
    sendAndFeedback(pkt, false);
}

void RemoteController::handlePairRequest() {
    doParingSequence();
}

void RemoteController::handleFactoryReset() {
    Packet pkt;
    buildPacket(pkt, CMD_FACTORY_RESET, false);  // No token needed for reset
    SendResult result = _radio->sendPublic(pkt);
    if (result == SEND_OK) {
        _stor->clear();          // Wipe local pairing data too
        _led->factoryReset();    // 6 blinks
    } else {
        _led->error();
    }
}

void RemoteController::handleSensorOn() {
    if (!_stor->isPaired()) {
        _led->error();  // 3 blinks = not paired
        return;
    }
    Packet pkt;
    buildPacket(pkt, CMD_SENSOR_ON, true);
    sendAndFeedback(pkt, false);
}

void RemoteController::handleSensorOff() {
    if (!_stor->isPaired()) {
        _led->error();  // 3 blinks = not paired
        return;
    }
    Packet pkt;
    buildPacket(pkt, CMD_SENSOR_OFF, true);
    sendAndFeedback(pkt, false);
}

// ── Pairing sequence ─────────────────────────────────────────

void RemoteController::doParingSequence() {
    Packet pkt;
    buildPacket(pkt, CMD_PAIR, false);  // No token yet

    // Send PAIR on public pipe
    SendResult result = _radio->sendPublic(pkt);
    if (result != SEND_OK) {
        _led->error();
        return;
    }

    // Fast-blink LED while waiting for bulb's PAIR_ACK
    Packet reply;
    uint32_t deadline = millis() + 5000UL;  // Wait up to 5s for ACK
    bool gotAck = false;

    while (millis() < deadline) {
        _led->pairingBlink();
        if (_radio->waitForPairAck(reply)) {
            gotAck = true;
            break;
        }
    }

    if (!gotAck) {
        _led->error();
        return;
    }

    // Store token received from bulb
    _stor->savePairing(reply.token);

    // Open private pipe with our remote addr (now we're paired)
    uint8_t myAddr[5];
    _stor->getRemoteAddr(myAddr);
    _radio->setPrivatePipe(myAddr);

    _led->pairSuccess();  // 1 blink = done
}

// ── Packet builder ────────────────────────────────────────────

void RemoteController::buildPacket(Packet& pkt, uint8_t cmd,
                                    bool includeToken) const {
    pkt.cmd   = cmd;
    pkt.token = includeToken ? _stor->getToken() : 0;
    pkt.nonce = (uint8_t)(millis() & 0xFF);
    _stor->getRemoteAddr(pkt.remote_addr);
}

// ── Send helper ───────────────────────────────────────────────

void RemoteController::sendAndFeedback(const Packet& pkt, bool usePublic) {
    SendResult result = usePublic
        ? _radio->sendPublic(pkt)
        : _radio->sendPrivate(pkt);

    switch (result) {
        case SEND_OK:      _led->ack();   break;
        case SEND_FAIL:    _led->error(); break;
        case SEND_NO_PIPE: _led->error(); break;
    }
}
