// ============================================================
// Radio.cpp — Bulb
// ============================================================
#include "Radio.h"
#include "Config.h"

Radio::Radio()
    : _radio(PIN_NRF_CE, PIN_NRF_CSN),
      _privatePipeOpen(false) {
    memset(_privateAddr, 0, 5);
}

void Radio::begin() {
    _radio.begin();
    _radio.setChannel(RADIO_CHANNEL);
    _radio.setDataRate(RADIO_SPEED);
    _radio.setPALevel(RADIO_POWER);
    _radio.setPayloadSize(PACKET_SIZE);
    _radio.enableAckPayload();
    _radio.setRetries(RADIO_RETRY_DELAY, RADIO_RETRY_COUNT);

    // Pipe 0 always open — receives PAIR and FACTORY_RESET
    _radio.openReadingPipe(0, PUBLIC_PIPE);
    _radio.startListening();
}

void Radio::openPrivatePipe(const uint8_t* remote_addr) {
    memcpy(_privateAddr, remote_addr, 5);
    _radio.stopListening();
    _radio.openReadingPipe(1, _privateAddr);
    _radio.startListening();
    _privatePipeOpen = true;
}

void Radio::closePrivatePipe() {
    _radio.stopListening();
    // RF24 has no explicit "close pipe" — re-init pipe 0 only
    _radio.openReadingPipe(0, PUBLIC_PIPE);
    _radio.startListening();
    _privatePipeOpen = false;
}

bool Radio::receive(Packet& pkt, uint8_t& pipe) {
    if (!_radio.available(&pipe)) return false;
    _radio.read(&pkt, PACKET_SIZE);
    return true;
}

bool Radio::sendTo(const uint8_t* addr, const Packet& pkt) {
    _radio.stopListening();
    _radio.openWritingPipe(addr);
    // RF24 automatically mirrors this address onto pipe 0 RX
    // to catch the hardware ACK — no manual workaround needed.
    bool ok = _radio.write(&pkt, PACKET_SIZE);
    // Restore pipe 0 listening after send
    _radio.openReadingPipe(0, PUBLIC_PIPE);
    _radio.startListening();
    return ok;
}

bool Radio::sendPrivate(const Packet& pkt) {
    if (!_privatePipeOpen) return false;
    _radio.stopListening();
    _radio.openWritingPipe(_privateAddr);
    bool ok = _radio.write(&pkt, PACKET_SIZE);
    _radio.openReadingPipe(0, PUBLIC_PIPE);
    if (_privatePipeOpen) {
        _radio.openReadingPipe(1, _privateAddr);
    }
    _radio.startListening();
    return ok;
}
