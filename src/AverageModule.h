#pragma once

#include <IModule.h>
#if defined(ESP8266)
#include <EEPROM.h>
#else
#include <Preferences.h>
#endif
#include <TemperatureAverage.h>
#include "TemperatureModule.h"

class AverageModule : public IModule {
public:
    explicit AverageModule(const TemperatureModule& sensor) : sensor_(sensor) {}
    bool begin() override;
    void update() override;
    uint8_t minutes() const { return minutes_; }
    float temperature() const { return temperature_; }
    bool saveMinutes(uint8_t minutes);

private:
    const TemperatureModule& sensor_;
    TemperatureAverage history_;
#if !defined(ESP8266)
    Preferences preferences_;
#endif
    uint32_t lastSampleId_ = 0;
    uint8_t minutes_ = 5;
    float temperature_ = NAN;
    bool storageReady_ = false;
};
