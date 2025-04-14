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