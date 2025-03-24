#include <gpx1.h>
#include <paging/paging.h>

typedef struct {
    uint64_t x;
    uint64_t y;
    uint64_t width;
    uint64_t height;
} Rect;

void FillRect(Rect rect, uint32_t color) {
    DrawRect(rect.x, rect.y, rect.width, rect.height, color);
}

void Blit(void* source, uint32_t pitch, Rect dest) {
    uint32_t* src_pixels = (uint32_t*)source;
    uint32_t pixels_per_row = pitch / 4;

    for (uint64_t y = 0; y < dest.height; y++) {
        for (uint64_t x = 0; x < dest.width; x++) {
            uint32_t color = src_pixels[y * pixels_per_row + x];
            PutPx(dest.x + x, dest.y + y, color);
        }
    }
}

void Repaint(Window* window_ctx) {
    if (!window_ctx || !window_ctx->winfb) {
        return;
    }

    for (uint64_t x = 0; x < window_ctx->winfb->width; x++) {
        for (uint64_t y = 0; y < 22; y++) {
            PutPx(window_ctx->winfb->dispx + x, window_ctx->winfb->dispy + y, 0xFF282828);
        }
    }

    volatile uint32_t* win_ptr = (volatile uint32_t*)window_ctx->winfb->address;
    uint64_t pitch_in_pixels = window_ctx->winfb->pitch / 4;
    for (uint64_t y = 0; y < window_ctx->winfb->height; y++) {
        for (uint64_t x = 0; x < window_ctx->winfb->width; x++) {
            uint32_t color = win_ptr[y * pitch_in_pixels + x];
            PutPx(window_ctx->winfb->dispx + x, window_ctx->winfb->dispy + 22 + y, color);
        }
    }

    for (uint64_t x = 0; x < 22; x++) {
        for (uint64_t y = 0; y < 22; y++) {
            PutPx(window_ctx->winfb->dispx + window_ctx->winfb->width - 22 + x,
                  window_ctx->winfb->dispy + y,
                  0xFFFF0000);
        }
    }

    int title_length = 0;
    while (window_ctx->WinName[title_length] != '\0') {
        title_length++;
    }
    int title_width = title_length * 8;  // Each character is 8px wide.
    int title_x = window_ctx->winfb->dispx + (window_ctx->winfb->width - title_width) / 2;
    int title_y = window_ctx->winfb->dispy + (22 - 16) / 2; // Assuming 16px tall font
    FontPutStr(window_ctx->WinName, title_x, title_y, 0xFFFFFFFF);

    int x_pos = window_ctx->winfb->dispx + window_ctx->winfb->width - 22 + (22 - 8) / 2;
    int y_pos = window_ctx->winfb->dispy + (22 - 16) / 2;
    FontPutChar('X', x_pos, y_pos, 0xFFFFFFFF);
}

static size_t calculate_pages(uint32_t width, uint32_t height) {
    size_t total_bytes = (size_t)width * height * 4;
    return (total_bytes + 4095) / 4096;
}

Window* CreateWindow(char* title, uint64_t width, uint64_t height, void (*FinishWindowProc)) {
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
    Button_t exit = {0};
    exit.Handler = FinishWindowProc;
    exit.Position.X = 25 + width - 22;
    exit.Position.Y = 25 + height - 22;
    exit.Scale.X = 22;
    exit.Scale.Y = 22;
    exit.Enabled = 1;
    
    winctx->exit_button = exit;      

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
