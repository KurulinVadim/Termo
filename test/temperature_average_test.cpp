#include <assert.h>
#include <TemperatureAverage.h>

int main() {
    TemperatureAverage average;
    assert(isnan(average.value(0, 5)));
    average.add(20.0f, 0);
    average.add(NAN, 1000);
    average.add(INFINITY, 2000);
    average.add(24.0f, 30000);
    assert(average.value(30000, 1) == 22.0f);
    assert(average.value(59999, 1) == 22.0f);
    assert(average.value(60000, 1) == 24.0f); // Left edge is excluded.
    assert(average.value(60000, 5) == 22.0f); // Longer window retains history.
    assert(isnan(average.value(90000, 1)));   // No stale value after expiry.
    assert(average.value(90000, 60) == 22.0f);
    assert(isnan(average.value(3630000, 60)));
    assert(isnan(average.value(3630000, 0)));
    assert(isnan(average.value(3630000, 61)));

    TemperatureAverage wrap;
    const uint32_t start = UINT32_MAX - 30000;
    wrap.add(-10.0f, start);
    wrap.add(10.0f, uint32_t(start + 20000));
    assert(wrap.value(uint32_t(start + 59999), 1) == 0.0f);
    assert(wrap.value(uint32_t(start + 60000), 1) == 10.0f);
    assert(isnan(wrap.value(uint32_t(start + 80000), 1)));

    TemperatureAverage full;
    // More than one full ring: 3600 samples, with exactly 1800 in a 60m window.
    for (uint32_t i = 0; i < 3600; ++i) full.add(float(i), i * 2000);
    assert(full.value(3599 * 2000, 60) == (1800.0f + 3599.0f) / 2);
    assert(full.value(3599 * 2000, 1) == (3570.0f + 3599.0f) / 2);
    assert(full.value(3599 * 2000, 60) == (1800.0f + 3599.0f) / 2);
}
