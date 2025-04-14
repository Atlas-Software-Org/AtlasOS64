#include "RTL8139Dev.h"
#include <IDT/idt.h>
#include <HtKernelUtils/debug.h>

__attribute__((interrupt)) void RTL8139_Hndlr(int* __unused) {
    e9debugkf("RTL8139: Notification recieved\n\r");

    PIC_sendEOI(11);
}