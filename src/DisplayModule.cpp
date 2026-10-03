#include "DisplayModule.h"
#include <Arduino.h>

bool DisplayModule::begin() {
    const int status = LcdUi::begin(sdaPin_, sclPin_);
    if (status != 0) {
        Serial.printf("LCD initialization failed: %d\n", status);
        return false;
    }
    update();
    Serial.println("LCD ready.");
    return true;
}

void DisplayModule::update() {
    const int rotation = encoder_.takeRotation();
    const bool press = encoder_.takePress();
    if (screen_ == Screen::AverageEditor) {
        const int64_t next = int64_t(editedMinutes_) + rotation;
        editedMinutes_ = next < 1 ? 1 : next > 60 ? 60 : uint8_t(next);
        if (rotation) saveFailed_ = false;
        if (press) {
            saveFailed_ = !average_.saveMinutes(editedMinutes_);
            if (!saveFailed_) screen_ = Screen::Settings;
        }
    } else if (screen_ == Screen::Home) {
        if (press) {
            selected_ = 0;
            screen_ = Screen::Menu;
        }
    } else {
        const int64_t next = int64_t(selected_) + rotation;
        selected_ = next < 0 ? 0 : next > 1 ? 1 : uint8_t(next);
        if (press) {
            if (screen_ == Screen::Menu) {
                screen_ = selected_ == 0 ? Screen::Settings : Screen::Home;
            } else if (selected_ == 0) {
                editedMinutes_ = average_.minutes();
                saveFailed_ = false;
                screen_ = Screen::AverageEditor;
            } else {
                screen_ = Screen::Menu;
            }
            selected_ = 0;
        }
    }

    static const char* const menuItems[] = {"Settings", "Back"};
    static const char* const settingsItems[] = {"Avg period", "Back"};
    switch (screen_) {
        case Screen::Home: {
            LcdUi::HomeData data = data_;
            data.averageTemperature = average_.temperature();
            data.averageMinutes = average_.minutes();
            LcdUi::showHome(data);
            break;
        }
        case Screen::Menu: LcdUi::showMenu("Menu", menuItems, 2, selected_); break;
        case Screen::Settings: LcdUi::showMenu("Settings", settingsItems, 2, selected_); break;
        case Screen::AverageEditor: LcdUi::showAverageEditor(editedMinutes_, saveFailed_); break;
    }
}
