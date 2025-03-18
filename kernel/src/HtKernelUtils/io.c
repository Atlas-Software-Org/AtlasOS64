#include "io.h"
#include <stdint.h>

// outb: Write a byte to the specified port
void outb(uint16_t port, uint8_t byte) {
    asm volatile ("outb %0, %1" : : "a"(byte), "dN"(port));
}

// inb: Read a byte from the specified port
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

// outw: Write a word (2 bytes) to the specified port
void outw(uint16_t port, uint16_t word) {
    asm volatile ("outw %0, %1" : : "a"(word), "dN"(port));
}

// inw: Read a word (2 bytes) from the specified port
uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

// outl: Write a double word (4 bytes) to the specified port
void outl(uint16_t port, uint32_t dword) {
    asm volatile ("outl %0, %1" : : "a"(dword), "dN"(port));
}

// inl: Read a double word (4 bytes) from the specified port
uint32_t inl(uint16_t port) {
    uint32_t result;
    asm volatile ("inl %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void IOWait() {
    outb(0x80, 0x00);
}

/**
 * Reads a series of 16-bit words from an I/O port into memory.
 *
 * @param port The I/O port to read from.
 * @param addr The memory buffer to store data.
 * @param count The number of 16-bit words to read.
 */
inline void insw(uint16_t port, void* addr, int count) {
    asm volatile ("rep insw"
                  : "+D"(addr), "+c"(count) // Destination address and counter
                  : "d"(port)               // Source port
                  : "memory");              // Clobbers memory
}

/**
 * Writes a series of 16-bit words from memory to an I/O port.
 *
 * @param port The I/O port to write to.
 * @param addr The memory buffer containing the data.
 * @param count The number of 16-bit words to write.
 */
inline void outsw(uint16_t port, const void* addr, int count) {
    asm volatile ("rep outsw"
                  : "+S"(addr), "+c"(count) // Source address and counter
                  : "d"(port));             // Destination port
}