// ============================================================
// Radio.h — Bulb (STM32G031K8T6)
// Thin wrapper around RF24. One place for all radio logic.
// ============================================================
#pragma once
#include <Arduino.h>
#include <RF24.h>
#include "Protocol.h"

class Radio {
public:
    Radio();

    // Initialise nRF24, open pipe 0 (public) for listening
    void begin();

    // Call after successful pairing to open pipe 1 (private)
    // remote_addr = the paired remote's 5-byte MAC
    void openPrivatePipe(const uint8_t* remote_addr);

    // Wipe pipe 1 after factory reset
    void closePrivatePipe();

    // Returns true if a packet is waiting. Fills 'pipe' with 0 or 1.
    // pipe 0 = public channel, pipe 1 = private channel
    bool receive(Packet& pkt, uint8_t& pipe);

    // Send a packet to a specific address (used for PAIR_ACK on pipe 0)
    // Returns true if ACK received from the other side
    bool sendTo(const uint8_t* addr, const Packet& pkt);

    // Send on the private pipe (normal commands ACK).
    // Requires openPrivatePipe() to have been called first.
    bool sendPrivate(const Packet& pkt);

private:
    RF24    _radio;
    uint8_t _privateAddr[5];
    bool    _privatePipeOpen;
};
