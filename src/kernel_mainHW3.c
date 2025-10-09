#include <stdint.h>
#include <stddef.h>
#include "rprintf.h"
#include "page.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

#define width 80
#define height 25
#define default_color 0x07

typedef struct termbuf{
   unsigned char ASCII;
   unsigned char COLOR;
} termbuf;

#define video_memory ((volatile termbuf*)0xB8000) 

static int row=0;
static int column=0;

static void scroll(void);
static void newLine(void);

int putc(int ch) {
    if (ch == '\r') { 
      column = 0; return ch; 
    }
    if (ch == '\n') { 
      newLine();
      return ch;
    }

    video_memory[row * width + column].ASCII = (unsigned char)ch;
    video_memory[row * width + column].COLOR = default_color;

    if (++column >= width) newLine();
    return ch;
}


static void scroll(void) {
    for (int r = 1; r < height; r++) {
        for (int c = 0; c < width; c++) {
            video_memory[(r - 1) * width + c] = video_memory[r * width + c];
        }
    }
    for (int c = 0; c < width; c++) {
        video_memory[(height - 1) * width + c].ASCII = ' ';
        video_memory[(height - 1) * width + c].COLOR = default_color;
    }
}

static void newLine(void) {
    column = 0;
    row++;
    if (row >= height) {
        scroll();
        row = height - 1;
    }
}


int vga_putc(int c) { return putc(c); }

unsigned char keyboard_map[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
 '9', '0', '-', '=', '\b',     /* Backspace */
 '\t',                 /* Tab */
 'q', 'w', 'e', 'r',   /* 19 */
 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
   0,                  /* 29   - Control */
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
'\'', '`',   0,                /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
 'm', ',', '.', '/',   0,                              /* Right shift */
 '*',
   0,  /* Alt */
 ' ',  /* Space bar */
   0,  /* Caps lock */
   0,  /* 59 - F1 key ... > */
   0,   0,   0,   0,   0,   0,   0,   0,  
   0,  /* < ... F10 */
   0,  /* 69 - Num lock*/
   0,  /* Scroll Lock */
   0,  /* Home key */
   0,  /* Up Arrow */
   0,  /* Page Up */
 '-',
   0,  /* Left Arrow */
   0,  
   0,  /* Right Arrow */
 '+',
   0,  /* 79 - End key*/
   0,  /* Down Arrow */
   0,  /* Page Down */
   0,  /* Insert Key */
   0,  /* Delete Key */
   0,   0,   0,  
   0,  /* F11 Key */
   0,  /* F12 Key */
   0,  /* All other keys are undefined */
};

void main(){


init_pfa_list();
   esp_printf(vga_putc, "Page Frame Allocator Initialized!\n");
   print_pfa_state();

   while(1) {
    uint8_t status = inb(0x64);
 

     if(status & 1) { 
        uint8_t scancode = inb(0x60);

       if(scancode < 128) {
            esp_printf(putc, "0x%02x    %c\n", scancode, keyboard_map[scancode]);
       }
     }
   }
} 
