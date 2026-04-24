/* ============================================================================
 *  EarlnuxOS - PS/2 Controller Utilities
 * kernel/drivers/ps2.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <arch/x86/ports.h>

/* Wait for PS/2 controller input buffer to be empty */
static void ps2_wait_write(void) {
    while (inb(KBC_CMD) & 0x02) { /* bit1 = input buffer full */ }
}

/* Wait for PS/2 controller output buffer to have data */
static void ps2_wait_read(void) {
    while (!(inb(KBC_CMD) & 0x01)) { /* bit0 = output buffer full */ }
}

/* Enable the A20 line (via keyboard controller method) */
void enable_a20(void) {
    /* First, try fast A20 gate (port 0x92) */
    uint8_t val = inb(PS2_CTRL_PORT);
    if (!(val & 0x02)) {
        outb(PS2_CTRL_PORT, val | 0x02);
        return;
    }

    /* Keyboard controller method */
    ps2_wait_write();
    outb(KBC_CMD, 0xAD);       /* Disable keyboard */
    io_wait();

    ps2_wait_write();
    outb(KBC_CMD, 0xD0);       /* Read controller output port */
    io_wait();

    ps2_wait_read();
    uint8_t status = inb(KBC_DATA);

    ps2_wait_write();
    outb(KBC_CMD, 0xD1);       /* Write controller output port */
    io_wait();

    outb(KBC_DATA, status | 0x02); /* Set A20 enable bit */
    io_wait();

    ps2_wait_write();
    outb(KBC_CMD, 0xAE);       /* Enable keyboard */
    io_wait();
}

/* Initialize PS/2 controller and keyboard/mouse ports */
void ps2_init(void) {
    /* Get current controller configuration */
    ps2_wait_write();
    outb(KBC_CMD, 0x20);
    ps2_wait_read();
    uint8_t config = inb(KBC_DATA);

    /* Enable interrupts, disable translation, enable A20? */
    config |= 0x01;   /* Enable first PS/2 port interrupt */
    config &= ~0x10;  /* No translation for first port */
    config &= ~0x20;  /* No translation for second port */

    ps2_wait_write();
    outb(KBC_CMD, 0x60);
    ps2_wait_write();
    outb(KBC_DATA, config);
    io_wait();

    /* Perform self-test */
    ps2_wait_write();
    outb(KBC_CMD, 0xAA);
    ps2_wait_read();
    uint8_t test = inb(KBC_DATA);
    if (test != 0x55) {
        KWARN("ps2", "Controller self-test failed: 0x%x", test);
    } else {
        KINFO("ps2", "Controller self-test passed");
    }

    /* Enable keyboard interface */
    ps2_wait_write();
    outb(KBC_CMD, 0xAE);
    io_wait();
}

/* Read a byte from PS/2 data port (blocking) */
uint8_t ps2_read_data(void) {
    ps2_wait_read();
    return inb(KBC_DATA);
}

/* Write a command byte to PS/2 controller */
void ps2_write_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(KBC_CMD, cmd);
}

/* Write data to PS/2 data port */
void ps2_write_data(uint8_t data) {
    ps2_wait_write();
    outb(KBC_DATA, data);
}
