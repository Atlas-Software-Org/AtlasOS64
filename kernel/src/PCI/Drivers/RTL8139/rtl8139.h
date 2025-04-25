#ifndef RTL8139_H
#define RTL8139_H

#include <HtKernelUtils/debug.h>
#include <HtKernelUtils/io.h>
#include <memory/paging.h>
#include <mem/mem.h>
#include <PCI/PCI.h>
#include <IDT/idt.h>
#include <InterruptDescriptors/Drivers/Net/RTL8139/RTL8139Dev.h>
#include <Regs.h>

void* rtl8139_init(uint32_t base_address, uint8_t bus, uint8_t dev, uint8_t func);
void rtl8139_send(uint8_t* data, uint32_t size);
x64Regs* rtl8139_interrupt(x64Regs* regs);
void rtl8139_receive();
void rtl8139_get_mac();

__attribute__((interrupt)) void rtl8139_handler(int* __unused);

void test_TCPIPWrapper();

#endif // RTL8139_H
