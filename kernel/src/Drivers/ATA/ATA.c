#include "ATA.h"

DiskDriver* g_DiskDriver;

void AtaWaitBsy() {
    while (inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_BSY);
}

void AtaWaitDrq() {
    while (!(inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_DRQ));
}

void AtaSelectDrive(uint8_t drive) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | (drive << 4));
}

void AtaReadSect(uint32_t lba, uint8_t* buffer) {
    AtaWaitBsy();
    AtaSelectDrive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    AtaWaitBsy();
    AtaWaitDrq();

    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }
}

void AtaWriteSect(uint32_t lba, uint8_t* buffer) {
    AtaWaitBsy();
    AtaSelectDrive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    AtaWaitBsy();
    AtaWaitDrq();

    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_IO + ATA_REG_DATA, ((uint16_t*)buffer)[i]);
    }

    AtaWaitBsy();
}

uint32_t ATA_GetTotalSectors(void) {
    uint16_t identify_data[256];

    AtaWaitBsy();

    outb(ATA_REG_COMMAND, 0xEC);

    AtaWaitBsy();

    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_REG_DATA);
    }

    uint32_t total_sectors = identify_data[61] | ((uint32_t)identify_data[60] << 16);
    
    return total_sectors;
}

void DiskDriver_WriteSect(uint32_t lba, uint8_t buffer[512]) {
    for (int i = 0; i < 512; i++) buffer[i] = 0xAB;

    AtaWriteSect(lba, buffer);
}

void DiskDriver_ReadSect(uint32_t lba, uint8_t buffer[512]) {
    AtaReadSect(lba, buffer);
}

uint32_t DiskDriver_GetTotalSects() {
    return ATA_GetTotalSectors();
}

DiskDriver* InitAtaDriver() {
    static DiskDriver __disk_driver;
    //__disk_driver.IdentifyDriveName = DiskDriver_Identify;
    __disk_driver.ReadSect = DiskDriver_ReadSect;
    __disk_driver.WriteSect = DiskDriver_WriteSect;
    __disk_driver.GetTotalSectors = DiskDriver_GetTotalSects;
    return &__disk_driver;
}
