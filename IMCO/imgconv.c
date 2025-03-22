#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------
// BMP Header Structure (14-byte file header + 40-byte BITMAPINFOHEADER)
// ---------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct {
    uint8_t  magic[2];       // Should be "BM"
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;     // Offset to pixel data
    uint32_t headerSize;     // Should be 40
    int32_t  width;
    int32_t  height;
    uint16_t planes;         // Must be 1
    uint16_t bitsPerPixel;   // Should be 24
    uint32_t compression;    // 0 = BI_RGB (uncompressed)
    uint32_t imageSize;      // May be 0 for uncompressed images
    int32_t  xPixelsPerM;
    int32_t  yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t importantColors;
} BMPHeader;
#pragma pack(pop)

// ---------------------------------------------------------------------
// TGA Header Structure (18 bytes)
// ---------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct {
    uint8_t  idLength;         // 0
    uint8_t  colorMapType;     // 0
    uint8_t  imageType;        // 2 (uncompressed true-color)
    uint16_t colorMapOrigin;   // 0
    uint16_t colorMapLength;   // 0
    uint8_t  colorMapDepth;    // 0
    uint16_t xOrigin;          // 0
    uint16_t yOrigin;          // 0
    uint16_t width;
    uint16_t height;
    uint8_t  pixelDepth;       // 32
    uint8_t  imageDescriptor;  // 0x20 for top-left origin
} TGAHeader;
#pragma pack(pop)

// ---------------------------------------------------------------------
// GPX1 Header Structure (Graphyx 1 format)
// ---------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct {
    uint8_t  magic[4];         // 0xAB, 0xFF, 0xEA, 0x00
    uint64_t x_len;            // Image width
    uint64_t y_len;            // Image height
    uint8_t  bpp;              // 32
    uint8_t  header_end_mag[2]; // 0xAB, 0xEA
} GPX1Header;
#pragma pack(pop)

// ---------------------------------------------------------------------
// flip_pixels(): Flip BMP pixel data vertically.
// BMP rows are padded to a multiple of 4 bytes.
// ---------------------------------------------------------------------
void flip_pixels(uint8_t *data, int width, int height) {
    int rowSize = ((width * 3 + 3) / 4) * 4;
    uint8_t *temp = malloc(rowSize);
    if (!temp) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int y = 0; y < height / 2; y++) {
        uint8_t *top    = data + y * rowSize;
        uint8_t *bottom = data + (height - 1 - y) * rowSize;
        memcpy(temp, top, rowSize);
        memcpy(top, bottom, rowSize);
        memcpy(bottom, temp, rowSize);
    }
    free(temp);
}

// ---------------------------------------------------------------------
// convert_to_bgra(): Convert 24-bit BMP (BGR, with padded rows) to a new
// 32-bit BGRA buffer. This function is used by both TGA and GPX1 conversion.
// ---------------------------------------------------------------------
uint8_t *convert_to_bgra(const uint8_t *src, int width, int height) {
    int srcRowSize = ((width * 3 + 3) / 4) * 4;
    int dstRowSize = width * 4;
    uint8_t *dst = malloc(height * dstRowSize);
    if (!dst) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int y = 0; y < height; y++) {
        const uint8_t *srcRow = src + y * srcRowSize;
        uint8_t *dstRow = dst + y * dstRowSize;
        for (int x = 0; x < width; x++) {
            dstRow[x * 4 + 0] = srcRow[x * 3 + 0]; // Blue
            dstRow[x * 4 + 1] = srcRow[x * 3 + 1]; // Green
            dstRow[x * 4 + 2] = srcRow[x * 3 + 2]; // Red
            dstRow[x * 4 + 3] = 255;               // Alpha
        }
    }
    return dst;
}

// For GPX1 conversion, use the same BGRA ordering.
uint8_t *convert_to_bgra_for_gpx1(const uint8_t *src, int width, int height) {
    return convert_to_bgra(src, width, height);
}

