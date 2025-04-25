#include "KbdDev.h"

#define BUFFER_SIZE 1024

static char kbd_buffer[BUFFER_SIZE];
static volatile int kbd_head = 0;
static volatile int kbd_tail = 0;
static volatile bool kbd_data_ready = false;

bool IsShift = false;
bool IsCaps = false;

bool IsCtrl = false;
bool IsAlt = false;

char USLayoutNrml[128] = {
    '\e','`','1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\','z',
    'x','c','v','b','n','m',',','.','/', 0,'*', 0,' ', 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'-',
    0,0,0,'+',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char USLayoutShft[128] = {
    '\e','~','!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z',
    'X','C','V','B','N','M','<','>','?',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'-',
    0,0,0,'+',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char USLayoutCaps[128] = {
    '\e','`','1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','[',']','\n',0,
    'A','S','D','F','G','H','J','K','L',';','\'','`',0,'\\','Z',
    'X','C','V','B','N','M',',','.','/',0,'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'-',
    0,0,0,'+',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void kbd_buffer_push(char c) {
    int next = (kbd_head + 1) % BUFFER_SIZE;
    if (next == kbd_tail) {
        kbd_tail = (kbd_tail + 1) % BUFFER_SIZE;
    }
    kbd_buffer[kbd_head] = c;
    kbd_head = next;
    kbd_data_ready = true;
    if ((0x20 <= c && c <= 0x7E) || c == '\e' || c == '\b' || c == '\n' || c == '\r') {
        goto end;
        uint64_t cellx = getCellx();
        uint64_t celly = getCelly();
        int xoff_tty_get = getXoffParam();
        int yoff_tty_get = getYoffParam();
        DrawRect((cellx+xoff_tty_get)*8, (celly+yoff_tty_get)*16, 8, 16, 0x000000);
        tty_putchar_simple(c);
        cellx = getCellx();
        celly = getCelly();
        DrawRect((cellx+xoff_tty_get)*8+1, (celly+yoff_tty_get)*16+1, 1, 14, 0xFFFFFFFF);
    }
    end:
}

int kbd_buffer_pop(void) {
    if (kbd_head == kbd_tail) {
        return -1;
    }
    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % BUFFER_SIZE;
    if (kbd_head == kbd_tail) {
        kbd_data_ready = false;
    }
    return (int)c;
}

int __keyboard_getc(void) {
    int c;
    while ((c = kbd_buffer_pop()) < 0) {
        __asm__ volatile("hlt");
    }
    tty_putchar(c);
    return c;
}

int __keyboard_gets(char *out, int maxlen) {
    if (maxlen <= 1) {
        return 0;
    }
    int count = 0;
    while (count < maxlen - 1) {
        int ci = __keyboard_getc();
        if (ci == '\r') {
            continue;
        }
        if (ci == '\n' || ci == '\b') {
            if (ci == '\b' && count > 0) {
                count--;
                //tty_putchar('\b');
                continue;
            }
            //tty_putchar('\n');
            break;
        }
        out[count++] = (char)ci;
        //tty_putchar((char)ci);
    }
    out[count] = '\0';
    return count;
}

extern int NotificationRecievedKbdInterrut;

__attribute__((interrupt)) void KeyboardInt_Hndlr(struct InterruptFrame* Frame) {
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) {
        switch (scancode) {
            case 0xAA:
            case 0xB6:
                IsShift = false;
                break;
            default:
                PIC_sendEOI(1);
                IOWait();
                return;
        }
    } else {
        switch (scancode) {
            case 0x2A:
            case 0x36:
                IsShift = true;
                PIC_sendEOI(1);
                IOWait();
                return;
            case 0x3A:
                IsCaps = !IsCaps;
                PIC_sendEOI(1);
                IOWait();
                return;
            case 0x1D:
                IsCtrl = true;
                PIC_sendEOI(1);
                IOWait();
                break;
            case 0x9D:
                IsCtrl = false;
                PIC_sendEOI(1);
                IOWait();
                break;
            case 0x38:
                IsAlt = true;
                PIC_sendEOI(1);
                IOWait();
                break;
            case 0xB8:
                IsAlt = false;
                PIC_sendEOI(1);
                IOWait();
                break;
            default:
                break;
        }
    }
    char ch = 0;
    if (!(scancode & 0x80)) {
        if (IsShift && IsCaps) {
            ch = USLayoutShft[scancode];
        } else if (IsShift) {
            ch = USLayoutShft[scancode];
        } else if (IsCaps) {
            ch = USLayoutCaps[scancode];
        } else {
            ch = USLayoutNrml[scancode];
        }
        if (ch) {
            NotificationRecievedKbdInterrut = 1;
            kbd_buffer_push(ch);
        }
    }
    PIC_sendEOI(1);
    IOWait();
}

bool IsAttrTrue(uint8_t attr) {
    const uint8_t kbd_ctrl_attr_mask = 0b00000001;
    const uint8_t kbd_shft_attr_mask = 0b00000010;
    const uint8_t kbd_alt_attr_mask = 0b00000100;
    const uint8_t kbd_caps_attr_mask = 0b00001000;

    uint8_t _attr = attr;

    if (_attr & kbd_ctrl_attr_mask) {
        return IsCtrl;
    }

    if (_attr & kbd_shft_attr_mask) {
        return IsShift;
    }

    if (_attr & kbd_alt_attr_mask) {
        return IsAlt;
    }

    if (_attr & kbd_caps_attr_mask) {
        return IsCaps;
    }

    return false;  // If no attribute matches, return false
}

bool IsCtrlSet() {return IsCtrl;}
bool IsShiftSet() {return IsShift;}
bool IsAltSet() {return IsAlt;}
bool IsCapsSet() {return IsCaps;}
