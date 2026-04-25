#include <kernel/kernel.h>
#include <stdint.h>

void pit_init(uint32_t frequency) {
    (void)frequency;
    KINFO("PIT", "Programmable Interval Timer initialized (stub)\n");
}

uint64_t timer_get_uptime_ms(void) {
    static uint64_t uptime = 0;
    uptime += 10; // Increment by 10ms each call
    return uptime;
}