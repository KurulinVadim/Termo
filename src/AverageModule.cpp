#include "AverageModule.h"

bool AverageModule::begin() {
    EEPROM.begin(4);
    storageReady_ = EEPROM.length() >= 4;
    if (!storageReady_) Serial.println("Average settings storage unavailable; using 5 minutes.");
    if (storageReady_ && EEPROM.read(0) == 0x54 && EEPROM.read(1) == 1 &&
        EEPROM.read(3) == static_cast<uint8_t>(~EEPROM.read(2))) {
        const uint8_t saved = EEPROM.read(2);
        minutes_ = saved >= 1 && saved <= 60 ? saved : 5;
    }
    return true; // Averaging works even if persistent storage is unavailable.
}

void AverageModule::update() {
    if (sensor_.sampleId() != lastSampleId_) {
        lastSampleId_ = sensor_.sampleId();
        history_.add(sensor_.temperature(), sensor_.sampledAt());
    }
    temperature_ = history_.value(millis(), minutes_);
}

bool AverageModule::saveMinutes(uint8_t minutes) {
    if (minutes < 1 || minutes > 60 || !storageReady_) return false;
    if (minutes == minutes_) return true;
    uint8_t previous[4];
    for (int i = 0; i < 4; ++i) previous[i] = EEPROM.read(i);
    EEPROM.write(0, 0x54);
    EEPROM.write(1, 1);
    EEPROM.write(2, minutes);
    EEPROM.write(3, static_cast<uint8_t>(~minutes));
    if (!EEPROM.commit()) {
        for (int i = 0; i < 4; ++i) EEPROM.write(i, previous[i]);
        return false;
    }
    minutes_ = minutes;
    temperature_ = history_.value(millis(), minutes_);
    return true;
}
