#include <stdio.h>

int main() {
    unsigned char bytes[] = { 0x48, 0xC7, 0xC0, 0x00, 0x10, 0x00, 0x00, 0xC3 }; 

    // Example: Writing bytes to a file (optional, if needed)
    FILE *file = fopen("output.bin", "wb");
    if (file != NULL) {
        fwrite(bytes, sizeof(bytes), 1, file);
        fclose(file);
    }

    return 0;
}
