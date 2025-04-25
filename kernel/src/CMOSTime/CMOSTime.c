#include "CMOSTime.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static inline uint8_t cmos_read(uint8_t reg) {
    __asm__ volatile ("cli");
    *((volatile uint8_t*)CMOS_ADDRESS) = reg;
    uint8_t value = *((volatile uint8_t*)CMOS_DATA);
    __asm__ volatile ("sti");
    return value;
}

static const char* months[] = {
    "Err", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static uint8_t bcd_to_bin(uint8_t val) {
    return ((val / 16) * 10) + (val & 0x0F);
}

char* CmosGetTime() {
    static char buf[24];

    uint8_t second = bcd_to_bin(cmos_read(0x00));
    uint8_t minute = bcd_to_bin(cmos_read(0x02));
    uint8_t hour   = bcd_to_bin(cmos_read(0x04));
    uint8_t day    = bcd_to_bin(cmos_read(0x07));
    uint8_t month  = bcd_to_bin(cmos_read(0x08));

    const char* ampm = "AM";
    if (hour == 0) {
        hour = 12;
        ampm = "AM";
    } else if (hour == 12) {
        ampm = "PM";
    } else if (hour > 12) {
        hour -= 12;
        ampm = "PM";
    }

    if (month == 0 || month > 12) month = 1;

    int i = 0;
    while (months[month][i]) {
        buf[i] = months[month][i];
        i++;
    }
    buf[i++] = ' ';
    if (day >= 10) {
        buf[i++] = '0' + (day / 10);
    }
    buf[i++] = '0' + (day % 10);
    buf[i++] = ' ';
    if (hour >= 10) {
        buf[i++] = '0' + (hour / 10);
    }
    buf[i++] = '0' + (hour % 10);
    buf[i++] = ':';
    buf[i++] = '0' + (minute / 10);
    buf[i++] = '0' + (minute % 10);
    buf[i++] = ' ';
    buf[i++] = ampm[0];
    buf[i++] = ampm[1];
    buf[i++] = '\0';

    return buf;
}