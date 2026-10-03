#include "TemperatureModule.h"

namespace {
constexpr uint32_t samplePeriodMs = 2000;
constexpr uint32_t conversionMs = 800; // Covers 12-bit DS18B20 and DS1820.
}

bool TemperatureModule::begin() {
    sensors_.begin();
    sensors_.setWaitForConversion(false);
    temperature_ = NAN;
    converting_ = false;
    startedAt_ = uint32_t(millis()) - samplePeriodMs;
    // Stay active even without a sensor, so reconnecting can recover.
    return true;
}

void TemperatureModule::update() {
    const uint32_t now = millis();
    if (converting_) {
        if (uint32_t(now - startedAt_) < conversionMs) {
            return;
        }
        const float value = sensors_.getTempC(address_);
        temperature_ = isfinite(value) && value >= -55.0f && value <= 125.0f
                           ? value : NAN;
        converting_ = false;
        sampledAt_ = millis();
        ++sampleId_;
        if (isfinite(temperature_)) {
            Serial.printf("Room: %.2f C\n", temperature_);
        } else {
            Serial.println("Temperature read failed.");
        }
        return;
    }
    if (uint32_t(now - startedAt_) < samplePeriodMs) {
        return;
    }
    startedAt_ = now;
    // One sensor on the bus. Rediscover to recover after unplugging/replacing it.
    wire_.reset_search();
    if (!wire_.search(address_) || !sensors_.validAddress(address_) ||
        (address_[0] != DS18S20MODEL && address_[0] != DS18B20MODEL)) {
        temperature_ = NAN;
        Serial.println("DS1820/DS18B20 not found.");
        return;
    }
    if (!sensors_.requestTemperaturesByAddress(address_)) {
        temperature_ = NAN;
        Serial.println("Temperature request failed.");
        return;
    }
    startedAt_ = millis();
    converting_ = true;
}
