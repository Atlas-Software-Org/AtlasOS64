#ifndef GPX1_H
#define GPX1_H 1

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
    int idx;
} Button_t;

typedef struct windowfb_t {
    void* fbaddr;
    uint64_t width;
    uint64_t height;
    uint64_t dispx;
    uint64_t dispy;
    uint64_t pitch;
    uint64_t bpp;
} WindowFramebuffer;

struct window_t;

typedef void (*RepaintFunc)(struct window_t*);

typedef struct window_t {
    char WinName[17];
    WindowFramebuffer* winfb;
    RepaintFunc Repaint;
    uint8_t win_attr;
    Button_t *exit_button;
    bool ____exists__;
    bool ____isRoot__;
} Window;

#define MAX_WIN_USABLE 1024

typedef struct root_window_handle {
    int NumUsedWinHandles;
    Window WinHandles[MAX_WIN_USABLE];
    Window* RootWindow;
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
extern PSF1_FONT* mainAr_psf1_font;
extern uint32_t ClearColor;

typedef struct {
    uint64_t x, y, width, height;
    uint32_t pixels[];  // Dynamic array to store original pixel colors
} RectBuffer;

extern Point SelectionBoxStart;
extern Point SelectionBoxEnd;
extern bool IsRightBtnPressed;

/* General Graphics */
void InitGfx(struct limine_framebuffer* fb);
struct limine_framebuffer *GetFb();
void PutPx(uint64_t x, uint64_t y, uint32_t clr);
void AddrPutPx(volatile uint32_t* addr, uint64_t x, uint64_t y, uint32_t clr);
uint32_t GetPx(uint64_t x, uint64_t y);
uint32_t AddrGetPx(volatile uint32_t* addr, uint64_t x, uint64_t y);
void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr);
void DrawRectOutline(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t clr);
void DrawRectOutlineDotted(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t clr);
void DeleteRectOutlineDotted(void);
void SaveRectToBuffer(uint64_t x, uint64_t y, uint64_t width, uint64_t height);
void RestoreRectFromBuffer(uint64_t x, uint64_t y, uint64_t width, uint64_t height);
void DrawSelectionMarkers();
void DrawSelectionBox(uint64_t x, uint64_t y, uint64_t width, uint64_t height);

typedef enum {
    ATES_COLOR_RESET = 0xE4E4E4,        // 0: Reset (Slight-Dark White)
    ATES_COLOR_RED = 0xFF0000,          // 1: Red
    ATES_COLOR_GREEN = 0x00FF00,        // 2: Green
    ATES_COLOR_BLUE = 0x0000FF,         // 3: Blue
    ATES_COLOR_WHITE = 0xFFFFFF,        // 4: White
    ATES_COLOR_BLACK = 0x000000,        // 5: Black
    ATES_COLOR_BRIGHT_RED = 0xFF6666,   // 6: Bright Red
    ATES_COLOR_BRIGHT_GREEN = 0x66FF66, // 7: Bright Green
    ATES_COLOR_BRIGHT_BLUE = 0x6666FF,  // 8: Bright Blue
    ATES_COLOR_GRAY = 0x808080,         // 9: Gray
} AtesColorValues24;

AtesColorValues24 ParseAtes(int code);

void FontPutAtesChar(char c, uint64_t x, uint64_t y);
void FontPutAtesStr(const char* s, uint64_t x, uint64_t y);
void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr);
void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr);
void FontPutCharSize(char c, uint64_t x, uint64_t y, uint32_t clr, int size_multiplier);
void FontPutStrSize(const char* s, uint64_t x, uint64_t y, uint32_t clr, int size_multiplier);

unsigned int *tga_parse(unsigned char *ptr, int size);
int DisplayTarga(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);

unsigned int *gpx1_parse(unsigned char* ptr, int size);
int DisplayGraphyx1(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);

/* Window Manager */
__attribute__((hot)) void WinPutPx(Window* window, uint64_t x, uint64_t y, uint32_t color);
void WinPutStr(Window* window, uint64_t x, uint64_t y, const char* str, uint32_t color);
void WinPutChar(Window* window, uint64_t x, uint64_t y, char c, uint32_t color);

Window* CreateWindow(char* title, uint64_t width, uint64_t height, void (*FinishWindowProc));
Window* CreateRootWindow();
void Repaint(Window* window);
void FreeWindow(Window* window);
/* ^ Window Manager ^ */

void ClearMouseCursor(uint8_t* MouseCursor, Point Position);
void DrawOverlayMouseCursor(uint8_t* MouseCursor, Point Position, uint32_t Colour);

void ClearScreenColor(uint32_t color);

void DrawBmp(void* ptr, uint64_t size, int xoff, int yoff);

void DrawLine(int x0, int y0, int x1, int y1, uint32_t color);

// Arabic

void ArPuts(const char* _s, uint64_t x, uint64_t y, uint32_t clr);
char* En2Ar(const char* _en_pron, void* out);
char* UniAr2Ar(const char* _s, void* out);

uint32_t* getPixelsInSize(void* bmpPtr, size_t fsz);

#endif /* GPX1_H */