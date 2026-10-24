#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "Config.h"
#include "Storage.h"
#include "Radio.h"
#include "ButtonHandler.h"
#include "LEDFeedback.h"
#include "RemoteController.h"

// ── Module instances ─────────────────────────────────────────
static Storage          storage;
static Radio            radio;
static ButtonHandler    buttons;
static LEDFeedback      led;
static RemoteController remoteCtrl;

// ── Pin-change ISR (wakes from sleep — no body needed) ───────
ISR(PORTB_PORT_vect) {
    // Clear all PORTB interrupt flags
    PORTB.INTFLAGS = 0xFF;
}

// ── Sleep helper ─────────────────────────────────────────────
static void sleepUntilButton() {
    // Ensure all three button pins can wake us
    // megaTinyCore: use attachInterrupt on the button pins
    // to configure pin-change wake. Already set in begin().
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();      // CPU halts here → wakes on button press
    sleep_disable();
}

// ── Setup ────────────────────────────────────────────────────
void setup() {
    storage.begin();

    uint8_t myAddr[5];
    storage.getRemoteAddr(myAddr);

    radio.begin(myAddr);
    buttons.begin();
    led.begin();

    remoteCtrl.begin(&storage, &radio, &buttons, &led);

    // Configure button pins as wake sources
    // megaTinyCore uses PORT register directly for wake-on-change
    pinMode(PIN_BTN_POWER,     INPUT_PULLUP);
    pinMode(PIN_BTN_SENSOR_ON, INPUT_PULLUP);
    pinMode(PIN_BTN_SENSOR_OFF,INPUT_PULLUP);

    // Enable pin-change interrupt on each button pin
    // PORTB input sense config: both edges
    PORTB.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;  // PB0 = SENSOR_ON
    PORTB.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;  // PB1 = POWER
    PORTB.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc;  // PB2 = SENSOR_OFF

    // If paired, open private pipe
    if (storage.isPaired()) {
        radio.setPrivatePipe(myAddr);
    }
}

// ── Main loop ────────────────────────────────────────────────
void loop() {
    remoteCtrl.handleWake();  // Full event handled here
    sleepUntilButton();       // Back to sleep immediately after
}
