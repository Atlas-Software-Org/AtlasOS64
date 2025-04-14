#pragma once

#include <gpx1.h>

void tty_init();
void tty_putchar(char c);
void tty_puts(const char *s);
void tty_set_cursor(uint64_t x, uint64_t y);
uint64_t tty_get_dimensions();

uint64_t getCellx();
uint64_t getCelly();