#include "MouseDev.h"

#include <HtKernelUtils/debug.h>

Button_t Buttons[4096];
uint64_t last_btn = 0;

Button_t BTN_NULL = {
    .label = "NULL",
    .Handler = NULL,
    .Position = {0, 0},
    .Scale = {0, 0},
    .Enabled = 0
};

Button_t *CreateButton(const char* label, void (*Handler)(), uint64_t x, uint64_t y, uint64_t sx, uint64_t sy) {
    if (last_btn >= 4096) {
        return &BTN_NULL; // No more space for buttons
    }
    //Buttons[last_btn].label = label;
    Buttons[last_btn].Handler = Handler;
    Buttons[last_btn].Position.X = x;
    Buttons[last_btn].Position.Y = y;
    Buttons[last_btn].Scale.X = sx;
    Buttons[last_btn].Scale.Y = sy;
    Buttons[last_btn].Enabled = 0;
    Buttons[last_btn].idx = last_btn;
    return &Buttons[last_btn++];
}

int AddButton(Button_t* btn) {
    if (last_btn >= 4096) {
        return -1;
    }
    Buttons[last_btn] = *btn;
    Buttons[last_btn].idx = last_btn;
    last_btn++;
    return last_btn-1;
}

void RemoveButton(int btn_index) {
    if (btn_index < 0 || btn_index >= last_btn) return; // Ensure the index is valid
    // Move all the elements after the removed button back one position
    for (int i = btn_index; i < last_btn - 1; i++) {
        Buttons[i] = Buttons[i + 1];
    }
    last_btn--; // Decrease the last button counter
}

extern RootWindowHandle *RootWindowTree;
extern uint8_t *RootWindowTreeBitmap;
extern uint8_t GetBit(uint8_t* map, uint32_t bit_index);

int draggingWin = -1;
int dragOffsetX = 0;
int dragOffsetY = 0;

extern void e9debugkf(const char*,...);

void HandleWindowMovementMouse(uint64_t x, uint64_t y) {
    if (draggingWin == -1) {
        for (int j = 0; j < 1024; j++) {
            if (RootWindowTree->WinHandles[j].____exists__ != true) continue;

            int wx = RootWindowTree->WinHandles[j].winfb->dispx;
            int wy = RootWindowTree->WinHandles[j].winfb->dispy;
            int ww = RootWindowTree->WinHandles[j].winfb->width;

            if (x >= wx && x < wx + ww && y >= wy && y < wy + 22) {
                draggingWin = j;
                dragOffsetX = x - wx;
                dragOffsetY = y - wy;
                break;
            }
        }
    }

    if (draggingWin != -1) {
        RootWindowTree->WinHandles[draggingWin].winfb->dispx = x;
        RootWindowTree->WinHandles[draggingWin].winfb->dispy = y;

        RootWindowTree->WinHandles[draggingWin].Repaint(&RootWindowTree->WinHandles[draggingWin]);
    }
}

void CheckBtns(uint64_t x, uint64_t y) {
    for (int i = 0; i < last_btn; i++) { // Iterate only up to last_btn to avoid unnecessary checks
        HandleWindowMovementMouse(x, y);

        // Dereference Buttons[i] to get the button and check if the point (x, y) is inside the button
        if (x >= Buttons[i].Position.X &&
            x <= (Buttons[i].Position.X + Buttons[i].Scale.X) &&
            y >= Buttons[i].Position.Y &&
            y <= (Buttons[i].Position.Y + Buttons[i].Scale.Y)) {
            
            if (Buttons[i].Enabled == 0) {
                continue;
            }

            // If the point is inside the button, call its handler
            Buttons[i].Enabled = 0;
            Buttons[i].Handler();
            Buttons[i].Enabled = 1;
        }
    }
}

void SetBtnEnabled(Button_t btn) {
    btn.Enabled = 1;
}

void ClearBtnEnabled(Button_t btn) {
    btn.Enabled = 0;
}

uint8_t MousePointer[] = {
    0b10000000, 0b00000000, 
    0b11000000, 0b00000000, 
    0b11100000, 0b00000000, 
    0b11110000, 0b00000000, 
    0b11111000, 0b00000000, 
    0b11111100, 0b00000000, 
    0b11111110, 0b00000000, 
    0b11111111, 0b00000000, 
    0b11111111, 0b10000000, 
    0b11111111, 0b11000000, 
    0b11111100, 0b00000000, 
    0b11101100, 0b00000000, 
    0b11000110, 0b00000000, 
    0b10000110, 0b00000000, 
    0b00000011, 0b00000000, 
    0b00000011, 0b00000000, 
};

