// ============================================================
// Config.h — Bulb (STM32G031K8T6)
// All pins, timing constants, and EEPROM layout in one place.
// Change values here only — never scatter magic numbers in code.
// ============================================================
#pragma once

// ── Pin Definitions ──────────────────────────────────────────
#define PIN_MOTION      PA8     // BISS0001 digital output
#define PIN_LDR         PA1     // LDR voltage divider → ADC
#define PIN_NRF_CSN     PA3     // nRF24L01P chip select
#define PIN_NRF_SCK     PA5     // nRF24L01P SPI clock
#define PIN_NRF_MISO    PA6     // nRF24L01P SPI MISO
#define PIN_NRF_MOSI    PA7     // nRF24L01P SPI MOSI
#define PIN_BULB_LED    PA9     // Status LED (also drives bulb PWM if applicable)
#define PIN_NRF_CE      PB0     // nRF24L01P chip enable
#define PIN_NRF_IRQ     PB1     // nRF24L01P interrupt (active low)

// ── Timing (milliseconds unless noted) ───────────────────────
#define PAIRING_TIMEOUT_MS      60000UL // How long bulb waits for a PAIR command
#define MOTION_TIMEOUT_MS       60000UL // Auto-off after no movement
#define LDR_SETTLE_MS           500UL   // Wait after bulb turns OFF before sampling LDR
#define MAIN_LOOP_INTERVAL_MS   5UL     // Main loop tick rate
#define WATCHDOG_REFRESH_MS     4000UL  // Must be < IWDG timeout (8s)

// ── LED Pattern Timing ────────────────────────────────────────
#define PAIR_BLINK_ON_MS        1000UL  // Pairing mode: LED on duration
#define PAIR_BLINK_OFF_MS       2000UL  // Pairing mode: LED off duration
#define SUCCESS_BLINK_ON_MS     3000UL  // Pair success: 3s on, 2s off, 3s on
#define SUCCESS_BLINK_OFF_MS    2000UL
#define SHORT_FLASH_MS          150UL   // Sensor mode change flash duration

// ── Sensor Calibration ────────────────────────────────────────
// LDR voltage divider: adjust to match your circuit.
// If LDR is pull-down: low ADC = dark, high ADC = bright.
// If LDR is pull-up:  high ADC = dark, low ADC = bright.
// Default assumes pull-down: ADC < threshold means dark room.
#define LDR_DARK_THRESHOLD      512     // 0–1023 (tune on your hardware)

// ── EEPROM Layout (byte addresses) ───────────────────────────
#define EEPROM_MAGIC_ADDR       0       // 1 byte  — valid data sentinel
#define EEPROM_TOKEN_ADDR       1       // 4 bytes — pairing token
#define EEPROM_REMOTE_ADDR_ADDR 5       // 5 bytes — paired remote MAC
#define EEPROM_SENSOR_MODE_ADDR 10      // 1 byte  — 0=AUTO, 1=MANUAL
#define EEPROM_MAGIC_VALUE      0xAB    // Sentinel value

// ── Radio ─────────────────────────────────────────────────────
#define RADIO_CHANNEL           76      // 2.476 GHz — avoid common WiFi channels
#define RADIO_SPEED             RF24_250KBPS
#define RADIO_POWER             RF24_PA_LOW
#define RADIO_RETRY_DELAY       5       // 5 × 250µs = 1.25ms between retries
#define RADIO_RETRY_COUNT       15      // Max retransmissions
