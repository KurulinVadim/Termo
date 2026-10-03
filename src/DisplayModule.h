#pragma once

#include <IModule.h>
#include <LcdUi.h>
#include "EncoderModule.h"
#include "AverageModule.h"

class DisplayModule : public IModule {
public:
    DisplayModule(int sdaPin, int sclPin, const LcdUi::HomeData& data,
                  EncoderModule& encoder, AverageModule& average)
        : sdaPin_(sdaPin), sclPin_(sclPin), data_(data), encoder_(encoder), average_(average) {}

    bool begin() override;
    void update() override;

private:
    int sdaPin_;
    int sclPin_;
    const LcdUi::HomeData& data_;
    EncoderModule& encoder_;
    AverageModule& average_;
    enum class Screen { Home, Menu, Settings, AverageEditor };
    Screen screen_ = Screen::Home;
    uint8_t selected_ = 0;
    uint8_t editedMinutes_ = 5;
    bool saveFailed_ = false;
};
