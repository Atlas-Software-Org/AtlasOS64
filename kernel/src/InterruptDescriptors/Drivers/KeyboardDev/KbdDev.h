#ifndef KBDDEV_H
#define KBDDEV_H 1

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

#define KbdAttrCtrl     0b00000001
#define KbdAttrShift    0b00000010
#define KbdAttrAlt      0b00000100
#define KbdAttrCaps     0b00001000

bool IsAttrTrue(uint8_t attr);

bool IsCtrlSet();
bool IsShiftSet();
bool IsAltSet();
bool IsCapsSet();

#endif /* KBDDEV_H */