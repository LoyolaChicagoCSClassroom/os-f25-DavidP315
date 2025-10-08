#include <stdint.h>
#include "interrupt.h"

#define W 80
#define H 25

struct terminal_buffer {
    char ascii_value;
    char color;
};

int col = 0, row = 0;

void clear_line() {
    struct terminal_buffer *vram = (struct terminal_buffer*)0xB8000;
    for (int i = 0; i < W; i++) {
        vram[(H - 1) * W + i].ascii_value = ' ';
        vram[(H - 1) * W + i].color = 7;
    }
}
void scroll() {
    struct terminal_buffer *vram = (struct terminal_buffer*)0xB8000;
    for (int i = 1; i < H; i++)
        for (int j = 0; j < W; j++)
            vram[(i - 1) * W + j] = vram[i * W + j];
    clear_line();
}
void putc(char data) {
    struct terminal_buffer *vram = (struct terminal_buffer*)0xB8000;
    if (data == '\n') { col = 0; row++; }
    if (col >= W) { col = 0; row++; }
    if (row >= H) { scroll(); row = H - 1; }
    vram[row * W + col].ascii_value = data;
    vram[row * W + col].color = 7;
    col++;
}

void main(void) {
    remap_pic();
    load_gdt();
    init_idt();
    asm("sti");           

    for (int i = 0; i < H; i++) scroll();   
    while (1);            
}
