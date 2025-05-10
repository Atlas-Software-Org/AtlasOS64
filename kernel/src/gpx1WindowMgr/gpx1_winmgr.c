#include <gpx1.h>
#include <memory/heap.h>
#include <mem/mem.h>
#include <InterruptDescriptors/Drivers/MouseDev/MouseDev.h>

RootWindowHandle __ins_root_win_hndl;
RootWindowHandle *RootWindowTree = &__ins_root_win_hndl;

uint8_t RootWindowTreeBitmap[MAX_WIN_USABLE / 8] = {0};

void SetBit(uint8_t* map, uint32_t bit_index, bool state) {
    uint32_t byte = bit_index / 8;
    uint8_t bit = bit_index % 8;

    if (state)
        map[byte] |= (1 << bit);
    else
        map[byte] &= ~(1 << bit);
}

uint8_t GetBit(uint8_t* map, uint32_t bit_index) {
    uint32_t byte = bit_index / 8;
    uint8_t bit = bit_index % 8;

    return (map[byte] >> bit) & 1;
}

extern void outb(uint16_t,uint8_t);
int idx = 0;
void d() {
    outb(0xE9, 'a'+idx);
    idx++;
}

void FinishedWindowProcNone() {
    asm volatile ("nop");
}

Window* CreateWindow(char* title, uint64_t width, uint64_t height, void (*FinishWindowProc)) {
    int slot_found = -1;
    for (int i = 0; i < MAX_WIN_USABLE / 8; i++) {
        if (GetBit(RootWindowTreeBitmap, i) == 0) {
            slot_found = i * 8;
            break;
        }
    }

    if (slot_found == -1) {
        return NULL; // No available slot found
    }

    // Mark slot as used
    SetBit(RootWindowTreeBitmap, slot_found, true);

    // Create and initialize the window
    Window* window = (Window*)malloc(sizeof(Window));
    window->____exists__ = true;
    if (!window) {
        return NULL;
    }

    // Copy the title
    int len_title = 0;
    while (title[len_title] != 0) {
        len_title++;
    }

    for (int i = 0; i < len_title; i++) {
        window->WinName[i] = title[i];
    }

    // Allocate framebuffer memory
    window->winfb = (WindowFramebuffer*)malloc(sizeof(WindowFramebuffer));
    if (!window->winfb) {
        free(window);
        return NULL;
    }
    window->winfb->fbaddr = (uint32_t*)malloc(width * height * 4); // Allocate framebuffer
    if (!window->winfb->fbaddr) {
        free(window->winfb);
        free(window);
        return NULL;
    }
    window->winfb->width = width;
    window->winfb->height = height;
    window->winfb->dispx = 35;
    window->winfb->dispy = 35;
    window->winfb->pitch = width * 4; // Assuming 32-bit color depth
    window->winfb->bpp = 4;
    window->____isRoot__ = false;

    // Assign the repaint function
    window->Repaint = Repaint;
    window->win_attr = 0;

    // Create and initialize the "EXIT" button
    Button_t newButton = {
        "EXIT",
        FinishWindowProc == NULL ? FinishedWindowProcNone : FinishWindowProc,
        {window->winfb->dispx + width - 22, window->winfb->dispy},
        {22, 22},
        1
    };

    window->exit_button = &newButton;

    int idx = AddButton(&newButton); // Add the button to the system

    RootWindowTree->WinHandles[slot_found] = *window;
    RootWindowTree->NumUsedWinHandles++;

    return window;
}

Window* CreateRootWindow() {
    Window* window = (Window*)malloc(sizeof(Window));
    window->____exists__ = true;
    window->____isRoot__ = true;
    if (!window) {
        return NULL;
    }

    window->winfb = (WindowFramebuffer*)malloc(sizeof(WindowFramebuffer));
    if (!window->winfb) {
        free(window);
        return NULL;
    }
    window->winfb->fbaddr = (uint32_t*)malloc(GetFb()->width * GetFb()->height * 4);
    if (!window->winfb->fbaddr) {
        free(window->winfb);
        free(window);
        return NULL;
    }
    window->winfb->width = GetFb()->width;
    window->winfb->height = GetFb()->height;
    window->winfb->dispx = 0;
    window->winfb->dispy = 0;
    window->winfb->pitch = GetFb()->width * 4;
    window->winfb->bpp = 4;

    window->Repaint = Repaint;
    window->win_attr = 0;

    RootWindowTree->RootWindow = window;

    return window;
}

static const Window* WindowNull = {
    0, NULL, NULL, 0, NULL, false
};

