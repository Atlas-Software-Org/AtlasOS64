#include <gpx1.h>
#include <mem/mem.h>

struct limine_framebuffer* main_fb;
PSF1_FONT* main_psf1_font;
uint32_t ClearColor;

bool MouseDrawn;
uint32_t MouseCursorBuffer[16 * 16];
uint32_t MouseCursorBufferAfter[16 * 16];
Point CursorPosition;

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

void AddrPutPx(volatile uint32_t* addr, uint64_t x, uint64_t y, uint32_t clr) {
    addr[y * (main_fb->pitch / (main_fb->bpp/8)) + x] = clr;
}

uint32_t GetPx(uint64_t x, uint64_t y) {
    volatile uint32_t *fb_ptr = main_fb->address;
    return (fb_ptr[y * (main_fb->pitch / (main_fb->bpp/8)) + x]);
}

uint32_t AddrGetPx(volatile uint32_t* addr, uint64_t x, uint64_t y) {
    return (addr[y * (main_fb->pitch / (main_fb->bpp/8)) + x]);
}

void DrawRect(uint64_t x, uint64_t y, uint64_t width, uint64_t len, uint32_t clr) {
    for (uint64_t x__ = x; x__ < x + width; x__++) {
        for (uint64_t y__ = y; y__ < y + len; y__++) {
            PutPx(x__, y__, clr);
        }
    }
}

void DrawRectOutline(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t clr) {
    uint64_t startX = (x < x + width) ? x : x + width;
    uint64_t endX = (x < x + width) ? x + width : x;

    uint64_t startY = (y < y + height) ? y : y + height;
    uint64_t endY = (y < y + height) ? y + height : y;

    for (uint64_t i = startX; i < endX; i++) {
        PutPx(i, startY, clr);
        PutPx(i, endY - 1, clr);
    }

    for (uint64_t i = startY; i < endY; i++) {
        PutPx(startX, i, clr);
        PutPx(endX - 1, i, clr);
    }
}

#define MAX_RECT_SIZE 1024

static uint32_t SelectionRectBuffer[MAX_RECT_SIZE * MAX_RECT_SIZE];
bool IsRightBtnPressed = false;

Point SelectionBoxStart = {0, 0};
Point SelectionBoxEnd = {0, 0};

void DrawSelectionMarkers() {
    PutPx(SelectionBoxStart.X, SelectionBoxStart.Y, 0xFF0000);  // Red color
    PutPx(SelectionBoxEnd.X, SelectionBoxEnd.Y, 0x00FF00);  // Green color
}

void DrawSelectionBox(uint64_t x, uint64_t y, uint64_t width, uint64_t height) {
    DrawRectOutlineDotted(x, y, width, height, 0xFFFFFFFF);  // White color
}

void SaveRectToBuffer(uint64_t x, uint64_t y, uint64_t width, uint64_t height) {
    if (width > MAX_RECT_SIZE || height > MAX_RECT_SIZE) {
        return;
    }

    for (uint64_t i = 0; i < height; i++) {
        for (uint64_t j = 0; j < width; j++) {
            SelectionRectBuffer[i * width + j] = GetPx(x + j, y + i);
        }
    }
}

void RestoreRectFromBuffer(uint64_t x, uint64_t y, uint64_t width, uint64_t height) {
    if (width > MAX_RECT_SIZE || height > MAX_RECT_SIZE) {
        return;
    }

    for (uint64_t i = 0; i < height; i++) {
        for (uint64_t j = 0; j < width; j++) {
            PutPx(x + j, y + i, SelectionRectBuffer[i * width + j]);
        }
    }
}

void DrawRectOutlineDotted(uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint32_t clr) {
    for (uint64_t i = x; i < x + width; i += 2) {
        PutPx(i, y, clr);
        PutPx(i, y + height - 1, clr);
    }
    for (uint64_t i = y; i < y + height; i += 2) {
        PutPx(x, i, clr);
        PutPx(x + width - 1, i, clr);
    }
}

