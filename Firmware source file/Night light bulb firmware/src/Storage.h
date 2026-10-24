// ============================================================
// Storage.h — Bulb (STM32G031K8T6)
// Wraps EEPROM emulation. One place for all persistent state.
// ============================================================
#pragma once
#include <Arduino.h>

// Everything the bulb needs to remember across power cycles
struct PairingData {
    uint32_t token;
    uint8_t  remote_addr[5];
    uint8_t  sensor_mode;  // 0 = AUTO, 1 = MANUAL
};

class Storage {
public:
    // Call once in setup()
    void begin();

    // True if valid pairing data exists in EEPROM
    bool isPaired() const;

    // Fill 'out' from EEPROM. Returns false if not paired.
    bool load(PairingData& out) const;

    // Write pairing data and mark EEPROM as valid.
    void save(const PairingData& data);

    // Overwrite just the sensor mode (called when user changes mode)
    void saveSensorMode(uint8_t mode);

    // Wipe the valid-data sentinel — bulb will re-enter pairing on next boot
    void clear();

private:
    bool _paired;
};
