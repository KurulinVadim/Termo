#pragma once

#include <IModule.h>
#include <EncButton.h>

class EncoderModule : public IModule {
public:
    EncoderModule(int clkPin, int dtPin, int buttonPin)
        : clkPin_(clkPin), dtPin_(dtPin), buttonPin_(buttonPin) {}

    bool begin() override;
    void update() override;
    int takeRotation();
    bool takePress();

private:
    int clkPin_;
    int dtPin_;
    int buttonPin_;
    // Virtual class keeps GPIO initialization inside begin(), not globals.
    VirtEncButton input_;
    int rotation_ = 0;
    bool pressed_ = false;
};