void FreeWindow(Window* window) {
    if (window) {
        for (int i = 0; i < 1024; i++) {
            if (RootWindowTree->WinHandles[i].winfb->fbaddr == window->winfb->fbaddr &&
                RootWindowTree->WinHandles[i].winfb->height == window->winfb->height &&
                RootWindowTree->WinHandles[i].winfb->width == window->winfb->width
            ) {
                RootWindowTree->WinHandles[i] = *WindowNull;
            }
        }
        // Free framebuffer memory
        if (window->winfb) {
            if (window->winfb->fbaddr) {
                free(window->winfb->fbaddr);
            }
            free(window->winfb);
        }

        // Remove the button associated with the window
        if (window->exit_button) {
            RemoveButton(window->exit_button->idx);
        }

        free(window); // Free the window structure
    }
}

void Repaint(Window* window) {
    if (!window || !window->winfb || !window->winfb->fbaddr) return;

    uint64_t startx = window->winfb->dispx;
    uint64_t starty = window->winfb->dispy;
    uint64_t width = window->winfb->width;
    uint64_t height = window->winfb->height;
    uint64_t pitch = window->winfb->pitch;

    volatile uint32_t* fb = (volatile uint32_t*)window->winfb->fbaddr;

    uint64_t screenw = GetFb()->width;
    uint64_t screenh = GetFb()->height;

    if (!window->____isRoot__) {
        for (uint64_t x = startx; x < startx + width; x++) {
            for (uint64_t y = starty; y < starty + 22; y++) {
                if (x < screenw && y < screenh) PutPx(x, y, 0xAAAAAA);
            }
        }

        if (startx + 7 < screenw && starty + 4 < screenh) {
            FontPutStr(window->WinName, startx + 5, starty + 2, 0x000000);
            FontPutStr(window->WinName, startx + 7, starty + 4, 0x000000);
            FontPutStr(window->WinName, startx + 7, starty + 2, 0x000000);
            FontPutStr(window->WinName, startx + 5, starty + 4, 0x000000);
            FontPutStr(window->WinName, startx + 6, starty + 3, 0xFFFFFF);
        }

        uint64_t cx = startx + width - 11;
        uint64_t cy = starty + 11;

        for (uint64_t y = 0; y < 20; y++) {
            for (uint64_t x = 0; x < 20; x++) {
                int64_t dx = (int64_t)x - 11;
                int64_t dy = (int64_t)y - 11;
                if (dx * dx + dy * dy <= 100) {
                    uint64_t px = cx - 11 + x;
                    uint64_t py = cy - 11 + y;
                    if (px < screenw && py < screenh) PutPx(px, py, 0xE82828);
                }
            }
        }
    }

    uint64_t contentStartY = starty + (window->____isRoot__ ? 0 : 22);

    for (uint64_t y = 0; y < height; y++) {
        for (uint64_t x = 0; x < width; x++) {
            uint64_t px = startx + x;
            uint64_t py = contentStartY + y;
            if (px < screenw && py < screenh) {
                uint64_t pxidx = (y * pitch / 4) + x;
                uint32_t color = fb[pxidx];
                PutPx(px, py, color);
            }
        }
    }
}

__attribute__((hot)) void WinPutPx(Window* window, uint64_t x, uint64_t y, uint32_t color) {
    if (!window || !window->winfb || !window->winfb->fbaddr) return;
    if (x >= window->winfb->width || y >= window->winfb->height) return;

    volatile uint32_t* px = (volatile uint32_t*)window->winfb->fbaddr;
    px += y * (window->winfb->pitch / 4) + x;
    *px = color;
}

void WinPutStr(Window* window, uint64_t x, uint64_t y, const char* str, uint32_t color) {
    uint64_t xoff = x;
    uint64_t yoff = y;

    uint64_t screenwidthChars = GetFb()->width / 8;
    uint64_t screenheightChars = GetFb()->height / 16;

    while (*str) {
        char c = *str;

        if (c == '\b') {
            if (xoff > x) {
                xoff -= 8;
            }
            DrawRect(xoff, yoff, 8, 16, ClearColor);
        } else if (c == '\n') {
            xoff = x;
            yoff += 16;
            if (yoff >= GetFb()->height) {
                yoff = GetFb()->height - 16;
            }
        } else if (c == '\r') {
            xoff = x;
        } else {
            if (xoff + 8 > GetFb()->width) {
                xoff = x;
                yoff += 16;
                if (yoff >= GetFb()->height) {
                    yoff = GetFb()->height - 16;
                }
            }

            WinPutChar(window, xoff, yoff, c, color);
            xoff += 8;
        }

        str++;
    }
}

void WinPutChar(Window* window, uint64_t x, uint64_t y, char c, uint32_t color) {
    uint8_t* fontPtr = (uint8_t*)main_psf1_font->glyphBuffer + (c * main_psf1_font->psf1_Header->charsize);

    for (uint64_t row = 0; row < main_psf1_font->psf1_Header->charsize; row++) {
        uint8_t pixelData = fontPtr[row];

        for (uint64_t col = 0; col < 8; col++) {
            if ((pixelData >> (7 - col)) & 1) {
                WinPutPx(window, x + col, y + row, color);
            }
        }
    }
}
