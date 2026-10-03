#pragma once
#include <stdint.h>

namespace LcdUi {

struct HomeData {
    float roomTemperature;
    float targetTemperature;
    bool pumpOn;
    float averageTemperature;
    uint8_t averageMinutes;
};

// Returns zero on success; otherwise the LCD driver's error code.
int begin(int sdaPin, int sclPin);
void showHome(const HomeData& data);
void showMenu(const char* title, const char* const* items, uint8_t count, uint8_t selected);
void showAverageEditor(uint8_t minutes, bool saveFailed);

}  // namespace LcdUi
