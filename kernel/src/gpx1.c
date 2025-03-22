#include <gpx1.h>
#include <mem/mem.h>

struct limine_framebuffer* main_fb;
PSF1_FONT* main_psf1_font;
uint32_t ClearColor;

void InitGfx(struct limine_framebuffer* fb) {
    main_fb = fb;
}

struct limine_framebuffer *GetFb() {
    return main_fb;
}

void PutPx(uint64_t x, uint64_t y, uint32_t clr) {
    volatile uint32_t *fb_ptr = main_fb->address;
    fb_ptr[y * (main_fb->pitch / (main_fb->bpp/8)) + x] = clr;
}

uint32_t GetPx(uint64_t x, uint64_t y) {
    volatile uint32_t *fb_ptr = main_fb->address;
    return (fb_ptr[y * (main_fb->pitch / (main_fb->bpp/8)) + x]);
}

void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr) {
    for (uint64_t x__ = x; x__ < x + width; x__++) {
        for (uint64_t y__ = y; y__ < y + len; y__++) {
            PutPx(x__, y__, clr);
        }
    }
}

void FontPutChar(char c, uint64_t x, uint64_t y, uint32_t clr) {
    uint8_t* fontPtr = (uint8_t*)main_psf1_font->glyphBuffer + (c * main_psf1_font->psf1_Header->charsize);

    for (uint64_t row = 0; row < main_psf1_font->psf1_Header->charsize; row++) {
        uint8_t pixelData = fontPtr[row];

        for (uint64_t col = 0; col < 8; col++) {
            if ((pixelData >> (7 - col)) & 1) {
                PutPx(x + col, y + row, clr);
            }
        }
    }
}

void FontPutStr(const char* s, uint64_t x, uint64_t y, uint32_t clr) {
    uint64_t xoff = x;
    uint64_t yoff = y;

    uint64_t screenWidthChars = main_fb->width / 8;
    uint64_t screenHeightChars = main_fb->height / 16;

    while (*s) {
        char c = *s;

        if (c == '\b') {
            if (xoff > x) {
                xoff -= 8;
            }
            DrawRect(xoff, yoff, 8, 16, ClearColor);
        } else if (c == '\n') {
            xoff = x;
            yoff += 16;
        } else if (c == '\r') {
            xoff = x;
        } else {
            if (xoff + 8 > main_fb->width) {
                xoff = x;
                yoff += 16;
            }

            if (yoff + 16 > main_fb->height) {
                s++;
                if (*s == 0) {
                    s--;
                }
            }

            FontPutChar(c, xoff, yoff, clr);
            xoff += 8;
        }

        s++;
    }
}

