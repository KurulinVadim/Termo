#include "EncoderModule.h"
#include <Arduino.h>
#include <limits.h>

bool EncoderModule::begin() {
    pinMode(clkPin_, INPUT_PULLUP);
    pinMode(dtPin_, INPUT_PULLUP);
    pinMode(buttonPin_, INPUT_PULLUP);
    input_.setBtnLevel(LOW);
    input_.setEncType(EB_STEP4_LOW);
    input_.setEncReverse(false);
    input_.initEnc(digitalRead(clkPin_), digitalRead(dtPin_));
    // Seed the initial button state without emitting a boot-time press.
    input_.setDebTimeout(0);
    input_.tick(digitalRead(clkPin_), digitalRead(dtPin_), digitalRead(buttonPin_));
    input_.clear();
    input_.setDebTimeout(50);
    rotation_ = 0;
    pressed_ = false;
    return true;
}

void EncoderModule::update() {
    input_.tick(digitalRead(clkPin_), digitalRead(dtPin_), digitalRead(buttonPin_));
    // Library events remain set until tick/clear: consume them here only.
    const int step = input_.turn() ? input_.dir() : 0;
    if ((step > 0 && rotation_ < INT_MAX) || (step < 0 && rotation_ > INT_MIN)) {
        rotation_ += step;
    }
    if (input_.press()) {
        pressed_ = true;
    }
    input_.clear();
}

int EncoderModule::takeRotation() {
    const int result = rotation_;
    rotation_ = 0;
    return result;
}

bool EncoderModule::takePress() {
    const bool result = pressed_;
    pressed_ = false;
    return result;
}
