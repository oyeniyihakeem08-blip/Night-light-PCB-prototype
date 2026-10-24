// ============================================================
// Config.h — Remote (ATtiny1616-s)
// ============================================================
#pragma once

// ── Pin Definitions ──────────────────────────────────────────
#define PIN_BTN_POWER       PIN_PB1   // POWER button (active low, internal pull-up)
#define PIN_BTN_SENSOR_ON   PIN_PB0   // SENSOR ON button (active low, internal pull-up)
#define PIN_BTN_SENSOR_OFF  PIN_PB2   // SENSOR OFF button (active low, internal pull-up)
#define PIN_LED             PIN_PA7   // Status LED (active high)
#define PIN_NRF_CSN         PIN_PC0   // nRF24L01P chip select
#define PIN_NRF_CE          PIN_PC1   // nRF24L01P chip enable
#define PIN_NRF_IRQ         PIN_PC2   // nRF24L01P interrupt (active low, input)
// SPI pins (PA1=MOSI, PA2=MISO, PA3=SCK) used by hardware SPI peripheral — no defines needed

// ── Button Hold Thresholds (milliseconds) ────────────────────
#define HOLD_PAIR_MS            3000UL  // POWER hold > 3s = enter pairing
#define HOLD_RESET_MS           6000UL  // POWER hold > 6s = factory reset
#define DEBOUNCE_MS             50UL    // Debounce window
#define ACK_WAIT_MS             150UL   // Max time to wait for bulb ACK

// ── LED Blink Timing ─────────────────────────────────────────
#define BLINK_SHORT_MS          100UL   // Single ACK blink duration
#define BLINK_FAST_MS           80UL    // Fast blink half-period (pairing)
#define BLINK_GAP_MS            100UL   // Gap between blink pulses

// ── Radio ─────────────────────────────────────────────────────
#define RADIO_CHANNEL           76
#define RADIO_SPEED             RF24_250KBPS
#define RADIO_POWER             RF24_PA_LOW
#define RADIO_RETRY_DELAY       5
#define RADIO_RETRY_COUNT       15

// ── EEPROM Addresses ─────────────────────────────────────────
#define EEPROM_MAGIC_ADDR       0       // 1 byte
#define EEPROM_TOKEN_ADDR       1       // 4 bytes
#define EEPROM_MAGIC_VALUE      0xCD    // Sentinel
