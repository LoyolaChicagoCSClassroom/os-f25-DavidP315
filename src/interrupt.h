#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>

/* PIC (Programmable Interrupt Controller) constants */
#define PIC1        0x20
#define PIC2        0xA0
#define PIC_1_COMMAND   PIC1
#define PIC_2_COMMAND   PIC2
#define PIC_1_DATA      (PIC1 + 1)
#define PIC_2_DATA      (PIC2 + 1)
#define PIC_EOI         0x20

/* IDT structures */
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* GDT / TSS definitions (you already have these) */

/* Function prototypes */
void remap_pic(void);
void load_gdt(void);
void init_idt(void);
void IRQ_clear_mask(unsigned char IRQline);
void IRQ_set_mask(unsigned char IRQline);
void PIC_sendEOI(unsigned char irq);

#endif