void MouseWait() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if ((inb(0x64) & 0b10) == 0) {
            return;
        }
    }
}

void MouseWaitInput() {
    uint64_t timeout = 100000;
    while (timeout--) {
        if (inb(0x64) & 0b1) {
            return;
        }
    }
}

void MouseWrite(uint8_t value) {
    MouseWait();
    outb(0x64, 0xD4);
    MouseWait();
    outb(0x60, value);
}

uint8_t MouseRead() {
    MouseWaitInput();
    return inb(0x60);
}

uint8_t MouseCycle = 0;
uint8_t MousePacket[4];
bool MousePacketReady = false;
Point MousePosition;
Point MousePositionOld;

void HandlePS2Mouse(uint8_t data) {
    switch(MouseCycle) {
        case 0:
            if (MousePacketReady) break;
            if ((data & 0b00001000) == 0) break;
            MousePacket[0] = data;
            MouseCycle++;
            break;
        case 1:
            if (MousePacketReady) break;
            MousePacket[1] = data;
            MouseCycle++;
            break;
        case 2:
            if (MousePacketReady) break;
            MousePacket[2] = data;
            MousePacketReady = true;
            MouseCycle = 0;
            break;
    }

    ProcessMousePacket();
}

#include "../KeyboardDev/KbdDev.h"

__attribute__((hot)) void ProcessMousePacket() {
    if (!MousePacketReady) return;

        bool xNegative, yNegative, xOverflow, yOverflow;

        if (MousePacket[0] & PS2XSign) {
            xNegative = true;
        }else xNegative = false;

        if (MousePacket[0] & PS2YSign) {
            yNegative = true;
        }else yNegative = false;

        if (MousePacket[0] & PS2XOverflow) {
            xOverflow = true;
        }else xOverflow = false;

        if (MousePacket[0] & PS2YOverflow) {
            yOverflow = true;
        }else yOverflow = false;

        if (!xNegative) {
            MousePosition.X += MousePacket[1];
            if (xOverflow) {
                MousePosition.X += 255;
            }
        } else
        {
            MousePacket[1] = 256 - MousePacket[1];
            MousePosition.X -= MousePacket[1];
            if (xOverflow) {
                MousePosition.X -= 255;
            }
        }

        if (!yNegative) {
            MousePosition.Y -= MousePacket[2];
            if (yOverflow) {
                MousePosition.Y -= 255;
            }
        } else
        {
            MousePacket[2] = 256 - MousePacket[2];
            MousePosition.Y += MousePacket[2];
            if (yOverflow) {
                MousePosition.Y += 255;
            }
        }

        if (MousePosition.X <= 1) MousePosition.X = 1;
        if (MousePosition.X >= GetFb()->width-1) MousePosition.X = GetFb()->width-1;
        
        if (MousePosition.Y <= 1) MousePosition.Y = 1;
        if (MousePosition.Y >= GetFb()->height-1) MousePosition.Y = GetFb()->height-1;
        
        ClearMouseCursor(MousePointer, MousePositionOld);
        DrawOverlayMouseCursor(MousePointer, MousePosition, 0xffffffff);

        if (MousePacket[0] & PS2Leftbutton) {
            CheckBtns(MousePosition.X, MousePosition.Y);
        } else {
            draggingWin = -1;
        }
        if (MousePacket[0] & PS2Middlebutton) {

        }
        if (MousePacket[0] & PS2Rightbutton) {

        }

        MousePacketReady = false;
        MousePositionOld = MousePosition;
}

void InitPS2Mouse() {
    outb(0x64, 0xA8); //enabling the auxiliary device - mouse

    MouseWait();
    outb(0x64, 0x20); //tells the keyboard controller that we want to send a command to the mouse
    MouseWaitInput();
    uint8_t status = inb(0x60);
    status |= 0b10;
    MouseWait();
    outb(0x64, 0x60);
    MouseWait();
    outb(0x60, status); // setting the correct bit is the "compaq" status byte

    MouseWrite(0xF6);
    MouseRead();

    MouseWrite(0xF4);
    MouseRead();
}

__attribute__((interrupt)) void MouseInt_Hndlr(struct InterruptFrame* Frame) {
    uint8_t scancode = inb(0x60);

    HandlePS2Mouse(scancode);

    PIC_sendEOI(12);
}