// ============================================================
// Radio.h / .cpp — Remote (ATtiny1616-s)
// ============================================================
#pragma once
#include <Arduino.h>
#include <RF24.h>
#include "Protocol.h"
#include "Config.h"

// Result of a send attempt
enum SendResult : uint8_t {
    SEND_OK,        // Packet sent and ACK received
    SEND_FAIL,      // No ACK after retries
    SEND_NO_PIPE    // Tried private pipe but no pairing established
};

class Radio {
public:
    Radio();
    void begin(const uint8_t* remote_addr);

    // Open pipe 1 to bulb (call after pairing)
    void setPrivatePipe(const uint8_t* remote_addr);

    // Send on public pipe 0 (PAIR, FACTORY_RESET)
    SendResult sendPublic(const Packet& pkt);

    // Send on private pipe 1 (normal commands)
    // Returns SEND_NO_PIPE if not yet paired
    SendResult sendPrivate(const Packet& pkt);

    // Listen for a reply on the public pipe for up to ACK_WAIT_MS.
    // Used during pairing to receive PAIR_ACK.
    bool waitForPairAck(Packet& reply);

private:
    RF24   _radio;
    bool   _privatePipeOpen;
};
