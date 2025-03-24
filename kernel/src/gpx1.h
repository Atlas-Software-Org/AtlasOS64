#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <stdbool.h>

typedef struct Point{
    long X;
    long Y;
} Point;

extern bool MouseDrawn;
extern uint32_t MouseCursorBuffer[16 * 16];
extern uint32_t MouseCursorBufferAfter[16 * 16];
extern Point CursorPosition;

typedef struct _button_t {
    char label[17];
    void (*Handler)();
    Point Position;
    Point Scale;
    int Enabled;
} Button_t;

struct window_t;

typedef void (*RepaintFunc)(struct window_t*);

typedef struct windowfb_t {
    void* address;
    void* fbaddr;
    uint64_t width;
    uint64_t height;
    uint64_t dispx;
    uint64_t dispy;
    uint64_t pitch;
    uint64_t bpp;
} WindowFramebuffer;

typedef struct window_t {
    char* WinName;
    WindowFramebuffer* winfb;
    RepaintFunc Repaint;
    uint8_t win_attr;
    Button_t exit_button;
} Window;

#define MAX_WIN_USABLE 1024

typedef struct root_window_handle {
    int NumUsedWinHandles;
    Window WinHandles[MAX_WIN_USABLE];
} RootWindowHandle;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
} PSF1_HEADER;

typedef struct {
    PSF1_HEADER* psf1_Header;
    uint8_t* glyphBuffer;
} PSF1_FONT;

typedef struct {
    uint8_t magic1;             // must be zero
    uint8_t colormap;           // must be zero
    uint8_t encoding;           // must be 2
    unsigned short cmaporig;    // must be zero
    unsigned short cmaplen;     // must be zero
    uint8_t cmapent;            // must be zero
    unsigned short x;           // must be zero
    unsigned short y;           // must be zero
    unsigned short h;           // image's height
    unsigned short w;           // image's width
    uint8_t bpp;                // must be 32
    uint8_t pixeltype;          // must be 40
} __attribute__((packed)) tga_header_t;

typedef struct {
    uint8_t magic[4];           // must be 0xAB 0xFF 0xEA 0x00
    uint64_t x_len;             // length of the image on x axis
    uint64_t y_len;             // length of the image on y axis
    uint8_t bpp;                // Bits per pixel, prefered 32
    uint8_t header_end_mag[2];  // must be 0xAB 0xEA
    // DATA
} __attribute__((packed)) gpx1_header_t; // Graphyx 1 image file format

extern PSF1_FONT* main_psf1_font;
extern uint32_t ClearColor;

void InitGfx(struct limine_framebuffer* fb);
struct limine_framebuffer *GetFb();
void PutPx(uint64_t x, uint64_t y, uint32_t clr);
void AddrPutPx(volatile uint32_t* addr, uint64_t x, uint64_t y, uint32_t clr);
uint32_t GetPx(uint64_t x, uint64_t y);
uint32_t AddrGetPx(volatile uint32_t* addr, uint64_t x, uint64_t y);
void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr);

void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr);
void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr);

unsigned int *tga_parse(unsigned char *ptr, int size);
int DisplayTarga(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);

unsigned int *gpx1_parse(unsigned char* ptr, int size);
int DisplayGraphyx1(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);

void WinPutPx(Window* windowctx, uint64_t x, uint64_t y, uint32_t color);
void WinPutStr(Window* windowctx, uint64_t x, uint64_t y, const char* str, uint32_t color);
void WinPutChar(Window* windowctx, uint64_t x, uint64_t y, char c, uint32_t color);

Window* CreateWindow(char* title, uint64_t width, uint64_t height, void (*FinishWindowProc));
void Repaint(Window* window_ctx);

void ClearMouseCursor(uint8_t* MouseCursor, Point Position);
void DrawOverlayMouseCursor(uint8_t* MouseCursor, Point Position, uint32_t Colour);
