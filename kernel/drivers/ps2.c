/* ============================================================================
 *  EarlnuxOS - PS/2 Controller Utilities
 * kernel/drivers/ps2.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <kernel/ps2.h>
#include <stdint.h>

#define PS2_STATUS_OUTPUT_FULL 0x01
#define PS2_STATUS_INPUT_FULL  0x02

static int wait_for_write(void) {
    for (int i = 0; i < 10000; i++) {
        if ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) == 0) {
            return 0;
        }
    }
    return -1;
}

static int wait_for_read(void) {
    for (int i = 0; i < 10000; i++) {
        if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            return 0;
        }
    }
    return -1;
}

int ps2_write_cmd(uint8_t cmd) {
    if (wait_for_write() < 0) return -1;
    outb(PS2_COMMAND_PORT, cmd);
    return 0;
}

int ps2_write_data(uint8_t data) {
    if (wait_for_write() < 0) return -1;
    outb(PS2_DATA_PORT, data);
    return 0;
}

void ps2_init(void) {
    // Disable devices to prevent interrupts during init
    ps2_write_cmd(0xAD);  // Disable keyboard
    ps2_write_cmd(0xA7);  // Disable mouse
    
    // Flush output buffer to clear any pending data
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
        inb(PS2_DATA_PORT);
    }
    
    // Set controller configuration
    ps2_write_cmd(0x20);  // Read config byte
    uint8_t config = ps2_read_data();
    
    // Enable keyboard interrupts (Bit 0) and Enable translation (Bit 6)
    // Translation converts Set 2 scancodes to Set 1 for the driver.
    config |= 0x41;  
    
    ps2_write_cmd(0x60);  // Write config byte
    ps2_write_data(config);
    
    // Enable keyboard
    ps2_write_cmd(0xAE);
}

uint8_t ps2_read_data(void) {
    if (wait_for_read() < 0) return 0;
    return inb(PS2_DATA_PORT);
}
