#pragma once

#include <HtKernelUtils/io.h>
#include <HtKernelUtils/debug.h>
#include <stdint.h>
#include <stdbool.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define AHCI_CLASS_CODE     0x01 // Mass Storage Controller
#define AHCI_SUBCLASS_CODE  0x06 // AHCI Controller
#define AHCI_VENDOR_ID      0x8086 // Example: Intel Vendor ID
#define AHCI_DEVICE_ID      0x1C03 // Example: AHCI Device ID (can vary)

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t class_code;
    uint8_t subclass_code;
} PciDevice;

typedef struct {
    uint32_t BAR0; // Base Address Register 0
    uint32_t BAR1; // Base Address Register 1
    uint32_t BAR2; // Base Address Register 2
    uint32_t BAR3; // Base Address Register 3
    uint32_t BAR4; // Base Address Register 4
    uint32_t BAR5; // Base Address Register 5
} PciBars;

int PciFindAhci(PciDevice *device);
void PciGetBars(PciDevice *device, PciBars *bars);
void PciWriteConfigAddress(uint32_t address);
uint32_t PciReadConfigData(void);
uint32_t PciReadConfig(uint32_t config_address);
