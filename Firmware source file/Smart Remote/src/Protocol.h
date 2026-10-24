// ============================================================
// Protocol.h — Shared between Bulb and Remote projects
// Copy this file verbatim into both sketch folders.
// ============================================================
#pragma once
#include <stdint.h>

// ── Public pipe address (hardcoded, factory programmed) ──────
static const uint8_t PUBLIC_PIPE[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

// ── Command byte definitions ─────────────────────────────────
enum Command : uint8_t {
    CMD_PAIR          = 0x01,   // Remote → Bulb on pipe 0
    CMD_PAIR_ACK      = 0x02,   // Bulb   → Remote on pipe 0
    CMD_FACTORY_RESET = 0x03,   // Remote → Bulb on pipe 0
    CMD_POWER_ON      = 0x10,   // Remote → Bulb on pipe 1
    CMD_POWER_OFF     = 0x11,   // Remote → Bulb on pipe 1
    CMD_SENSOR_ON     = 0x12,   // Remote → Bulb on pipe 1
    CMD_SENSOR_OFF    = 0x13,   // Remote → Bulb on pipe 1
};

// ── Packet layout (same in both directions) ──────────────────
// Total: 11 bytes. Well within nRF24's 32-byte max payload.
struct Packet {
    uint8_t  cmd;               // One of Command enum values
    uint32_t token;             // Pairing token; 0 for PAIR / FACTORY_RESET
    uint8_t  remote_addr[5];    // Remote's unique MAC (from SIGROW on ATtiny)
    uint8_t  nonce;             // Random byte — prevents simple replay attacks
} __attribute__((packed));

static const uint8_t PACKET_SIZE = sizeof(Packet);
