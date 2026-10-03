#pragma once

#include <IModule.h>

class PumpModule : public IModule {
public:
    PumpModule(int relayPin, bool activeHigh)
        : relayPin_(relayPin), activeHigh_(activeHigh) {}

    bool begin() override;
    void update() override;
    void setOn(bool on);
    bool isOn() const { return on_; }

private:
    int relayPin_;
    bool activeHigh_;
    bool on_ = false;
    bool ready_ = false;
};
