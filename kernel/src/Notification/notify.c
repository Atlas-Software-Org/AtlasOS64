#include "notify.h"

uint32_t _side_slide_buffer[200 * 125];
int is_side_slide_buffer_removed = 1;

static void _side_slide(uint32_t unused) {
    uint32_t* buffer = (uint32_t*)GetFb()->address;
    int width = GetFb()->width;
    int height = GetFb()->height;
    int y_start = height - 125 - 32;
    int x_start = width - 200;

    uint32_t top_color = 0xFFDAB9;
    uint32_t bottom_color = 0xFFB6C1;
    uint32_t outline_color = 0x444444;
    uint32_t shadow_color = 0x222222;

    for (int x = 199; x >= 0; x--) {
        for (int y = 0; y < 125; y++) {
            int draw_x = x_start + x;
            int draw_y = y_start + y;
            if (draw_x < 0 || draw_x >= width || draw_y < 0 || draw_y >= height) continue;

            int fb_index = draw_y * width + draw_x;
            int buf_index = y * 200 + x;

            if (is_side_slide_buffer_removed)
                _side_slide_buffer[buf_index] = buffer[fb_index];

            int is_outer_edge = (x == 0 || x == 199 || y == 0 || y == 124);
            int is_inner_shadow = (x <= 1 || y >= 123); // 2px left inner and 2px bottom inner

            if (is_outer_edge) {
                buffer[fb_index] = outline_color;
            } else if (is_inner_shadow) {
                buffer[fb_index] = shadow_color;
            } else {
                uint32_t blend = (
                    (((((top_color >> 16) & 0xFF) * (125 - y) + ((bottom_color >> 16) & 0xFF) * y) / 125) << 16) |
                    (((((top_color >> 8)  & 0xFF) * (125 - y) + ((bottom_color >> 8)  & 0xFF) * y) / 125) << 8)  |
                    ((( (top_color        & 0xFF) * (125 - y) + ((bottom_color       & 0xFF) * y)) / 125))
                );
                buffer[fb_index] = blend;
            }
        }
        for (volatile int delay = 0; delay < 50000; delay++);
    }

    is_side_slide_buffer_removed = 1;
}

static void _side_slide_remove() {
    uint32_t* buffer = (uint32_t*)GetFb()->address;
    int width = GetFb()->width;
    int height = GetFb()->height;
    int y_start = height - 125 - 32;
    int x_start = width - 200;

    for (int x = 199; x >= 0; x--) {
        for (int y = 0; y < 125; y++) {
            int fb_index = (y_start + y) * width + (x_start + x);
            int buf_index = y * 200 + x;

            buffer[fb_index] = _side_slide_buffer[buf_index];
        }
        for (volatile int delay = 0; delay < 100000; delay++);
    }

    is_side_slide_buffer_removed = 0;
}

int NotificationRecievedKbdInterrut = 0;

void notify_message(const char* message) {
    _side_slide(0xA1C6EA);
    FontPutStr(message, GetFb()->width-200+4, GetFb()->height-125-32+4, 0x202020);
    while (NotificationRecievedKbdInterrut == 0) {
        asm volatile ("hlt");
    }
    NotificationRecievedKbdInterrut = 0;
    _side_slide_remove();
}

void notify_kernel_log(const char* kern_log);
void notify_non_fatal_panic(const char* nf_panic);