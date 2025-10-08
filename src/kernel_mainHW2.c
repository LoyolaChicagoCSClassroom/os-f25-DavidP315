#include <stdint.h>
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC  0xe85250d6

const unsigned int multiboot_header[] __attribute__((section(".multiboot"))) = {
    MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16 + MULTIBOOT2_HEADER_MAGIC), 0, 12
};

uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "dN"(_port));
    return rv;
}

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define VGA_MEMORY    0xB8000
#define DEFAULT_COLOR 7

struct termbuf {
    char ascii;
    char color;
};

static int x = 0;  
static int y = 0;

void scroll_up(void) {
    struct termbuf *vram = (struct termbuf*)VGA_MEMORY;

    for (int row = 1; row < SCREEN_HEIGHT; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            vram[(row - 1) * SCREEN_WIDTH + col] = vram[row * SCREEN_WIDTH + col];
        }
    }

    for (int col = 0; col < SCREEN_WIDTH; col++) {
        vram[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col].ascii = ' ';
        vram[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col].color = DEFAULT_COLOR;
    }
}

int kputc(int c) {
    struct termbuf *vram = (struct termbuf*)VGA_MEMORY;

    if (c == '\n') {
        x = 0;
        y++;
    } else if (c == '\r') {
        x = 0;
    } else {
        vram[y * SCREEN_WIDTH + x].ascii = (char)c;
        vram[y * SCREEN_WIDTH + x].color = DEFAULT_COLOR;
        x++;
    }

    if (x >= SCREEN_WIDTH) {
        x = 0;
        y++;
    }

    if (y >= SCREEN_HEIGHT) {
        scroll_up();
        y = SCREEN_HEIGHT - 1;
    }

    return c;
}

unsigned char keyboard_map[128] = {
   0, 27, '1','2','3','4','5','6','7','8',
   '9','0','-','=', '\b',
   '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
   0, 'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
   'z','x','c','v','b','n','m',',','.','/', 0, 0, 0, ' ',
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void main() {
    esp_printf(kputc, "Current execution level: Kernel mode (Ring 0)\r\n");

    for (int i = 1; i <= 30; i++) {
        esp_printf(kputc, "Line %d: The quick brown fox jumps over the lazy dog.\r\n", i);
    }

    esp_printf(kputc, "Decimal: %d  Hex: %x  Char: %c  String: %s\r\n",
               12345, 0xBEEF, 'A', "Hello esp_printf!");
    esp_printf(kputc, "\n");

    while (1) {
        uint8_t status = inb(0x64);

        if (status & 1) {
            uint8_t scancode = inb(0x60);

            if (scancode >= 128)
                continue;

            esp_printf(kputc, "%x=%c | ", scancode, keyboard_map[scancode]);
        }
    }
}
