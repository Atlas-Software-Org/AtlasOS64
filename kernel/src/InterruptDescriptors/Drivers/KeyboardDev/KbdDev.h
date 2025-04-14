#pragma once

#include <InterruptDescriptors/Drivers/PIT/PIT.h>
#include <mem/mem.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <gpx1.h>
#include <IDT/idt.h>
#include <TTY/AtsTty.h>

struct InterruptFrame;
__attribute__((interrupt)) void KeyboardInt_Hndlr(struct InterruptFrame* frame);


int __keyboard_getc();
int __keyboard_gets(char *out, int maxlen);