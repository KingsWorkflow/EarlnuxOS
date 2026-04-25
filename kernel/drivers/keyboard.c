#include <kernel/kernel.h>
#include <kernel/regs.h>
#include <kernel/pic.h>
#include <kernel/ps2.h>
#include <arch/x86/idt.h>
#include <stdint.h>

/* Forward declarations */
void keyboard_handle_scancode(uint8_t sc);

/* Keyboard buffer size */
#define KB_BUF_SIZE     128

/* Circular buffer for scancodes */
static volatile uint8_t kb_buffer[KB_BUF_SIZE];
static volatile uint8_t kb_head = 0;
static volatile uint8_t kb_tail = 0;

/* Modifier keys state */
static volatile bool shift_down = false;
static volatile bool ctrl_down = false;
static volatile bool alt_down = false;

/* Scancode to ASCII translation (US QWERTY, set 1 basic, set2? We'll use set1 style mapping for simplicity) */
static const char scancode_to_ascii[128] = {
    0,   0x1B,'1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
    'd','f','g','h','j','k','l',';',39,'`',0,'\\','z','x','c','v',
    'b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Shifted scancodes produce uppercase / symbols */
static const char scancode_shift[128] = {
    0,   0x1B,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S',
    'D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V',
    'B','N','M','<','>','?',0,'*',0,' ',0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Interrupt handler called by IDT */
void keyboard_handler(struct regs *r) {
    (void)r;
    uint8_t scancode = inb(0x60);
    keyboard_handle_scancode(scancode);
}

/* Initialize keyboard driver */
void keyboard_init(void) {
    /* Power on the hardware controller */
    extern void ps2_init(void);
    ps2_init();

    /* Remapped IRQ1 is the vector defined in idt.h */
    isr_register_handler(INT_IRQ1, keyboard_handler);
    
    /* Hardware IRQ line 1 for the PIC */
    pic_unmask_irq(1);
    
    /* Reset buffer state */
    kb_head = kb_tail = 0;
    shift_down = false;
    
    KINFO("keyboard", "PS/2 keyboard initialized\n");
}

/* Handle a scancode from hardware interrupt */
void keyboard_handle_scancode(uint8_t sc) {
    /* Handle release (bit7 set) */
    if (sc & 0x80) {
        uint8_t release = sc & 0x7F;
        /* Update modifier state */
        if (release == 0x2A || release == 0x36) shift_down = false;
        else if (release == 0x1D) ctrl_down = false;
        else if (release == 0x38) alt_down = false;
        return;
    }

    /* Make code: handle modifier keys */
    if (sc == 0x2A || sc == 0x36) { shift_down = true; return; }
    if (sc == 0x1D) { ctrl_down = true; return; }
    if (sc == 0x38) { alt_down = true; return; }

    /* Translate to ASCII */
    char c = 0;
    if (sc < 128) {
        c = shift_down ? scancode_shift[sc] : scancode_to_ascii[sc];
    }

    if (c != 0) {
        /* Store in circular buffer */
        uint8_t next = (kb_head + 1) % KB_BUF_SIZE;
        if (next != kb_tail) {
            kb_buffer[kb_head] = (uint8_t)c;
            kb_head = next;
        }
        /* Could also echo to console if desired */
    }
}

/* Get next character from keyboard buffer (blocking) */
char keyboard_getchar(void) {
    while (kb_tail == kb_head) {
        /* Wait for interrupt to fill buffer */
        __asm__ volatile("hlt");
    }
    char c = (char)kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

/* Check if buffer has data (non-blocking) */
bool keyboard_has_data(void) {
    return kb_tail != kb_head;
}
