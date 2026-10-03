#pragma once

#include <stddef.h>
#include <stdint.h>
#include "IModule.h"

// Modules must outlive the scheduler. Call only from the Arduino loop task.
template <size_t Capacity>
class Scheduler {
    static_assert(Capacity > 0, "Scheduler needs at least one slot");

public:
    using Clock = uint32_t (*)();
    explicit Scheduler(Clock clock) : clock_(clock) {}

    bool add(IModule& module, uint32_t periodMs) {
        if (started_ || count_ == Capacity || periodMs == 0) {
            return false;
        }
        for (size_t i = 0; i < count_; ++i) {
            if (entries_[i].module == &module) {
                return false;
            }
        }
        entries_[count_++] = {&module, periodMs, 0, false};
        return true;
    }

    bool begin() {
        if (started_) {
            return false;
        }
        started_ = true;
        bool success = true;
        for (size_t i = 0; i < count_; ++i) {
            entries_[i].active = entries_[i].module->begin();
            success = entries_[i].active && success;
        }
        const uint32_t now = clock_();
        for (size_t i = 0; i < count_; ++i) {
            entries_[i].lastRun = now;
        }
        return success;
    }

    void tick() {
        if (!started_) {
            return;
        }
        for (size_t i = 0; i < count_; ++i) {
            Entry& entry = entries_[i];
            const uint32_t now = clock_();
            // Unsigned subtraction also handles millis() rollover.
            if (entry.active && uint32_t(now - entry.lastRun) >= entry.periodMs) {
                entry.module->update();
                // Schedule from completion: never replay missed intervals.
                entry.lastRun = clock_();
            }
        }
    }

private:
    struct Entry {
        IModule* module;
        uint32_t periodMs;
        uint32_t lastRun;
        bool active;
    };
    Clock clock_;
    Entry entries_[Capacity] = {};
    size_t count_ = 0;
    bool started_ = false;
};
