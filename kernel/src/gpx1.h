#pragma once

#include <stdint.h>
#include <limine.h>

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
uint32_t GetPx(uint64_t x, uint64_t y);
void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr);

void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr);
void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr);

unsigned int *tga_parse(unsigned char *ptr, int size);
int DisplayTarga(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);

unsigned int *gpx1_parse(unsigned char* ptr, int size);
int DisplayGraphyx1(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos);