#include "PCI.h"

int PciFindAhci(PciDevice *device) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device_num = 0; device_num < 32; device_num++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t config_address = 0x80000000 | (bus << 16) | (device_num << 11) | (func << 8);
                uint32_t config_data = PciReadConfig(config_address);

                uint16_t vendor_id = (config_data & 0xFFFF);
                uint16_t device_id = ((config_data >> 16) & 0xFFFF);

                if (vendor_id == AHCI_VENDOR_ID) {
                    uint8_t class_code = (config_data >> 24) & 0xFF;
                    uint8_t subclass_code = (config_data >> 20) & 0xFF;

                    if (class_code == AHCI_CLASS_CODE && subclass_code == AHCI_SUBCLASS_CODE) {
                        device->vendor_id = vendor_id;
                        device->device_id = device_id;
                        device->bus = bus;
                        device->device = device_num;
                        device->function = func;
                        device->class_code = class_code;
                        device->subclass_code = subclass_code;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void PciGetBars(PciDevice *device, PciBars *bars) {
    for (int i = 0; i < 6; i++) {
        uint32_t config_address = 0x80000000 | (device->bus << 16) | (device->device << 11) | (device->function << 8) | (0x10 + i * 4);
        uint32_t config_data = PciReadConfig(config_address);

        switch (i) {
            case 0: bars->BAR0 = config_data; break;
            case 1: bars->BAR1 = config_data; break;
            case 2: bars->BAR2 = config_data; break;
            case 3: bars->BAR3 = config_data; break;
            case 4: bars->BAR4 = config_data; break;
            case 5: bars->BAR5 = config_data; break;
        }

        if (config_data & 1) {
            e9debugkf("BAR%d: I/O Port Address\n", i);
        } else {
            e9debugkf("BAR%d: MMIO Address\n", i);
        }

        if (config_data & 0x04) {
            e9debugkf("BAR%d is 64-bit address\n", i);
        }
    }
}

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

void PciWriteConfigAddress(uint32_t address) {
    outl(PCI_CONFIG_ADDRESS, address);
}

uint32_t PciReadConfigData(void) {
    return inl(PCI_CONFIG_DATA);
}

uint32_t PciReadConfig(uint32_t config_address) {
    PciWriteConfigAddress(config_address);
    return PciReadConfigData();
}
