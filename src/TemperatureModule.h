#pragma once

#include <IModule.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

class TemperatureModule : public IModule {
public:
    explicit TemperatureModule(int pin) : wire_(pin), sensors_(&wire_) {}
    bool begin() override;
    void update() override;
    float temperature() const { return temperature_; }
    uint32_t sampleId() const { return sampleId_; }
    uint32_t sampledAt() const { return sampledAt_; }

private:
    OneWire wire_;
    DallasTemperature sensors_;
    DeviceAddress address_ = {};
    float temperature_ = NAN;
    uint32_t startedAt_ = 0;
    bool converting_ = false;
    uint32_t sampleId_ = 0;
    uint32_t sampledAt_ = 0;
};