unsigned int *tga_parse(unsigned char *ptr, int size)
{
    unsigned int *data;
    int i, j, k, x, y, w = (ptr[13] << 8) + ptr[12], h = (ptr[15] << 8) + ptr[14], o = (ptr[11] << 8) + ptr[10];
    int m = ((ptr[1]? (ptr[7]>>3)*ptr[5] : 0) + 18);

    if(w<1 || h<1) return NULL;

    unsigned int __data_alloc[w * h * 2];
    data = (unsigned int*) __data_alloc;
    if(!data) return NULL;

    switch(ptr[2]) {
        case 1:
            if(ptr[6]!=0 || ptr[4]!=0 || ptr[3]!=0 || (ptr[7]!=24 && ptr[7]!=32)) { return NULL; }
            for(y=i=0; y<h; y++) {
                k = ((!o?h-y-1:y)*w);
                for(x=0; x<w; x++) {
                    j = ptr[m + k++]*(ptr[7]>>3) + 18;
                    data[2 + i++] = ((ptr[7]==32?ptr[j+3]:0xFF) << 24) | (ptr[j+2] << 16) | (ptr[j+1] << 8) | ptr[j];
                }
            }
            break;
        case 2:
            if(ptr[5]!=0 || ptr[6]!=0 || ptr[1]!=0 || (ptr[16]!=24 && ptr[16]!=32)) { return NULL; }
            for(y=i=0; y<h; y++) {
                j = ((!o?h-y-1:y)*w*(ptr[16]>>3));
                for(x=0; x<w; x++) {
                    data[2 + i++] = ((ptr[16]==32?ptr[j+3]:0xFF) << 24) | (ptr[j+2] << 16) | (ptr[j+1] << 8) | ptr[j];
                    j += ptr[16]>>3;
                }
            }
            break;
        case 9:
            if(ptr[6]!=0 || ptr[4]!=0 || ptr[3]!=0 || (ptr[7]!=24 && ptr[7]!=32)) { return NULL; }
            y = i = 0;
            for(x=0; x<w*h && m<size;) {
                k = ptr[m++];
                if(k > 127) {
                    k -= 127; x += k;
                    j = ptr[m++]*(ptr[7]>>3) + 18;
                    while(k--) {
                        if(!(i%w)) { i=((!o?h-y-1:y)*w); y++; }
                        data[2 + i++] = ((ptr[7]==32?ptr[j+3]:0xFF) << 24) | (ptr[j+2] << 16) | (ptr[j+1] << 8) | ptr[j];
                    }
                } else {
                    k++; x += k;
                    while(k--) {
                        j = ptr[m++]*(ptr[7]>>3) + 18;
                        if(!(i%w)) { i=((!o?h-y-1:y)*w); y++; }
                        data[2 + i++] = ((ptr[7]==32?ptr[j+3]:0xFF) << 24) | (ptr[j+2] << 16) | (ptr[j+1] << 8) | ptr[j];
                    }
                }
            }
            break;
        case 10:
            if(ptr[5]!=0 || ptr[6]!=0 || ptr[1]!=0 || (ptr[16]!=24 && ptr[16]!=32)) { return NULL; }
            y = i = 0;
            for(x=0; x<w*h && m<size;) {
                k = ptr[m++];
                if(k > 127) {
                    k -= 127; x += k;
                    while(k--) {
                        if(!(i%w)) { i=((!o?h-y-1:y)*w); y++; }
                        data[2 + i++] = ((ptr[16]==32?ptr[m+3]:0xFF) << 24) | (ptr[m+2] << 16) | (ptr[m+1] << 8) | ptr[m];
                    }
                    m += ptr[16]>>3;
                } else {
                    k++; x += k;
                    while(k--) {
                        if(!(i%w)) { i=((!o?h-y-1:y)*w); y++; }
                        data[2 + i++] = ((ptr[16]==32?ptr[m+3]:0xFF) << 24) | (ptr[m+2] << 16) | (ptr[m+1] << 8) | ptr[m];
                        m += ptr[16]>>3;
                    }
                }
            }
            break;
        default:
            return NULL;
    }
    data[0] = w;
    data[1] = h;
    return data;
}

int DisplayTarga(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos) {
    tga_header_t* header = (tga_header_t*)ptr;
    uint32_t* pixel_data = tga_parse(ptr, size);
    uint64_t x_len = header->w;
    uint64_t y_len = header->h;

    if (pixel_data == NULL)
        return -1;

    if (header->bpp != 32)
        return -2;

    for (uint64_t x = 0; x < x_len; x++) {
        for (uint64_t y = 0; y < y_len; y++) {
            PutPx(x+xpos, y+ypos, pixel_data[y * x_len + x]);
        }
    }

    return 0;
}

unsigned int *gpx1_parse(unsigned char* ptr, int size) {
    gpx1_header_t* header = (gpx1_header_t*)ptr;

    if (header->magic[0] == 0xAB &&
        header->magic[1] == 0xFF &&
        header->magic[2] == 0xEA &&
        header->magic[3] == 0x00 &&
        header->header_end_mag[0] == 0xAB &&
        header->header_end_mag[1] == 0xEA) {}
    else {
        return NULL;
    }

    char* ptr2 = (char*)&ptr + sizeof(gpx1_header_t);

    return (uint32_t*)ptr2;
}

int DisplayGraphyx1(void* ptr, uint64_t size, uint64_t xpos, uint64_t ypos) {
    gpx1_header_t* header = (gpx1_header_t*)ptr;
    uint32_t* pixel_data = gpx1_parse(ptr, size);
    uint64_t x_len = header->x_len;
    uint64_t y_len = header->y_len;

    if (pixel_data == NULL)
        return -1;

    if (header->bpp != 32)
        return -2;

    for (uint64_t x = 0; x < x_len; x++) {
        for (uint64_t y = 0; y < y_len; y++) {
            PutPx(x+xpos, y+ypos, pixel_data[y * x_len + x]);
        }
    }

    return 0;
}