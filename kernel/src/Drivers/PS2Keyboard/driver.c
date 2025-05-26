#include <KiSimple.h>
#include <PMM/pmm.h>
#include <IDT/idt.h>

char kbd_drvr_buf[16 * 4096];
uint16_t kbd_drvr_buf_idx = 0;

static void OverflowKbdBfr() {
    kbd_drvr_buf_idx = 0;
}

void InitKeyboardDriver() {
    kbd_drvr_buf_idx = 0;
}

static void KbdPushback(char c) {
    if (kbd_drvr_buf_idx < sizeof(kbd_drvr_buf)) {
        kbd_drvr_buf[kbd_drvr_buf_idx++] = c;
        printk("%c", c);
    } else {
        OverflowKbdBfr();
        KbdPushback(c);
    }
}

static void KbdNAFPushback(char c) {
    if (kbd_drvr_buf_idx < sizeof(kbd_drvr_buf)) {
        kbd_drvr_buf[kbd_drvr_buf_idx++] = c;
    } else {
        OverflowKbdBfr();
        KbdNAFPushback(c);
    }
}

uint8_t __global_keyboard_autoflush = 0;

void KbdFlushCheck(char chr) {
    if (__global_keyboard_autoflush == 0) {
        KbdPushback(chr);
    } else {
        KbdNAFPushback(chr);
    }
}

bool IsCaps = false;
bool IsShift = false;

char USLayoutNrml[128] = {
    0, '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

char USLayoutCaps[128] = {
    0, '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 0, 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

char USLayoutShft[128] = {
    0, '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '`', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void KeyboardDriverMain(uint8_t scancode) {
    static char ch = 0;

    if (scancode & 0x80) {
        // Released keys
        switch (scancode) {
            case 0xAA: // Left Shift release
            case 0xB6: // Right Shift release
                IsShift = false;
                break;
            case 0x3A: // Caps Lock release (toggle only on release is fine)
                IsCaps = !IsCaps;
                break;
            default:
                break;
        }
    } else {
        // Pressed keys
        switch (scancode) {
            case 0x2A: // Left Shift press
            case 0x36: // Right Shift press
                IsShift = true;
                return;
            default:
                break;
        }

        if (IsShift) {
            ch = USLayoutShft[scancode];
        } else {
            ch = IsCaps ? USLayoutCaps[scancode] : USLayoutNrml[scancode];
        }

        if (ch) {
            KbdFlushCheck(ch);
            ch = 0;
        }
    }
}
