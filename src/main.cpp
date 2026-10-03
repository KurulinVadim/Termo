#include <Arduino.h>
#include <Scheduler.h>
#include "DisplayModule.h"
#include "EncoderModule.h"
#include "TemperatureModule.h"
#include "PumpModule.h"

namespace {
constexpr int lcdSdaPin = 21;
constexpr int lcdSclPin = 22;
constexpr int pumpRelayPin = 26;
constexpr bool pumpRelayActiveHigh = true; // Set false for an active-LOW relay.
LcdUi::HomeData homeData{NAN, 23.0f, false, NAN, 5};
EncoderModule encoder(32, 33, 25);
TemperatureModule temperature(27);
AverageModule average(temperature);
DisplayModule display(lcdSdaPin, lcdSclPin, homeData, encoder, average);
PumpModule pump(pumpRelayPin, pumpRelayActiveHigh);
Scheduler<5> scheduler([]() -> uint32_t { return millis(); });
}

void setup() {
    Serial.begin(115200);
    if (!scheduler.add(pump, 100) || !scheduler.add(encoder, 1) ||
        !scheduler.add(temperature, 10) ||
        !scheduler.add(average, 100) || !scheduler.add(display, 20)) {
        Serial.println("Module registration failed.");
        return;
    }
    if (!scheduler.begin()) {
        Serial.println("Some modules failed to initialize.");
    }
}

void loop() {
    scheduler.tick();
    homeData.roomTemperature = temperature.temperature();
    homeData.pumpOn = pump.isOn();
}
