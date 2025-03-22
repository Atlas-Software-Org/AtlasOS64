#include <gpx1.h>
#include <paging/paging.h>

void Repaint(Window* window_ctx) {
    if (window_ctx == NULL || window_ctx->winfb == NULL) {
        return;
    }
    
    for (uint64_t w = 0; w < window_ctx->winfb->width; w++) {
        for (uint64_t h = 0; h < 22; h++) {
            PutPx((w+window_ctx->winfb->dispx), (h+window_ctx->winfb->dispy), 0xFF282828);
        }
        for (uint64_t h = 0; h < 22; h++) {
            for (uint64_t w = window_ctx->winfb->width - 22; w < window_ctx->winfb->width; w++) {
                PutPx(window_ctx->winfb->dispx + w, window_ctx->winfb->dispy + h, 0xFFE82828);
            }
        }       
        for (uint64_t h = 22; h < window_ctx->winfb->height+22; h++) {
            volatile uint32_t *win_ptr = window_ctx->winfb->address;

            PutPx(w + window_ctx->winfb->dispx, h + window_ctx->winfb->dispy, win_ptr[h * (window_ctx->winfb->pitch / 4) + w]);
        }
    }

    FontPutStr(window_ctx->WinName, window_ctx->winfb->dispx + 8, window_ctx->winfb->dispy + (22 - 16) / 2, 0xFFFFFFFF);
    int x_pos = window_ctx->winfb->dispx + window_ctx->winfb->width - 22 + (22 - 8) / 2;
    int y_pos = window_ctx->winfb->dispy + (22 - 16) / 2;

    FontPutChar('X', x_pos, y_pos, 0xFFFFFFFF);
}

#include <HtKernelUtils/io.h>

static size_t calculate_pages(uint32_t width, uint32_t height) {
    size_t total_bytes = (size_t)width * height * 4;
    return (total_bytes + 4095) / 4096;
}

Window* CreateWindow(char* title, uint64_t width, uint64_t height, void* address) {
    Window* winctx = (Window*)page_alloc(sizeof(Window) / 4096 + 1);
    if (!winctx) return NULL;

    winctx->winfb = (WindowFramebuffer*)page_alloc(sizeof(WindowFramebuffer) / 4096 + 1);
    if (!winctx->winfb) return NULL;

    winctx->WinName = title;

    size_t framebuffer_size = calculate_pages(width, height);
    winctx->winfb->address = (void*)page_alloc_n(framebuffer_size / 4096 + 1);
    if (!winctx->winfb->address) return NULL;

    winctx->winfb->width = width;
    winctx->winfb->height = height;
    winctx->winfb->dispx = 25;
    winctx->winfb->dispy = 25;

    for (uint64_t x = 0; x < width; x++) {
        for (uint64_t y = 0; y < height; y++) {
            volatile uint32_t* ptr = (volatile uint32_t*)winctx->winfb->address;
            ptr[y * width + x] = 0xFF303030;  // Set pixel to a dark color
        }
    }

    Repaint(winctx);

    return winctx;
}

void WinPutPx(Window* window_ctx, uint64_t x, uint64_t y, uint32_t color) {
    if (window_ctx == NULL || window_ctx->winfb == NULL) {
        return;
    }

    volatile uint32_t* ptr = (volatile uint32_t*)window_ctx->winfb->address;
    uint64_t index = y * window_ctx->winfb->width + x;
    ptr[index] = color;
}

void WinPutStr(Window* windowctx, uint64_t x, uint64_t y, const char* str, uint32_t color) {
    while (*str != '\0') {
        WinPutChar(windowctx, x, y, *str, color);
        str++;
        x += 8;
    }
}

void WinPutChar(Window* windowctx, uint64_t x, uint64_t y, char c, uint32_t color) {
    uint8_t* fontPtr = (uint8_t*)main_psf1_font->glyphBuffer + (c * main_psf1_font->psf1_Header->charsize);

    for (uint64_t row = 0; row < main_psf1_font->psf1_Header->charsize; row++) {
        uint8_t pixelData = fontPtr[row];

        for (uint64_t col = 0; col < 8; col++) {
            if ((pixelData >> (7 - col)) & 1) {
                WinPutPx(windowctx, x + col, y + row, color);
            }
        }
    }
}