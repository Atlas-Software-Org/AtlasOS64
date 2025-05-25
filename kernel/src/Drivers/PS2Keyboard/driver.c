#include <KiSimple.h>
#include <PMM/pmm.h>
#include <IDT/idt.h>

char *kbd_drvr_buf;
uint16_t kbd_drvr_buf_idx = 0;

void InitKeyboardDriver() {
    kbd_drvr_buf = (char*)KiPmmNAlloc(16);
    kbd_drvr_buf_idx = 0;
}

void DeinitKeyboardDriver() {
    KiPmmNFree(kbd_drvr_buf, 16);
}

static void OverflowKbdBfr() {
    for (int i = 0; i < 65535; i++) {
        kbd_drvr_buf[i] = 0;
    }
    kbd_drvr_buf_idx = 0;
}

static void KbdPushback(char c) {
    if (kbd_drvr_buf_idx < 65535) {
        kbd_drvr_buf[kbd_drvr_buf_idx++] = c;
        printk("%c", c);
    } else {
        OverflowKbdBfr();
        KbdPushback(c); /* identity-call */
        return;
    }
}

static void KbdNAFPushback(char c) {
    if (kbd_drvr_buf_idx < 65535) {
        kbd_drvr_buf[kbd_drvr_buf_idx++] = c;
    } else {
        OverflowKbdBfr();
        KbdNAFPushback(c); /* identity-call */
        return;
    }
}

