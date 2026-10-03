#pragma once

class IModule {
public:
    virtual ~IModule() = default;
    // A failed module is excluded from periodic updates.
    virtual bool begin() = 0;
    virtual void update() = 0;
};
