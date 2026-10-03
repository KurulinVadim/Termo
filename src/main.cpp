#include <Arduino.h>
#include <Scheduler.h>
#include "TemperatureModule.h"
#include "PumpModule.h"
#include "WebModule.h"
#if __has_include("NetworkConfig.h")
#include "NetworkConfig.h"
#else
#include "NetworkConfig.example.h"
#endif
#if TERMO_LOCAL_DISPLAY
#error "NodeMCU firmware uses the web interface; set TERMO_LOCAL_DISPLAY to 0"
#include "DisplayModule.h"
#include "EncoderModule.h"
#endif

namespace {
constexpr int pumpRelayPin = 12; // D6
constexpr int temperaturePin = 14; // D5
constexpr bool pumpRelayActiveHigh = true; // Set false for an active-LOW relay.
#if TERMO_LOCAL_DISPLAY
constexpr int lcdSdaPin = 21;
constexpr int lcdSclPin = 22;
LcdUi::HomeData homeData{NAN, 23.0f, false, NAN, 5};
EncoderModule encoder(32, 33, 25);
#endif
TemperatureModule temperature(temperaturePin);
AverageModule average(temperature);
#if TERMO_LOCAL_DISPLAY
DisplayModule display(lcdSdaPin, lcdSclPin, homeData, encoder, average);
#endif
PumpModule pump(pumpRelayPin, pumpRelayActiveHigh);
WebModule web(temperature, average, pump);
Scheduler<6> scheduler([]() -> uint32_t { return millis(); });
}

void setup() {
    Serial.begin(115200);
    if (!scheduler.add(pump, 100) ||
        !scheduler.add(temperature, 10) ||
        !scheduler.add(average, 100) || !scheduler.add(web, 10)
#if TERMO_LOCAL_DISPLAY
        || !scheduler.add(encoder, 1) || !scheduler.add(display, 20)
#endif
        ) {
        Serial.println("Module registration failed.");
        return;
    }
    if (!scheduler.begin()) {
        Serial.println("Some modules failed to initialize.");
    }
}

void loop() {
    scheduler.tick();
    yield(); // Service Wi-Fi and the ESP8266 watchdog.
#if TERMO_LOCAL_DISPLAY
    homeData.roomTemperature = temperature.temperature();
    homeData.pumpOn = pump.isOn();
#endif
}
