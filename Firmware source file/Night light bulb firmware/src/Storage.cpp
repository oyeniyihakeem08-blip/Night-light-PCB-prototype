// ============================================================
// Storage.cpp — Bulb
// ============================================================
#include "Storage.h"
#include "Config.h"
#include <EEPROM.h>

void Storage::begin() {
    EEPROM.begin();  // Required for STM32 EEPROM emulation init
    uint8_t magic;
    EEPROM.get(EEPROM_MAGIC_ADDR, magic);
    _paired = (magic == EEPROM_MAGIC_VALUE);
}

bool Storage::isPaired() const {
    return _paired;
}

bool Storage::load(PairingData& out) const {
    if (!_paired) return false;
    EEPROM.get(EEPROM_TOKEN_ADDR, out.token);
    for (uint8_t i = 0; i < 5; i++) {
        out.remote_addr[i] = EEPROM.read(EEPROM_REMOTE_ADDR_ADDR + i);
    }
    out.sensor_mode = EEPROM.read(EEPROM_SENSOR_MODE_ADDR);
    return true;
}

void Storage::save(const PairingData& data) {
    EEPROM.put(EEPROM_TOKEN_ADDR, data.token);
    for (uint8_t i = 0; i < 5; i++) {
        EEPROM.write(EEPROM_REMOTE_ADDR_ADDR + i, data.remote_addr[i]);
    }
    EEPROM.write(EEPROM_SENSOR_MODE_ADDR, data.sensor_mode);
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
    //EEPROM.commit();  // STM32 EEPROM emulation requires explicit commit
    _paired = true;
}

void Storage::saveSensorMode(uint8_t mode) {
    EEPROM.write(EEPROM_SENSOR_MODE_ADDR, mode);
    //EEPROM.commit();
}

void Storage::clear() {
    EEPROM.write(EEPROM_MAGIC_ADDR, 0x00);
    //EEPROM.commit();
    _paired = false;
}
