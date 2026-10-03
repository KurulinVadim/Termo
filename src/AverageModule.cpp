#include "AverageModule.h"

bool AverageModule::begin() {
    storageReady_ = preferences_.begin("termo", false);
    if (storageReady_) {
        const uint8_t saved = preferences_.getUChar("avg_minutes", 5);
        minutes_ = saved >= 1 && saved <= 60 ? saved : 5;
    } else {
        Serial.println("Average settings storage unavailable; using 5 minutes.");
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
    if (preferences_.putUChar("avg_minutes", minutes) != sizeof(uint8_t)) {
        return false;
    }
    minutes_ = minutes;
    temperature_ = history_.value(millis(), minutes_);
    return true;
}
