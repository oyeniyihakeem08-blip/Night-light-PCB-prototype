// ============================================================
// LEDFeedback.cpp — Remote
// ============================================================
#include "LEDFeedback.h"

void LEDFeedback::begin() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
}

void LEDFeedback::ack() {
    blink(1, BLINK_SHORT_MS, 0);
}

void LEDFeedback::error() {
    blink(3, BLINK_FAST_MS, BLINK_FAST_MS);
}

void LEDFeedback::pairingBlink() {
    // One on/off cycle — caller loops this while waiting for bulb
    blink(1, BLINK_FAST_MS, BLINK_FAST_MS);
}

void LEDFeedback::pairSuccess() {
    blink(1, BLINK_SHORT_MS, 0);
}

void LEDFeedback::factoryReset() {
    blink(6, BLINK_SHORT_MS, BLINK_GAP_MS);
}

void LEDFeedback::blink(uint8_t count, uint16_t onMs, uint16_t offMs) {
    for (uint8_t i = 0; i < count; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(onMs);
        digitalWrite(PIN_LED, LOW);
        if (offMs > 0 && i < count - 1) {
            delay(offMs);
        }
    }
}
