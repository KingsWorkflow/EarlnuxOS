/* ============================================================================
 *  EarlnuxOS - Programmable Interval Timer (PIT) Driver
 * kernel/drivers/pit.c
 * ============================================================================ */

#include <arch/x86/ports.h>
#include <kernel/kernel.h>

/* PIT channel 0 data port */
#define PIT_CH0       0x40
#define PIT_CMD       0x43

/* PIT frequency */
#define PIT_INPUT_FREQ 1193180
#define PIT_DEFAULT_HZ 1000

/* Global tick counter (incremented by IRQ0) */
volatile uint32_t system_ticks = 0;

/* Initialize PIT to run at given frequency (Hz) */
void pit_init(uint32_t hz) {
    uint16_t divisor = (uint16_t)(PIT_INPUT_FREQ / hz);

    /* Send command: channel 0, access mode 3 (square wave), binary mode */
    outb(PIT_CMD, 0x36);  /* 00 11 011 0: channel0, LSB then MSB, mode3, binary */

    /* Send divisor (low byte, then high byte) */
    outb(PIT_CH0, divisor & 0xFF);
    outb(PIT_CH0, (divisor >> 8) & 0xFF);

    /* Reset tick counter */
    system_ticks = 0;

    KINFO("pit", "PIT initialized at %u Hz (divisor %u)", hz, divisor);
}

/* Wait using PIT (busy loop for approximately ms milliseconds) */
void pit_msleep(uint32_t ms) {
    uint32_t target = system_ticks + ms;
    while (system_ticks < target) {
        /* Halt to save power */
        __asm__ volatile("hlt");
    }
}

/* Get uptime in milliseconds */
uint32_t timer_get_uptime_ms(void) {
    return system_ticks;
}
