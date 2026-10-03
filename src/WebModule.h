#pragma once
#include <IModule.h>
#include <WebServer.h>
#include "AverageModule.h"
#include "PumpModule.h"

class WebModule : public IModule {
public:
    WebModule(const TemperatureModule& sensor, AverageModule& average, PumpModule& pump)
        : sensor_(sensor), average_(average), pump_(pump) {}
    bool begin() override;
    void update() override;
private:
    void status();
    bool validMutation();
    const TemperatureModule& sensor_;
    AverageModule& average_;
    PumpModule& pump_;
    WebServer server_{80};
    bool connected_ = false;
    bool configured_ = false;
    uint32_t lastReconnect_ = 0;
    String controlToken_;
};