// ---------------------------------------------------------------------
// read_bmp(): Read a BMP file and return its 24-bit pixel data (with padding).
// Flips the data from BMP's bottom-up order to top-down order.
// ---------------------------------------------------------------------
int read_bmp(const char *filename, uint8_t **pixelData, int *width, int *height) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening BMP file");
        return -1;
    }
    BMPHeader header;
    if (fread(&header, sizeof(BMPHeader), 1, f) != 1) {
        fclose(f);
        fprintf(stderr, "Failed to read BMP header\n");
        return -1;
    }
    if (header.magic[0] != 'B' || header.magic[1] != 'M') {
        fclose(f);
        fprintf(stderr, "Not a valid BMP file\n");
        return -1;
    }
    *width = header.width;
    *height = header.height;
    int rowSize = ((header.width * 3 + 3) / 4) * 4;
    int dataSize = rowSize * header.height;
    uint8_t *data = malloc(dataSize);
    if (!data) {
        perror("malloc");
        fclose(f);
        return -1;
    }
    fseek(f, header.dataOffset, SEEK_SET);
    if (fread(data, 1, dataSize, f) != (size_t)dataSize) {
        fclose(f);
        free(data);
        fprintf(stderr, "Failed to read BMP pixel data\n");
        return -1;
    }
    fclose(f);
    // Flip the BMP data from bottom-up to top-down.
    flip_pixels(data, header.width, header.height);
    *pixelData = data;
    return 0;
}

// ---------------------------------------------------------------------
// write_tga(): Write a TGA file from BMP data by converting to 32-bit BGRA.
// ---------------------------------------------------------------------
int write_tga(const char *filename, const uint8_t *bmpData, int width, int height) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Error opening TGA file for writing");
        return -1;
    }
    TGAHeader header = {0};
    header.imageType = 2;         // Uncompressed true-color
    header.width = width;
    header.height = height;
    header.pixelDepth = 32;
    header.imageDescriptor = 0x20; // Top-left origin
    fwrite(&header, sizeof(TGAHeader), 1, f);
    
    uint8_t *converted = convert_to_bgra(bmpData, width, height);
    if (!converted) {
        fclose(f);
        return -1;
    }
    fwrite(converted, 1, width * height * 4, f);
    free(converted);
    fclose(f);
    return 0;
}

// ---------------------------------------------------------------------
// write_gpx1(): Write a GPX1 file from BMP data by converting to 32-bit BGRA.
// This fixes the hue issue by matching the channel order used in TGA.
// ---------------------------------------------------------------------
int write_gpx1(const char *filename, const uint8_t *bmpData, int width, int height) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Error opening GPX1 file for writing");
        return -1;
    }
    GPX1Header header = {0};
    header.magic[0] = 0xAB;
    header.magic[1] = 0xFF;
    header.magic[2] = 0xEA;
    header.magic[3] = 0x00;
    header.x_len = width;
    header.y_len = height;
    header.bpp = 32;
    header.header_end_mag[0] = 0xAB;
    header.header_end_mag[1] = 0xEA;
    fwrite(&header, sizeof(GPX1Header), 1, f);
    
    uint8_t *converted = convert_to_bgra_for_gpx1(bmpData, width, height);
    if (!converted) {
        fclose(f);
        return -1;
    }
    fwrite(converted, 1, width * height * 4, f);
    free(converted);
    fclose(f);
    return 0;
}

// ---------------------------------------------------------------------
// main(): Convert BMP file to either TGA or GPX1 based on output filename.
// Usage: converter input.bmp output.tga|output.gpx1
// ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.bmp output.tga|output.gpx1\n", argv[0]);
        return EXIT_FAILURE;
    }
    uint8_t *bmpData = NULL;
    int width, height;
    if (read_bmp(argv[1], &bmpData, &width, &height) != 0) {
        return EXIT_FAILURE;
    }
    
    int result = 0;
    if (strstr(argv[2], ".tga") != NULL) {
        result = write_tga(argv[2], bmpData, width, height);
    } else if (strstr(argv[2], ".gpx1") != NULL) {
        result = write_gpx1(argv[2], bmpData, width, height);
    } else {
        fprintf(stderr, "Unsupported output format. Use .tga or .gpx1\n");
        result = -1;
    }
    
    free(bmpData);
    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
