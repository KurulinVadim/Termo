#include "LcdUi.h"

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

namespace {
constexpr uint8_t columns = 20;
constexpr uint8_t rows = 4;
hd44780_I2Cexp lcd;  // Automatically detect the backpack address and pin mapping.
char previous[rows][columns + 1] = {};
bool ready = false;

void writeRow(uint8_t row, const char* text) {
    char padded[columns + 1];
    memset(padded, ' ', columns);
    padded[columns] = '\0';
    const size_t length = strlen(text);
    memcpy(padded, text, length < columns ? length : columns);
    if (strcmp(previous[row], padded) == 0) {
        return;
    }
    lcd.setCursor(0, row);
    lcd.print(padded);
    memcpy(previous[row], padded, sizeof(padded));
}
}  // namespace

int LcdUi::begin(int sdaPin, int sclPin) {
    ready = false;
    if (!Wire.begin(sdaPin, sclPin, 100000)) {
        return -1;
    }
    const int status = lcd.begin(columns, rows);
    if (status != 0) {
        return status;
    }
    lcd.backlight();
    memset(previous, 0, sizeof(previous));
    ready = true;
    return 0;
}

void LcdUi::showHome(const HomeData& data) {
    if (!ready) {
        return;
    }
    char line[32];
    if (isfinite(data.roomTemperature)) {
        snprintf(line, sizeof(line), "Room:   %.1f C", data.roomTemperature);
    } else {
        snprintf(line, sizeof(line), "Room:   --.- C");
    }
    writeRow(0, line);
    if (isfinite(data.averageTemperature)) {
        snprintf(line, sizeof(line), "Avg %um: %.1f C", unsigned(data.averageMinutes), data.averageTemperature);
    } else {
        snprintf(line, sizeof(line), "Avg %um: --.- C", unsigned(data.averageMinutes));
    }
    writeRow(1, line);
    snprintf(line, sizeof(line), "Target: %.1f C", data.targetTemperature);
    writeRow(2, line);
    writeRow(3, data.pumpOn ? "Pump:   ON" : "Pump:   OFF");
}

void LcdUi::showMenu(const char* title, const char* const* items, uint8_t count, uint8_t selected) {
    if (!ready) return;
    writeRow(0, title);
    const uint8_t first = selected >= 3 ? selected - 2 : 0;
    for (uint8_t row = 1; row < rows; ++row) {
        const unsigned index = first + row - 1;
        char line[columns + 1] = {};
        if (index < count) {
            snprintf(line, sizeof(line), "%c %s", index == selected ? '>' : ' ', items[index]);
        }
        writeRow(row, line);
    }
}

void LcdUi::showAverageEditor(uint8_t minutes, bool saveFailed) {
    if (!ready) return;
    char line[columns + 1];
    snprintf(line, sizeof(line), "Period: %u min", unsigned(minutes));
    writeRow(0, "Avg period");
    writeRow(1, line);
    writeRow(2, "Turn: change 1-60");
    writeRow(3, saveFailed ? "Save failed. Retry" : "Press: save");
}