const char KeyboardScancodeLookupTable[] = {
    0,      // 0x00
    '\x1B', // 0x01 - ESC
    '1',    // 0x02
    '2',    // 0x03
    '3',    // 0x04
    '4',    // 0x05
    '5',    // 0x06
    '6',    // 0x07
    '7',    // 0x08
    '8',    // 0x09
    '9',    // 0x0A
    '0',    // 0x0B
    '-',    // 0x0C
    '=',    // 0x0D
    '\b',   // 0x0E - Backspace
    '\t',   // 0x0F - Tab
    'q',    // 0x10
    'w',    // 0x11
    'e',    // 0x12
    'r',    // 0x13
    't',    // 0x14
    'y',    // 0x15
    'u',    // 0x16
    'i',    // 0x17
    'o',    // 0x18
    'p',    // 0x19
    '[',    // 0x1A
    ']',    // 0x1B
    '\n',   // 0x1C - Enter
    0,      // 0x1D - Left Ctrl
    'a',    // 0x1E
    's',    // 0x1F
    'd',    // 0x20
    'f',    // 0x21
    'g',    // 0x22
    'h',    // 0x23
    'j',    // 0x24
    'k',    // 0x25
    'l',    // 0x26
    ';',    // 0x27
    '\'',   // 0x28
    '`',    // 0x29
    0,      // 0x2A - Left Shift
    '\\',   // 0x2B
    'z',    // 0x2C
    'x',    // 0x2D
    'c',    // 0x2E
    'v',    // 0x2F
    'b',    // 0x30
    'n',    // 0x31
    'm',    // 0x32
    ',',    // 0x33
    '.',    // 0x34
    '/',    // 0x35
    0,      // 0x36 - Right Shift
    '*',    // 0x37 - Keypad *
    0,      // 0x38 - Left Alt
    ' ',    // 0x39 - Space
    0,      // 0x3A - Caps Lock
    0,      // 0x3B - F1
    0,      // 0x3C - F2
    0,      // 0x3D - F3
    0,      // 0x3E - F4
    0,      // 0x3F - F5
    0,      // 0x40 - F6
    0,      // 0x41 - F7
    0,      // 0x42 - F8
    0,      // 0x43 - F9
    0,      // 0x44 - F10
    0,      // 0x45 - Num Lock
    0,      // 0x46 - Scroll Lock
    '7',    // 0x47 - Keypad 7 (Home)
    '8',    // 0x48 - Keypad 8 (Up)
    '9',    // 0x49 - Keypad 9 (PgUp)
    '-',    // 0x4A - Keypad -
    '4',    // 0x4B - Keypad 4 (Left)
    '5',    // 0x4C - Keypad 5
    '6',    // 0x4D - Keypad 6 (Right)
    '+',    // 0x4E - Keypad +
    '1',    // 0x4F - Keypad 1 (End)
    '2',    // 0x50 - Keypad 2 (Down)
    '3',    // 0x51 - Keypad 3 (PgDn)
    '0',    // 0x52 - Keypad 0 (Ins)
    '.',    // 0x53 - Keypad . (Del)
    0,      // 0x54 - SysReq
    0,      // 0x55 - not used
    0,      // 0x56 - maybe OEM
    0,      // 0x57 - F11
    0,      // 0x58 - F12
    0,      // 0x00                                     Start of caps
    '\x1B', // 0x01 - ESC
    '1',    // 0x02
    '2',    // 0x03
    '3',    // 0x04
    '4',    // 0x05
    '5',    // 0x06
    '6',    // 0x07
    '7',    // 0x08
    '8',    // 0x09
    '9',    // 0x0A
    '0',    // 0x0B
    '-',    // 0x0C
    '=',    // 0x0D
    '\b',   // 0x0E - Backspace
    '\t',   // 0x0F - Tab
    'Q',    // 0x10
    'W',    // 0x11
    'E',    // 0x12
    'R',    // 0x13
    'T',    // 0x14
    'Y',    // 0x15
    'U',    // 0x16
    'I',    // 0x17
    'O',    // 0x18
    'P',    // 0x19
    '[',    // 0x1A
    ']',    // 0x1B
    '\n',   // 0x1C - Enter
    0,      // 0x1D - Left Ctrl
    'A',    // 0x1E
    'S',    // 0x1F
    'D',    // 0x20
    'F',    // 0x21
    'G',    // 0x22
    'H',    // 0x23
    'J',    // 0x24
    'K',    // 0x25
    'L',    // 0x26
    ';',    // 0x27
    '\'',   // 0x28
    '`',    // 0x29
    0,      // 0x2A - Left Shift
    '\\',   // 0x2B
    'Z',    // 0x2C
    'X',    // 0x2D
    'C',    // 0x2E
    'V',    // 0x2F
    'B',    // 0x30
    'N',    // 0x31
    'M',    // 0x32
    ',',    // 0x33
    '.',    // 0x34
    '/',    // 0x35
    0,      // 0x36 - Right Shift
    '*',    // 0x37 - Keypad *
    0,      // 0x38 - Left Alt
    ' ',    // 0x39 - Space
    0,      // 0x3A - Caps Lock
    0,      // 0x3B - F1
    0,      // 0x3C - F2
    0,      // 0x3D - F3
    0,      // 0x3E - F4
    0,      // 0x3F - F5
    0,      // 0x40 - F6
    0,      // 0x41 - F7
    0,      // 0x42 - F8
    0,      // 0x43 - F9
    0,      // 0x44 - F10
    0,      // 0x45 - Num Lock
    0,      // 0x46 - Scroll Lock
    '7',    // 0x47 - Keypad 7 (Home)
    '8',    // 0x48 - Keypad 8 (Up)
    '9',    // 0x49 - Keypad 9 (PgUp)
    '-',    // 0x4A - Keypad -
    '4',    // 0x4B - Keypad 4 (Left)
    '5',    // 0x4C - Keypad 5
    '6',    // 0x4D - Keypad 6 (Right)
    '+',    // 0x4E - Keypad +
    '1',    // 0x4F - Keypad 1 (End)
    '2',    // 0x50 - Keypad 2 (Down)
    '3',    // 0x51 - Keypad 3 (PgDn)
    '0',    // 0x52 - Keypad 0 (Ins)
    '.',    // 0x53 - Keypad . (Del)
    0,      // 0x54 - SysReq
    0,      // 0x55 - not used
    0,      // 0x56 - maybe OEM
    0,      // 0x57 - F11
    0,      // 0x58 - F12
    0,      // 0x00                                     Start of shift
    '\x1B', // 0x01 - ESC
    '!',    // 0x02
    '@',    // 0x03
    '#',    // 0x04
    '$',    // 0x05
    '%',    // 0x06
    '^',    // 0x07
    '&',    // 0x08
    '*',    // 0x09
    '(',    // 0x0A
    ')',    // 0x0B
    '_',    // 0x0C
    '+',    // 0x0D
    '\b',   // 0x0E - Backspace
    '\t',   // 0x0F - Tab
    'Q',    // 0x10
    'W',    // 0x11
    'E',    // 0x12
    'R',    // 0x13
    'T',    // 0x14
    'Y',    // 0x15
    'U',    // 0x16
    'I',    // 0x17
    'O',    // 0x18
    'P',    // 0x19
    '{',    // 0x1A
    '}',    // 0x1B
    '\n',   // 0x1C - Enter
    0,      // 0x1D - Left Ctrl
    'A',    // 0x1E
    'S',    // 0x1F
    'D',    // 0x20
    'F',    // 0x21
    'G',    // 0x22
    'H',    // 0x23
    'J',    // 0x24
    'K',    // 0x25
    'L',    // 0x26
    ':',    // 0x27
    '"',    // 0x28
    '~',    // 0x29
    0,      // 0x2A - Left Shift
    '|',    // 0x2B
    'Z',    // 0x2C
    'X',    // 0x2D
    'C',    // 0x2E
    'V',    // 0x2F
    'B',    // 0x30
    'N',    // 0x31
    'M',    // 0x32
    '<',    // 0x33
    '>',    // 0x34
    '?',    // 0x35
    0,      // 0x36 - Right Shift
    '*',    // 0x37 - Keypad *
    0,      // 0x38 - Left Alt
    ' ',    // 0x39 - Space
    0,      // 0x3A - Caps Lock
    0,      // 0x3B - F1
    0,      // 0x3C - F2
    0,      // 0x3D - F3
    0,      // 0x3E - F4
    0,      // 0x3F - F5
    0,      // 0x40 - F6
    0,      // 0x41 - F7
    0,      // 0x42 - F8
    0,      // 0x43 - F9
    0,      // 0x44 - F10
    0,      // 0x45 - Num Lock
    0,      // 0x46 - Scroll Lock
    '7',    // 0x47 - Keypad 7 (Home)
    'A',    // 0x48 - Keypad 8 (Up)
    '9',    // 0x49 - Keypad 9 (PgUp)
    '-',    // 0x4A - Keypad -
    'B',    // 0x4B - Keypad 4 (Left)
    '5',    // 0x4C - Keypad 5
    'C',    // 0x4D - Keypad 6 (Right)
    '+',    // 0x4E - Keypad +
    '1',    // 0x4F - Keypad 1 (End)
    'D',    // 0x50 - Keypad 2 (Down)
    '3',    // 0x51 - Keypad 3 (PgDn)
    '0',    // 0x52 - Keypad 0 (Ins)
    '.',    // 0x53 - Keypad . (Del)
    0,      // 0x54 - SysReq
    0,      // 0x55 - not used
    0,      // 0x56 - maybe OEM
    0,      // 0x57 - F11
    0       // 0x58 - F12
};

