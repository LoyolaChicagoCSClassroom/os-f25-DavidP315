#include <stdint.h>
#include "interrupt.h"

/* Low-level port I/O helpers */
void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "dN"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ __volatile__("inb %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

/* Global IDT table and pointer */
static struct idt_entry idt_entries[256];
static struct idt_ptr   idt_ptr;

/* Minimal memset */
void memset(char *s, char c, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) s[i] = c;
}

/* Forward declare our handler so we can reference it */
__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame* frame);

/* Keyboard handler (IRQ1) */
__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame* frame)
{
    uint8_t scancode = inb(0x60);

    if (!(scancode & 0x80)) {              // Key press only
        extern void putc(char);
        static const char map[128] = {
            0, 27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
            '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
            'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
            'z','x','c','v','b','n','m',',','.','/',0,0,0,' ',
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };
        char c = map[scancode];
        if (c) putc(c);
    }

    outb(0x20, 0x20);   // Send End-of-Interrupt (EOI)
}

/* Initialize IDT and map keyboard interrupt */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;
    memset((char*)&idt_entries, 0, sizeof(idt_entries));

    /* Set keyboard interrupt (IRQ1 = 0x21) */
    idt_set_gate(0x21, (uint32_t)keyboard_handler, 0x08, 0x8E);

    asm volatile("lidt %0" : : "m"(idt_ptr));
}

/* Remap and enable PIC for keyboard */
void remap_pic(void) {
    outb(PIC_1_COMMAND, 0x11);
    outb(PIC_2_COMMAND, 0x11);
    outb(PIC_1_DATA, 0x20);
    outb(PIC_2_DATA, 0x28);
    outb(PIC_1_DATA, 0x00);
    outb(PIC_2_DATA, 0x00);
    outb(PIC_1_DATA, 0x01);
    outb(PIC_2_DATA, 0x01);

    /* Mask all except keyboard (IRQ1) */
    outb(PIC_1_DATA, 0xFD);
    outb(PIC_2_DATA, 0xFF);
}


