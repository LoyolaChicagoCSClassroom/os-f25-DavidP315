#include <stdint.h>
#include "rprint.h" 

// Multiboot header for the GRUB bootloader
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
        };

uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

// variables for the terminal driver
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
static unsigned short *vga_buffer = (unsigned short*)0xB8000;
static unsigned int cursor_pos = 0; 

// Scrolls the line up
static void scroll() {
    // discards the top line.
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; ++i) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    
    // Clearing the last line
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; ++i) {
        vga_buffer[i] = (7 << 8) | ' '; 
    }
}


void putc(int data) {
    if (data == '\n') { 
        // beginning of the next line
        cursor_pos = cursor_pos + (VGA_WIDTH - (cursor_pos % VGA_WIDTH));
    } else if (data == '\r') { 
        // go to beginning of the current line
        cursor_pos = cursor_pos - (cursor_pos % VGA_WIDTH);
    } else if (data == '\b') { 
        // go back one position and clear the character
        if (cursor_pos > 0) {
            cursor_pos--;
            vga_buffer[cursor_pos] = (7 << 8) | ' ';
        }
    } else {
        unsigned short char_with_color = (7 << 8) | (data & 0xFF);
        vga_buffer[cursor_pos] = char_with_color;
        cursor_pos++;
    }

    // scroll if at end of screen
    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT) {
        scroll(); 
        cursor_pos = (VGA_HEIGHT - 1) * VGA_WIDTH; // move cursor to the start of the new last line
    }
}

void main() {

    // esp_printf to print the current execution level,
    esp_printf(putc, "Execution Level: Ring 0\r\n");
    esp_printf(putc, "Welcome to the kernel terminal!\r\n");


    for(int i = 0; i < 30; ++i) {
        esp_printf(putc, "Test line %d: Hex is 0x%x\r\n", i, i * 1234);
    }

    while(1) {
        uint8_t status = inb(0x64);
        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
