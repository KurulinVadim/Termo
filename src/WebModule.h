#pragma once
#include <IModule.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
using TermoWebServer = ESP8266WebServer;
#else
#include <WebServer.h>
using TermoWebServer = WebServer;
#endif
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
    TermoWebServer server_{80};
    bool connected_ = false;
    bool configured_ = false;
    uint32_t lastReconnect_ = 0;
    String controlToken_;
};
