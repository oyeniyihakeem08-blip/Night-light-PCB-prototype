// ============================================================
// LEDFeedback.h / .cpp — Remote (ATtiny1616-s)
// All LED patterns in one place.
// All functions are blocking — they complete before returning.
// Remote is fully awake during feedback; duration is very short.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class LEDFeedback {
public:
    void begin();

    void ack();             // 1 blink — command received
    void error();           // 3 fast blinks — failure / not paired
    void pairingBlink();    // 1 fast blink cycle (call in loop during pairing wait)
    void pairSuccess();     // 1 solid blink — pairing complete
    void factoryReset();    // 6 blinks — reset done

private:
    void blink(uint8_t count, uint16_t onMs, uint16_t offMs);
};
