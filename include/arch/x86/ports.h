/* ============================================================================
 *  EarlnuxOS - x86 Low-Level Port I/O
 * include/arch/x86/ports.h
 * ============================================================================ */

#ifndef  EarlnuxOS_ARCH_PORTS_H
#define  EarlnuxOS_ARCH_PORTS_H

/* PIC (8259A) interrupt controller ports */
#define PIC1_CMD        0x20
#define PIC1_DATA       0x21
#define PIC2_CMD        0xA0
#define PIC2_DATA       0xA1

/* PIC initialization constants */
#define PIC_ICW1_MASK   0x10
#define PIC_ICW1_ICW4   0x01
#define PIC_ICW4_8086   0x01

/* PIT (8253/8254) programmable interval timer */
#define PIT_CMD        0x43
#define PIT_CH0        0x40
#define PIT_CH1        0x41
#define PIT_CH2        0x42

/* PIT frequency */
#define PIT_INPUT_FREQ  1193180
#define PIT_DEFAULT_HZ  1000

/* Keyboard controller (8042) */
#define KBC_CMD         0x64
#define KBC_DATA        0x60

/* Keyboard PS/2 port */
#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64
#define KB_CMD_PORT     0x64

/* Keyboard commands */
#define KB_CMD_READ     0x20
#define KB_CMD_WRITE    0x60
#define KB_SELF_TEST    0xAA
#define KB_ENABLE       0xAE

/* CMOS/RTC ports */
#define CMOS_ADDR       0x70
#define CMOS_DATA       0x71

/* CMOS registers */
#define CMOS_SECONDS    0x00
#define CMOS_MINUTES    0x02
#define CMOS_HOURS      0x04
#define CMOS_DAY        0x07
#define CMOS_MONTH      0x08
#define CMOS_YEAR       0x09
#define CMOS_STATUS_A   0x0A
#define CMOS_STATUS_B   0x0B

/* A20 line control (using keyboard controller) */
#define A20_ENABLE_KB   0xD1
#define A20_DISABLE_KB  0xD0
#define A20_GATE_A      0x2
#define A20_GATE_B      0xA1

/* Port 0x92 (PS/2 system control port) - faster A20 */
#define PS2_CTRL_PORT   0x92
#define PS2_CTRL_A20    BIT(1)

#endif /*  EarlnuxOS_ARCH_PORTS_H */
