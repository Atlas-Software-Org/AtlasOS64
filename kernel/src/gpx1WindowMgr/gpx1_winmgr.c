#include <gpx1.h>
#include <paging/paging.h>

void Repaint(Window* window_ctx) {
    if (window_ctx == NULL || window_ctx->winfb == NULL) {
        return;
    }
    
    for (uint64_t w = 0; w < window_ctx->winfb->width; w++) {
        for (uint64_t h = 0; h < 22; h++) {
            PutPx(window_ctx->winfb->dispx + w, window_ctx->winfb->dispy + h, 0xFF282828);
        }

        for (uint64_t h = 22; h < window_ctx->winfb->height + 22; h++) {
            volatile uint32_t* win_ptr = window_ctx->winfb->address;
            uint32_t color = win_ptr[h * (window_ctx->winfb->pitch / 4) + w];
            PutPx(window_ctx->winfb->dispx + w, window_ctx->winfb->dispy + h, color);
        }
    }

    for (uint64_t w = 0; w < 22; w++) {
        for (uint64_t h = 0; h < 22; h++) {
            PutPx(window_ctx->winfb->dispx + window_ctx->winfb->width - 22 + w,
                  window_ctx->winfb->dispy + h, 0xFFFF0000);
        }
    }

    int title_length = 0;
    while (window_ctx->WinName[title_length] != '\0') {
        title_length++;
    }

    int title_width = title_length * 8;
    int title_x = window_ctx->winfb->dispx + (window_ctx->winfb->width - title_width) / 2;
    int title_y = window_ctx->winfb->dispy + (22 - 16) / 2;

    FontPutStr(window_ctx->WinName, title_x, title_y, 0xFFFFFFFF);

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
    winctx->winfb->address = (void*)page_alloc_n(framebuffer_size);
    if (!winctx->winfb->address) return NULL;

    winctx->winfb->width = width;
    winctx->winfb->height = height;
    winctx->winfb->dispx = 25;
    winctx->winfb->dispy = 25;

    for (uint64_t x = 0; x < width; x++) {
        for (uint64_t y = 0; y < height; y++) {
            volatile uint32_t* ptr = (volatile uint32_t*)winctx->winfb->address;
            ptr[y * width + x] = 0xFF303030;
        }
    }

    Repaint(winctx);

    return winctx;
}

void WinPutPx(Window* winctx, uint64_t x, uint64_t y, uint32_t color) {
    if (winctx == NULL || winctx->winfb == NULL) {
        return;
    }

    if (x >= winctx->winfb->width || y >= winctx->winfb->height) {
        return;
    }

    volatile uint32_t* framebuffer = (volatile uint32_t*)winctx->winfb->address;

    uint64_t offset = y * (winctx->winfb->pitch / 4) + x;

    framebuffer[offset] = color;
}
