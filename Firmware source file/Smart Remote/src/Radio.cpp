// ============================================================
// Radio.cpp — Remote
// ============================================================
#include "Radio.h"

Radio::Radio()
    : _radio(PIN_NRF_CE, PIN_NRF_CSN),
      _privatePipeOpen(false) {}

void Radio::begin(const uint8_t* remote_addr) {
    _radio.begin();
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setDataRate(RADIO_SPEED);
    _radio.setPALevel(RADIO_POWER);
    _radio.setPayloadSize(PACKET_SIZE);
    _radio.setAutoAck(true);
    _radio.setRetries(RADIO_RETRY_DELAY, RADIO_RETRY_COUNT);
    // Remote mostly transmits — start in TX mode
    _radio.stopListening();
}

void Radio::setPrivatePipe(const uint8_t* remote_addr) {
    _radio.openReadingPipe(1, remote_addr);
    _privatePipeOpen = true;
}

SendResult Radio::sendPublic(const Packet& pkt) {
    _radio.stopListening();
    _radio.openWritingPipe(PUBLIC_PIPE);
    bool ok = _radio.write(&pkt, PACKET_SIZE);
    return ok ? SEND_OK : SEND_FAIL;
}

SendResult Radio::sendPrivate(const Packet& pkt) {
    if (!_privatePipeOpen) return SEND_NO_PIPE;
    _radio.stopListening();
    // Remote addr was set as reading pipe 1 — use same addr for writing
    // (bulb's private pipe listens on this address)
    bool ok = _radio.write(&pkt, PACKET_SIZE);
    return ok ? SEND_OK : SEND_FAIL;
}

bool Radio::waitForPairAck(Packet& reply) {
    _radio.startListening();
    uint32_t start = millis();
    while (millis() - start < ACK_WAIT_MS * 10) {  // 10× normal — pairing is special
        if (_radio.available()) {
            _radio.read(&reply, PACKET_SIZE);
            _radio.stopListening();
            return (reply.cmd == CMD_PAIR_ACK);
        }
    }
    _radio.stopListening();
    return false;
}
