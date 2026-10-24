// ============================================================
// RemoteController.h / .cpp — Remote (ATtiny1616-s)
//
// Orchestrates one full wake cycle:
//   detect button → build packet → send → LED feedback → sleep
//
// The remote has no continuous loop. Every wakeup is one event.
// ============================================================
#pragma once
#include <Arduino.h>
#include "Storage.h"
#include "Radio.h"
#include "ButtonHandler.h"
#include "LEDFeedback.h"
#include "Protocol.h"

class RemoteController {
public:
    void begin(Storage* stor, Radio* radio,
               ButtonHandler* btn, LEDFeedback* led);

    // Call once per wakeup. Handles the full event, then returns.
    // After this returns, the main sketch puts the MCU to sleep.
    void handleWake();

private:
    void handlePowerTap();
    void handlePairRequest();
    void handleFactoryReset();
    void handleSensorOn();
    void handleSensorOff();

    void buildPacket(Packet& pkt, uint8_t cmd, bool includeToken) const;
    void sendAndFeedback(const Packet& pkt, bool usePublic);
    void doParingSequence();

    Storage*       _stor;
    Radio*         _radio;
    ButtonHandler* _btn;
    LEDFeedback*   _led;
};
