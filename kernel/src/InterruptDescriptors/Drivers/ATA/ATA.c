#include "ATA.h"

void ata_wait_bsy() {
    while (inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_BSY);
}

void ata_wait_drq() {
    while (!(inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_DRQ));
}

void ata_select_drive(uint8_t drive) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | (drive << 4));
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();
    ata_select_drive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }
}

void ata_write_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();
    ata_select_drive(0);

    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, 1);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_IO + ATA_REG_DATA, ((uint16_t*)buffer)[i]);
    }

    ata_wait_bsy();
}

char disk_buffer[512];

void ata_identify_name(char** model_name) {
    // Select Master drive
    outb(0x1F6, 0xA0); // Select Master
    outb(0x1F2, 0);    
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC); // IDENTIFY command

    if (inb(0x1F7) == 0) {
        *model_name = "No drive detected";
        return;
    }

    // Wait for BSY (busy) flag to clear
    int counter = 0;
    while (inb(0x1F7) & 0x80) {
        counter+=1;
        if (counter >= 500000) {
            *model_name = "No drive detected! Timeout";
        }
    }

    uint16_t disk_buffer[256]; // 512 bytes (ATA IDENTIFY response)
    insw(0x1F0, disk_buffer, 256);

    // The model name is stored at bytes 54-93 (27 words, 54 bytes)
    *model_name = (char*)&disk_buffer[27];

    // Fix byte-swapping issue (swap every 2 bytes)
    for (int i = 0; i < 40; i += 2) {
        char tmp = (*model_name)[i];
        (*model_name)[i] = (*model_name)[i + 1];
        (*model_name)[i + 1] = tmp;
    }

    // Ensure null termination
    (*model_name)[40] = '\0';  // Model name is 40 characters long
}
