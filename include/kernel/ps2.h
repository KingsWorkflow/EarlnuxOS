#ifndef _KERNEL_PS2_H
#define _KERNEL_PS2_H

#include <stdint.h>

// PS/2 controller ports
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

// PS/2 commands
#define PS2_CMD_ENABLE_KEYBOARD 0xAE
#define PS2_CMD_DISABLE_KEYBOARD 0xAD

// PS/2 functions
int ps2_write_cmd(uint8_t cmd);
int ps2_write_data(uint8_t data);
uint8_t ps2_read_data(void);
void ps2_init(void);

#endif