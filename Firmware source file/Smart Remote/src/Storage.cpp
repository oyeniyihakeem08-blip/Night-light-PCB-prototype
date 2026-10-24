// ============================================================
// Storage.cpp — Remote
// ============================================================
#include "Storage.h"
#include "Config.h"
#include <EEPROM.h>
#include <avr/io.h>   // For SIGROW access

void Storage::begin() {
    uint8_t magic = EEPROM.read(EEPROM_MAGIC_ADDR);
    _paired = (magic == EEPROM_MAGIC_VALUE);

    if (_paired) {
        EEPROM.get(EEPROM_TOKEN_ADDR, _token);
    } else {
        _token = 0;
    }
}

bool Storage::isPaired() const {
    return _paired;
}

uint32_t Storage::getToken() const {
    return _token;
}

void Storage::getRemoteAddr(uint8_t* addr) const {
    // ATtiny1616 SIGROW — unique serial number baked in at factory.
    // Using bytes 0–4 gives a stable, hardware-unique 5-byte address.
    addr[0] = SIGROW.SERNUM0;
    addr[1] = SIGROW.SERNUM1;
    addr[2] = SIGROW.SERNUM2;
    addr[3] = SIGROW.SERNUM3;
    addr[4] = SIGROW.SERNUM4;
}

void Storage::savePairing(uint32_t token) {
    _token  = token;
    _paired = true;
    EEPROM.put(EEPROM_TOKEN_ADDR, token);
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
    // ATtiny EEPROM writes are synchronous — no commit() needed
}

void Storage::clear() {
    EEPROM.write(EEPROM_MAGIC_ADDR, 0x00);
    _paired = false;
    _token  = 0;
}