void DeleteRectOutlineDotted(void) {
    RestoreRectFromBuffer(SelectionBoxStart.X, SelectionBoxStart.Y,
                          (SelectionBoxEnd.X > SelectionBoxStart.X) ? SelectionBoxEnd.X - SelectionBoxStart.X : SelectionBoxStart.X - SelectionBoxEnd.X,
                          (SelectionBoxEnd.Y > SelectionBoxStart.Y) ? SelectionBoxEnd.Y - SelectionBoxStart.Y : SelectionBoxStart.Y - SelectionBoxEnd.Y);
}

int gpx1_ascii_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else {
        return -1; // Invalid digit
    }
}

char gpx1_int_to_ascii(int num) {
    if (num >= 0 && num <= 9) {
        return '0' + num;
    } else {
        return '\0';
    }
}

uint32_t AtesColor = ATES_COLOR_RESET; // Default color

AtesColorValues24 ParseAtes(int code) {
    switch (code) {
        case 0: return ATES_COLOR_RESET;
        case 1: return ATES_COLOR_RED;
        case 2: return ATES_COLOR_GREEN;
        case 3: return ATES_COLOR_BLUE;
        case 4: return ATES_COLOR_WHITE;
        case 5: return ATES_COLOR_BLACK;
        case 6: return ATES_COLOR_BRIGHT_RED;
        case 7: return ATES_COLOR_BRIGHT_GREEN;
        case 8: return ATES_COLOR_BRIGHT_BLUE;
        case 9: return ATES_COLOR_GRAY;
        default: return ATES_COLOR_RESET;  // Default to reset if invalid
    }
}

void FontPutAtesChar(char c, uint64_t x, uint64_t y) {
    uint8_t* fontPtr = (uint8_t*)main_psf1_font->glyphBuffer + (c * main_psf1_font->psf1_Header->charsize);

    for (uint64_t row = 0; row < main_psf1_font->psf1_Header->charsize; row++) {
        uint8_t pixelData = fontPtr[row];

        for (uint64_t col = 0; col < 8; col++) {
            if ((pixelData >> (7 - col)) & 1) {
                PutPx(x + col, y + row, AtesColor); // Use current color
            }
        }
    }
}

