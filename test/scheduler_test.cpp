// Host-only test: no ESP32 or LCD required.
#include <assert.h>
#include <Scheduler.h>

uint32_t now = 0;
uint32_t clockMs() { return now; }

struct Module : IModule {
    bool healthy = true;
    int starts = 0;
    int updates = 0;
    uint32_t duration = 0;
    bool begin() override { ++starts; return healthy; }
    void update() override { ++updates; now += duration; }
};

int main() {
    Module fast, slow, failed, extra;
    failed.healthy = false;
    Scheduler<3> scheduler(clockMs);
    assert(!scheduler.add(fast, 0));
    assert(scheduler.add(fast, 100));
    assert(!scheduler.add(fast, 200));
    assert(scheduler.add(slow, 250));
    assert(scheduler.add(failed, 10));
    assert(!scheduler.add(extra, 100));
    scheduler.tick();
    assert(fast.updates == 0);
    assert(!scheduler.begin());
    assert(fast.starts == 1 && slow.starts == 1 && failed.starts == 1);
    assert(!scheduler.begin());
    assert(fast.starts == 1);
    assert(!scheduler.add(extra, 100));
    now = 99;
    scheduler.tick();
    assert(fast.updates == 0);
    now = 100;
    scheduler.tick();
    assert(fast.updates == 1 && slow.updates == 0);
    now = 250;
    scheduler.tick();
    assert(fast.updates == 2 && slow.updates == 1);
    now = 10000;
    scheduler.tick();
    assert(fast.updates == 3 && slow.updates == 2 && failed.updates == 0);
    scheduler.tick();
    assert(fast.updates == 3 && slow.updates == 2);

    Module rollover;
    Scheduler<2> wrap(clockMs);
    assert(wrap.add(rollover, 100));
    now = UINT32_MAX - 49;
    assert(wrap.begin());
    assert(!wrap.add(extra, 100)); // Rejected even with a free slot.
    now = 49;
    wrap.tick();
    assert(rollover.updates == 0);
    now = 50;
    wrap.tick();
    assert(rollover.updates == 1);

    Module longRunning;
    longRunning.duration = 200;
    Scheduler<1> completion(clockMs);
    assert(completion.add(longRunning, 100));
    now = 0;
    assert(completion.begin());
    now = 100;
    completion.tick();
    assert(now == 300 && longRunning.updates == 1);
    completion.tick();
    assert(longRunning.updates == 1);
    now = 400;
    completion.tick();
    assert(longRunning.updates == 2);
}
