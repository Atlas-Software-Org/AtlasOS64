#include "PCI.h"

static uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11)
           | ((uint32_t)function << 8) | (offset & 0xFC);
}

uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    outl(0xCF8, pci_config_address(bus, device, function, offset));
    return (inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF;
}

uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    outl(0xCF8, pci_config_address(bus, device, function, offset));
    return (inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF;
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_word(bus, device, function, 0x00);
}

uint16_t pci_get_device_id(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_word(bus, device, function, 0x02);
}

uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_byte(bus, device, function, 0x0E);
}

uint8_t pci_get_class(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_byte(bus, device, function, 0x0B);
}

uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_byte(bus, device, function, 0x0A);
}

uint8_t pci_get_irq_line(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_byte(bus, device, function, 0x3C);
}

uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function) {
    return pci_read_byte(bus, device, function, 0x19);
}

int pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t* out_bus, uint8_t* out_device, uint8_t* out_function) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint16_t v = pci_get_vendor_id(bus, device, function);
                if (v == 0xFFFF) continue;
                uint16_t d = pci_get_device_id(bus, device, function);
                if (v == vendor_id && d == device_id) {
                    if (out_bus) *out_bus = bus;
                    if (out_device) *out_device = device;
                    if (out_function) *out_function = function;
                    return 1;
                }
                if (function == 0 && !(pci_get_header_type(bus, device, function) & 0x80)) break;
            }
        }
    }
    return 0;
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) |
                       ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) |
                       (offset & 0xFC);
    
    outl(0xCF8, address);

    uint32_t data = inl(0xCFC);

    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

uint32_t pciConfigReadLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

volatile uint32_t *GetE1000MMIOBaseAddr(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t bar0 = pciConfigReadLong(bus, dev, func, 0x10);
    return (volatile uint32_t *)(uintptr_t)(bar0 & ~0xF);
}

uint32_t pci_get_bar0(uint32_t bus, uint32_t device, uint32_t function) {
    uint32_t bar0 = pciConfigReadLong(bus, device, function, 0x10);
    return bar0;
}

uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address;
    address = (uint32_t)(
        ((uint32_t)1 << 31)             |
        ((uint32_t)bus << 16)           |
        ((uint32_t)device << 11)        |
        ((uint32_t)function << 8)       |
        (offset & 0xFC)
    );

    outl(0xCF8, address);
    return inl(0xCFC);
}

int pci_find_device_class(uint8_t class_code, uint8_t subclass, pci_device_t* out) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t config = pci_config_read_dword(bus, device, function, 0x00); // Read Vendor ID and Device ID
                uint16_t vendor_id = (uint16_t)(config & 0xFFFF);
                uint16_t device_id = (uint16_t)((config >> 16) & 0xFFFF);
                
                // Skip if the device is not present
                if (vendor_id == 0xFFFF) continue;

                // Check class and subclass
                config = pci_config_read_dword(bus, device, function, 0x08); // Read Class Code and Subclass
                uint8_t pci_class = (uint8_t)(config >> 24);
                uint8_t pci_subclass = (uint8_t)((config >> 16) & 0xFF);

                if (pci_class == class_code && pci_subclass == subclass) {
                    out->bus = bus;
                    out->device = device;
                    out->function = function;
                    out->vendor_id = vendor_id;
                    out->device_id = device_id;
                    
                    // Read ProgIF (byte 0x09)
                    out->prog_if = (uint8_t)(pci_config_read_dword(bus, device, function, 0x09) & 0xFF);

                    // Read the BARs (0x10 to 0x24)
                    for (int i = 0; i < 6; i++) {
                        out->bar[i] = pci_config_read_dword(bus, device, function, 0x10 + (i * 4));
                    }
                    return 1;  // Device found
                }
            }
        }
    }
    return 0;  // Device not found
}

uint32_t pci_read_bar(void *baseAddress, uint8_t BAR) {
    if (BAR > 5) {
        // BAR can only be from 0 to 5
        return 0;
    }

    // The BARs are located at offsets 0x10, 0x14, ..., 0x24 (4 bytes each)
    uint32_t barOffset = 0x10 + (BAR * 4);
    
    // Read the BAR value at the given offset in the PCI config space
    uint32_t barValue = *((volatile uint32_t *)(baseAddress + barOffset));

    // If the BAR address is 32-bit, return the base value
    if ((barValue & 0x1) == 0) {
        return barValue;
    }
    // For 64-bit BARs, read the next 32 bits and combine them with the first 32 bits
    else {
        uint32_t highBarValue = *((volatile uint32_t *)(baseAddress + barOffset + 4));
        return (highBarValue << 32) | (barValue & 0xFFFFFFF0);
    }
}