void FontPutAtesStr(const char* s, uint64_t x, uint64_t y) {
    uint64_t xoff = x;
    uint64_t yoff = y;

    while (*s) {
        char c = *s;

        if (c == '\b') { // Backspace
            if (xoff > x) {
                xoff -= 8;
            }
            DrawRect(xoff, yoff, 8, 16, ClearColor);
        } 
        else if (c == '\n') { // Newline
            xoff = x;
            yoff += 16;
        } 
        else if (c == '\r') { // Carriage Return
            xoff = x;
        } 
        else if (c == '\xAB') { // ATES Sequence Detected
            s++;
            c = *s;
            if (c == '[') {
                s++;
                c = *s;
                if (c >= '0' && c <= '9') {
                    int code = c - '0';
                    AtesColor = ParseAtes(code);
                    s++;
                    c = *s;
                    if (c == ']') {
                        s++;
                        c = *s;
                        if (c == ';') {
                        } else {FontPutChar('(', xoff, yoff, AtesColor);xoff += 8;FontPutChar('n', xoff, yoff, AtesColor);xoff += 8;FontPutChar('u', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar(')', xoff, yoff, AtesColor);return;}
                    } else {FontPutChar('(', xoff, yoff, AtesColor);xoff += 8;FontPutChar('n', xoff, yoff, AtesColor);xoff += 8;FontPutChar('u', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar(')', xoff, yoff, AtesColor);return;}
                } else {FontPutChar('(', xoff, yoff, AtesColor);xoff += 8;FontPutChar('n', xoff, yoff, AtesColor);xoff += 8;FontPutChar('u', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar(')', xoff, yoff, AtesColor);return;}
            } else {FontPutChar('(', xoff, yoff, AtesColor);xoff += 8;FontPutChar('n', xoff, yoff, AtesColor);xoff += 8;FontPutChar('u', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar('l', xoff, yoff, AtesColor);xoff += 8;FontPutChar(')', xoff, yoff, AtesColor);return;}
        }
        else { // Normal character
            if (xoff + 8 > main_fb->width) { // Line wrap
                xoff = x;
                yoff += 16;
            }

            if (yoff + 16 > main_fb->height) { // Scroll if needed
                break;
            }

            FontPutAtesChar(c, xoff, yoff); // Draw character with current color
            xoff += 8;
        }

        s++;
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

    uint64_t screenwidthChars = main_fb->width / 8;
    uint64_t screenheightChars = main_fb->height / 16;

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

void ClearMouseCursor(uint8_t* MouseCursor, Point Position) {
    if (!MouseDrawn) return;

    int XMax = 16;
    int YMax = 16;
    int DifferenceX = GetFb()->width - Position.X;
    int DifferenceY = GetFb()->height - Position.Y;

    if (DifferenceX < 16) XMax = DifferenceX;
    if (DifferenceY < 16) YMax = DifferenceY;

    for (int Y = 0; Y < YMax; Y++) {
        for (int X = 0; X < XMax; X++) {
            int Bit = Y * 16 + X;
            int Byte = Bit / 8;
            if ((MouseCursor[Byte] & (0b10000000 >> (X % 8)))) {
                if (GetPx(Position.X + X, Position.Y + Y) == MouseCursorBufferAfter[X + Y *16]) {
                    PutPx(Position.X + X, Position.Y + Y, MouseCursorBuffer[X + Y * 16]);
                }
            }
        }
    }
}

void DrawOverlayMouseCursor(uint8_t* MouseCursor, Point Position, uint32_t Colour) {
    int XMax = 16;
    int YMax = 16;
    int DifferenceX = GetFb()->width - Position.X;
    int DifferenceY = GetFb()->height - Position.Y;

    if (DifferenceX < 16) XMax = DifferenceX;
    if (DifferenceY < 16) YMax = DifferenceY;

    for (int Y = 0; Y < YMax; Y++) {
        for (int X = 0; X < XMax; X++) {
            int Bit = Y * 16 + X;
            int __Byte = Bit / 8;
            if ((MouseCursor[__Byte] & (0b10000000 >> (X % 8)))) {
                MouseCursorBuffer[X + Y * 16] = GetPx(Position.X + X, Position.Y + Y);
                PutPx(Position.X + X, Position.Y + Y, Colour);
                MouseCursorBufferAfter[X + Y * 16] = GetPx(Position.X + X, Position.Y + Y);

            }
        }
    }

    MouseDrawn = true;
}

void ClearScreenColor(uint32_t color) {
    DrawRect(0, 0, GetFb()->width, GetFb()->height, color);
}

#define BMP_FILE_HEADER_SIZE 14
#define BMP_INFO_HEADER_SIZE 40

typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;

void DrawBmp(void* ptr, uint64_t size, int xoff, int yoff) {
    // Cast pointer to byte (uint8_t*) for easier access
    uint8_t* imgData = (uint8_t*)ptr;
    
    // Check if the size is sufficient to include a header and pixel data
    if (size < 54) {
        return;
    }

    // Retrieve the width and height of the image from the header (assuming 24-bit BMP)
    int width = *(int*)&imgData[18];  // Width starts at byte 18
    int height = *(int*)&imgData[22]; // Height starts at byte 22

    // Offset where pixel data begins
    int dataOffset = *(int*)&imgData[10];

    // Ensure we're drawing within the bounds of the screen
    int endX = xoff + width;
    int endY = yoff + height;

    // Iterate over each pixel and use PutPx to draw it, but reverse the y-axis
    for (int y = 0; y < height; y++) {
        // In BMP, the rows are stored from bottom to top, so we reverse the y-axis here
        int row = height - 1 - y; // Flip the row index for top-to-bottom drawing

        for (int x = 0; x < width; x++) {
            // Get the pixel data (BMP is usually stored in BGR format)
            int pixelOffset = dataOffset + (row * width + x) * 3;
            uint8_t blue = imgData[pixelOffset];
            uint8_t green = imgData[pixelOffset + 1];
            uint8_t red = imgData[pixelOffset + 2];

            // Check for a valid screen area to draw
            if ((x + xoff) < 0 || (x + xoff) >= GetFb()->width || (y + yoff) < 0 || (y + yoff) >= GetFb()->height) {
                continue;  // Skip drawing if outside screen bounds
            }

            // Combine RGB into a single color value and draw it
            uint32_t color = (red << 16) | (green << 8) | blue;
            PutPx(x + xoff, y + yoff, color);
        }
    }
}