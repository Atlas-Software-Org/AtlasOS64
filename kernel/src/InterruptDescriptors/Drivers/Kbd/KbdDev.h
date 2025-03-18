#pragma once

#include <mem/mem.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <gpx1.h>
#include <IDT/idt.h>

struct InterruptFrame;
__attribute__((interrupt)) void KeyboardInt_Hndlr(struct InterruptFrame* frame);