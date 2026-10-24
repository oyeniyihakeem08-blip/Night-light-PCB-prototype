// ============================================================
// Storage.h / .cpp — Remote (ATtiny1616-s)
// Stores pairing token in real EEPROM (not emulated — ATtiny has 256 bytes)
// Remote MAC comes from SIGROW (hardware unique serial) — never stored.
// ============================================================
#pragma once
#include <Arduino.h>

class Storage {
public:
    void begin();

    bool isPaired() const;

    // Returns the pairing token (only valid if isPaired() is true)
    uint32_t getToken() const;

    // Returns remote's unique 5-byte address from chip serial number.
    // This is fixed in hardware — same on every boot.
    void getRemoteAddr(uint8_t* addr) const;

    // Store token after successful pairing
    void savePairing(uint32_t token);

    // Wipe pairing data
    void clear();

private:
    bool     _paired;
    uint32_t _token;
};
