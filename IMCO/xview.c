#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ------------------------------
// BMP Loader (24-bit to 32-bit ARGB)
// ------------------------------
#pragma pack(push, 1)
typedef struct {
    uint8_t magic[2];       // "BM"
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
    uint32_t headerSize;    // Should be 40
    int32_t width;
    int32_t height;
    uint16_t planes;        // Must be 1
    uint16_t bitsPerPixel;  // Should be 24
    uint32_t compression;   // 0 = BI_RGB
    uint32_t imageSize;     // May be 0 for uncompressed images
    int32_t xPixelsPerM;
    int32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t importantColors;
} BMPHeader;
#pragma pack(pop)

uint32_t* load_bmp_image(const char *filename, int* width, int* height) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Failed to open BMP file");
        return NULL;
    }
    BMPHeader header;
    if (fread(&header, sizeof(BMPHeader), 1, f) != 1) {
        fclose(f);
        fprintf(stderr, "Failed to read BMP header\n");
        return NULL;
    }
    if (header.magic[0] != 'B' || header.magic[1] != 'M') {
        fclose(f);
        fprintf(stderr, "Not a BMP file\n");
        return NULL;
    }
    *width = header.width;
    *height = header.height;
    // Each row is padded to a multiple of 4 bytes.
    int rowSize = ((*width * 3 + 3) / 4) * 4;
    int dataSize = rowSize * (*height);
    uint8_t *data24 = malloc(dataSize);
    if (!data24) {
        fclose(f);
        perror("malloc");
        return NULL;
    }
    fseek(f, header.dataOffset, SEEK_SET);
    if (fread(data24, 1, dataSize, f) != (size_t)dataSize) {
        fclose(f);
        free(data24);
        fprintf(stderr, "Failed to read BMP pixel data\n");
        return NULL;
    }
    fclose(f);
    // BMP stores pixels bottom-up; flip them vertically.
    uint8_t *flipped = malloc(dataSize);
    if (!flipped) { free(data24); return NULL; }
    for (int y = 0; y < *height; y++) {
        memcpy(flipped + y * rowSize, data24 + ((*height - 1 - y) * rowSize), rowSize);
    }
    free(data24);
    // Convert 24-bit BGR to 32-bit ARGB (alpha=0xFF).
    uint32_t *img32 = malloc((*width) * (*height) * sizeof(uint32_t));
    if (!img32) { free(flipped); return NULL; }
    for (int y = 0; y < *height; y++) {
        for (int x = 0; x < *width; x++) {
            int srcIndex = y * rowSize + x * 3;
            uint8_t b = flipped[srcIndex];
            uint8_t g = flipped[srcIndex + 1];
            uint8_t r = flipped[srcIndex + 2];
            img32[y * (*width) + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
    free(flipped);
    return img32;
}

// ------------------------------
// Unified image loader based on extension
// ------------------------------
uint32_t* load_image(const char *filename, int* width, int* height) {
    const char *ext = strrchr(filename, '.');
    if (!ext) {
        fprintf(stderr, "No file extension found.\n");
        return NULL;
    }
    // Compare case-insensitively.
    if (strcasecmp(ext, ".bmp") == 0) {
        return load_bmp_image(filename, width, height);
    } else {
        fprintf(stderr, "Unsupported file format: %s\n", ext);
        return NULL;
    }
}

// ------------------------------
// Main X11 Application
// ------------------------------
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image_file>\n", argv[0]);
        return 1;
    }
    int width, height;
    uint32_t *imgData = load_image(argv[1], &width, &height);
    if (!imgData) {
        fprintf(stderr, "Failed to load image: %s\n", argv[1]);
        return 1;
    }

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Unable to open X display\n");
        free(imgData);
        return 1;
    }
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Window win = XCreateSimpleWindow(dpy, root, 0, 0, width, height, 1,
                                     BlackPixel(dpy, screen), WhitePixel(dpy, screen));

    XStoreName(dpy, win, argv[1]);
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(dpy, win);

    // Create XImage from the pixel data
    XImage *ximage = XCreateImage(dpy, DefaultVisual(dpy, screen), 24, ZPixmap, 0, (char *)imgData,
                                  width, height, 32, 0);

    for (;;) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
            XPutImage(dpy, win, DefaultGC(dpy, screen), ximage, 0, 0, 0, 0, width, height);
        }
        else if (ev.type == KeyPress) {
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            if (keysym == XK_Q) {
                break;
            }
        } 
        else if (ev.type == DestroyNotify || ev.type == ClientMessage) {
            break;
        }
    }

    // Clean up
    XDestroyImage(ximage);
    XCloseDisplay(dpy);
    free(imgData);

    return 0;
}
