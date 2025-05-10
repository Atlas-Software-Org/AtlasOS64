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

extern RootWindowHandle* RootWindowTree;
extern void e9debugkf(const char*,...);

static int holdingWin = -1;  // Holds the index of the window being dragged
static int wasHeld = -1;  // Indicates whether a window was held (dragged)

void HandleMouseDown(uint64_t x, uint64_t y) {
    if (RootWindowTree == NULL) return;

    for (int i = 0; i < 1024; i++) {
        if (RootWindowTree->WinHandles[i].winfb == NULL) continue;

        // Check if the mouse click is inside the window's title bar (assuming 22px height for title bar)
        if (x >= RootWindowTree->WinHandles[i].winfb->dispx && x <= RootWindowTree->WinHandles[i].winfb->dispx + RootWindowTree->WinHandles[i].winfb->width &&
            y >= RootWindowTree->WinHandles[i].winfb->dispy && y <= RootWindowTree->WinHandles[i].winfb->dispy + 22) {
            
            holdingWin = i;
            wasHeld = 1;
            return;
        }
    }
}

void HandleMouseUp(uint64_t x, uint64_t y) {
    if (wasHeld != -1) {
        
        int windowIndex = holdingWin;
        if (RootWindowTree->WinHandles[windowIndex].winfb != NULL) {
            ClearScreenColor(0x000000);

            if (RootWindowTree->WinHandles[windowIndex].Repaint != NULL) {
                RootWindowTree->RootWindow->Repaint(RootWindowTree->RootWindow);
                for (int i = 0; i < 1024; i++) {
                    if (RootWindowTree->WinHandles[i].____exists__ == true) {
                        RootWindowTree->WinHandles[i].Repaint(&RootWindowTree->WinHandles[i]);
                    }
                }
                RootWindowTree->WinHandles[windowIndex].Repaint(&RootWindowTree->WinHandles[windowIndex]); // to make sure selected window is on top
            } else {
            }
        }

        wasHeld = -1;
        holdingWin = -1;
    }
}

void HandleWindowMovementMouse(uint64_t x, uint64_t y) {
    if (RootWindowTree == NULL || holdingWin == -1) {
        return;
    }

    int windowIndex = holdingWin;
    if (RootWindowTree->WinHandles[windowIndex].winfb == NULL) return;

    uint64_t newX = x;
    uint64_t newY = y;

    if (newX + RootWindowTree->WinHandles[windowIndex].winfb->width > GetFb()->width || 
        newY + RootWindowTree->WinHandles[windowIndex].winfb->height > GetFb()->height ||
        newX < 0 || newY < 0) {
        return;
    }

    if (newX + RootWindowTree->WinHandles[windowIndex].winfb->width > GetFb()->width) goto end;
    if (newY + RootWindowTree->WinHandles[windowIndex].winfb->height > GetFb()->height) goto end;

    RootWindowTree->WinHandles[windowIndex].winfb->dispx = newX;
    RootWindowTree->WinHandles[windowIndex].winfb->dispy = newY;
    end:
}

void CheckBtns(uint64_t x, uint64_t y) {
    for (int i = 0; i < last_btn; i++) {
        HandleWindowMovementMouse(x, y);

        if (x >= Buttons[i].Position.X &&
            x <= (Buttons[i].Position.X + Buttons[i].Scale.X) &&
            y >= Buttons[i].Position.Y &&
            y <= (Buttons[i].Position.Y + Buttons[i].Scale.Y)) {
            
            if (Buttons[i].Enabled == 0) {
                continue;
            }

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
    } else xNegative = false;

    if (MousePacket[0] & PS2YSign) {
        yNegative = true;
    } else yNegative = false;

    if (MousePacket[0] & PS2XOverflow) {
        xOverflow = true;
    } else xOverflow = false;

    if (MousePacket[0] & PS2YOverflow) {
        yOverflow = true;
    } else yOverflow = false;

    if (!xNegative) {
        MousePosition.X += MousePacket[1];
        if (xOverflow) {
            MousePosition.X += 255;
        }
    } else {
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
    } else {
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
        HandleMouseDown(MousePosition.X, MousePosition.Y);
        CheckBtns(MousePosition.X, MousePosition.Y);
    } else {
        HandleMouseUp(MousePosition.X, MousePosition.Y);
        holdingWin = 0;
    }
    if (MousePacket[0] & PS2Middlebutton) {

    }
    if (MousePacket[0] & PS2Rightbutton) {

    }

    MousePacketReady = false;
    MousePositionOld = MousePosition;
}

void InitPS2Mouse() {
    outb(0x64, 0xA8);

    MouseWait();
    outb(0x64, 0x20);
    MouseWaitInput();
    uint8_t status = inb(0x60);
    status |= 0b10;
    MouseWait();
    outb(0x64, 0x60);
    MouseWait();
    outb(0x60, status);

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