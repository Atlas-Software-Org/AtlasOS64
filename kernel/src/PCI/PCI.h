#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stddef.h>
#include <HtKernelUtils/io.h>

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t bar[6];
    uint8_t prog_if;  // Programming Interface (ProgIF)
} pci_device_t;

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_get_device_id(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_class(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_irq_line(uint8_t bus, uint8_t device, uint8_t function);
uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pciConfigReadLong(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
int pci_find_device(uint16_t vendor_id, uint16_t device_id, uint8_t* out_bus, uint8_t* out_device, uint8_t* out_function);
volatile uint32_t *GetE1000MMIOBaseAddr(uint8_t bus, uint8_t device, uint8_t function);
uint32_t pci_get_bar0(uint32_t bus, uint32_t device, uint32_t function);
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
int pci_find_device_class(uint8_t class_code, uint8_t subclass, pci_device_t* out);
uint32_t pci_read_bar(void *baseAddress, uint8_t BAR);

#endif
