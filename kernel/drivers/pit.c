#include <kernel/kernel.h>
#include <kernel/pic.h>
#include <arch/x86/idt.h>
#include <stdint.h>

/* Forward declaration for ISR handler registration */
extern void isr_register_handler(int irq, void (*handler)(struct regs *));

/* PIT I/O ports */
#define PIT_CMD   0x43
#define PIT_CH0   0x40

/* PIT command byte format */
#define PIT_SEL_CH0   0x00
#define PIT_ACCESS_LOB 0x01
#define PIT_ACCESS_HIB 0x02
#define PIT_ACCESS_LOHI 0x03
#define PIT_MODE_RATEGEN 0x02
#define PIT_MODE_SQUAREWAVE 0x06
#define PIT_BINARY 0x00

/* Timer state */
static volatile uint32_t timer_ticks = 0;
static uint32_t timer_hz = 0;

/* Timer interrupt handler */
static void pit_irq_handler(struct regs *r) {
    (void)r;
    timer_ticks++;
}

void pit_init(uint32_t frequency) {
    uint16_t divisor;
    timer_hz = frequency;
    
    /* Calculate divisor: 1193180 Hz / frequency */
    divisor = (uint16_t)(1193180 / frequency);
    
    /* Program PIT channel 0:
     * - Channel 0 (0x00)
     * - Access mode: lobyte then hibyte (0x30)
     * - Mode 2 (rate generator)
     * - Binary mode (0x00)
     */
    outb(PIT_CMD, 0x36);
    io_wait();
    
    /* Send divisor (low byte then high byte) */
    outb(PIT_CH0, divisor & 0xFF);
    io_wait();
    outb(PIT_CH0, (divisor >> 8) & 0xFF);
    io_wait();
    
    /* Register timer interrupt handler */
    isr_register_handler(INT_IRQ0, pit_irq_handler);
    
    /* Unmask IRQ0 (timer) in PIC */
    pic_unmask_irq(IRQ0);
    
    KINFO("PIT", "Programmable Interval Timer initialized at %u Hz\n", frequency);
}

uint64_t timer_get_uptime_ms(void) {
    uint32_t ticks;
    /* Disable interrupts for atomic read of tick counter */
    cli();
    ticks = timer_ticks;
    sti();
    
    if (timer_hz == 0) return 0;
    return ((uint64_t)ticks * 1000) / timer_hz;
}