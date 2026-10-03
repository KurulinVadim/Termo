#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>

// Arithmetic mean of valid samples within a time window, not a mean of means.
// Capacity covers 60 minutes at the sensor's minimum 2-second sample interval.
class TemperatureAverage {
public:
    void add(float value, uint32_t timestamp) {
        expire(timestamp);
        if (!isfinite(value)) return;
        if (count_ == capacity) {
            head_ = (head_ + 1) % capacity;
            --count_;
        }
        samples_[(head_ + count_) % capacity] = {value, timestamp};
        ++count_;
    }

    float value(uint32_t now, uint8_t minutes) {
        expire(now);
        if (minutes < 1 || minutes > 60) return NAN;
        const uint32_t window = uint32_t(minutes) * 60000;
        double sum = 0;
        size_t used = 0;
        for (size_t i = 0; i < count_; ++i) {
            const Sample& sample = samples_[(head_ + i) % capacity];
            if (uint32_t(now - sample.timestamp) < window) {
                sum += sample.value;
                ++used;
            }
        }
        return used ? float(sum / used) : NAN;
    }

private:
    void expire(uint32_t now) {
        while (count_ && uint32_t(now - samples_[head_].timestamp) >= 3600000UL) {
            head_ = (head_ + 1) % capacity;
            --count_;
        }
    }
    struct Sample { float value; uint32_t timestamp; };
    static constexpr size_t capacity = 1801;
    Sample samples_[capacity] = {};
    size_t head_ = 0;
    size_t count_ = 0;
};