uint8_t __global_keyboard_autoflush = 0; /* 0 for autoflush, 1 for disable autoflush */ /* autoflush means it will write the character as soon as its 
                                                                                        pressed, otherwise it will buffer it and not write it */

void TranslateKeyboard(char* lookup_table, uint8_t sc, int off) {
    if (off > 89*3) off = 0; /* handle overflow */
    char c = lookup_table[(sc & 0x80) + 0];

    switch (__global_keyboard_autoflush) {
        case 0:
            KbdPushback(c);
            break;
        default:
            KbdNAFPushback(c); /* Kbd NO AUTOFLUSH pushback */
            break;
    }
}

typedef struct {
    uint8_t shift:1;
    uint8_t caps:1;
    uint8_t reserved:6;
} _kbd_flags;

_kbd_flags kbd_flags = {
    0, 0, 0
};

void __attribute__((interrupt)) KeyboardDriverMain(int *__unused) {
    uint8_t sc = inb(0x60);
    outb(0xE9, 'K');
    if (!(sc & 0x80)) {
        switch (sc) {
            case 0x2A: /* LShift make */
            case 0x36: /* RShift make */
                kbd_flags.shift = 1;
                break;
            case 0xAA: /* LShift release */
            case 0xB6: /* RShift release */
                kbd_flags.shift = 0;
                break;
            case 0x3A: /* Capslock make */
                kbd_flags.caps = kbd_flags.caps == 1 ? 0 : 1;
                break;
            default:
                if (kbd_flags.caps == 1 && kbd_flags.shift == 0) {
                    TranslateKeyboard(KeyboardScancodeLookupTable, sc, 89);
                    break;
                }
                if (kbd_flags.shift == 1) {
                    TranslateKeyboard(KeyboardScancodeLookupTable, sc, 178);
                    break;
                }
        }
    }

    KiPicSendEoi(1);
}