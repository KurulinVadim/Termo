#include "PumpModule.h"
#include <Arduino.h>

bool PumpModule::begin() {
    // Preload the inactive level before enabling the output.
    digitalWrite(relayPin_, activeHigh_ ? LOW : HIGH);
    pinMode(relayPin_, OUTPUT);
    ready_ = true;
    setOn(false);
    return true;
}

void PumpModule::update() {
    // Pump control logic will be added later.
}

void PumpModule::setOn(bool on) {
    if (!ready_) {
        return;
    }
    digitalWrite(relayPin_, on == activeHigh_ ? HIGH : LOW);
    on_ = on;
}
