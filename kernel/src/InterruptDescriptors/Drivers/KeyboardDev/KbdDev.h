#pragma once

#include <InterruptDescriptors/Drivers/PIT/PIT.h>
#include <mem/mem.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <gpx1.h>
#include <IDT/idt.h>

struct InterruptFrame;
__attribute__((interrupt)) void KeyboardInt_Hndlr(struct InterruptFrame* frame);

extern const uint64_t KbdXIdxStart;
extern const uint64_t KbdYIdxStart;

extern uint64_t KbdXIdx;
extern uint64_t KbdYIdx;

void InitKbd();