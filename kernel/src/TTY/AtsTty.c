#include "AtsTty.h"

uint64_t cell_x = 0;
uint64_t cell_y = 0;

uint64_t max_cell_x = 0;
uint64_t max_cell_y = 0;

uint32_t color = 0xFFFFFFFF; // Global color

void tty_init() {
    cell_x = cell_y = 0;
    max_cell_x = GetFb()->width / 8 - 1; // maximum amount of TTY cells on the x-axis is the width of the framebuffer divided by the width of the font-char (8)
    max_cell_y = GetFb()->height / 16 - 1; // maximum amount of TTY cells on the y-axis is the height of the framebuffer divided by the height of the font-char (16)
}

void tty_putchar(char c) {
    if (c == '\n') { // Handle newline
        cell_x = 0;
        cell_y++;
    } else if (c == '\r') { // Handle carriage return
        cell_x = 0;
    } else if (c == '\b') { // Handle backspace
        if (cell_x > 0) {
            cell_x--;
        } else if (cell_y > 0) { // If we're at the beginning of the line, move up
            cell_y--;
            cell_x = max_cell_x;
        }
        DrawRect(cell_x*8, cell_y*16, 8, 16, 0x000000);
    } else { // Normal character
        if (cell_x > max_cell_x) {
            cell_x = 0;
            cell_y++;
        }

        if (cell_y > max_cell_y) {
            DrawRect(0, 0, GetFb()->width, GetFb()->height, 0x000000);
            cell_x = 0;
            cell_y = 0;
            return;
        }

        DrawRect(cell_x*8, cell_y*16, 8, 16, 0x000000);
        FontPutChar(c, cell_x * 8, cell_y * 16, color);
        cell_x++;
    }
}

#include <HtKernelUtils/debug.h>

void tty_puts(const char *s) {
    while (*s != 0) {
        if (*s == '\e') { // Color escape sequence start
            s++;
            if (*s == '[') {  // Begin color code
                s++;
                uint8_t new_color = *s - '0'; // Parse color number (assuming single-digit)
                s++;
                if (*s == ']') {  // End of color code
                    s++;
                    if (*s == ';') {  // Color definition end
                        s++;
                        color = ParseAtes(new_color);  // Update global color
                    }
                }
            }
        }
        if (*s != 0) {
            tty_putchar(*s); // Print the character with the current color
        }
        s++;
    }
}

void tty_set_cursor(uint64_t x, uint64_t y) {
    if (0 <= x && x <= max_cell_x) {
        cell_x = x;
    } else {
        cell_x = cell_x; // Retain current value if out of bounds
    }
    if (0 <= y && y <= max_cell_y) {
        cell_y = y;
    } else {
        cell_y = cell_y; // Retain current value if out of bounds
    }
}

uint64_t tty_get_dimensions() {
    return (max_cell_x << 32) | (max_cell_y & 0xFFFFFFFF);
}

uint64_t getCellx() {
    return cell_x;
}

uint64_t getCelly() {
    return cell_y;
}