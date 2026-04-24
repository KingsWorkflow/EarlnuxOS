// keyboard.c - Keyboard input handling

#include <stdint.h>

extern void print_char(char c, uint8_t color);

/* Scancode to ASCII lookup table (US layout, unshifted) */
static const char scancode_table[] = {
    0,    0,   '1', '2', '3', '4', '5', '6',
    '7',  '8', '9', '0', '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o',  'p', '[', ']', '\n', 0,  'a', 's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b',  'n', 'm', ',', '.', '/', 0,   '*',
    0,    ' ', 0,   0,   0,   0,   0,   0
};

void keyboard_handler(uint32_t scancode) {
    if (scancode < 58) {  // Valid scancode range
        char c = scancode_table[scancode];
        if (c != 0) {
            print_char(c, 0x0F);  // Print character
        }
    }
}