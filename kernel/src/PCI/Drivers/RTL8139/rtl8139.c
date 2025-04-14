#include "rtl8139.h"
#include <HtKernelUtils/debug.h>
#include <HtKernelUtils/io.h>
#include <paging/paging.h>
#include <mem/mem.h>
#include <PCI/PCI.h>
#include <IDT/idt.h>
#include <InterruptDescriptors/Drivers/Net/RTL8139/RTL8139Dev.h>
#include <Regs.h>

#define RX_BUF_SIZE 8192
#define RX_READ_POINTER_MASK (RX_BUF_SIZE - 1)

#define RTL8139_ISR_ROK  (1 << 0)
#define RTL8139_ISR_TOK  (1 << 2)

#define IP_HEADER_SIZE 20
#define TCP_HEADER_SIZE 20
#define ETH_HEADER_SIZE 14

uint8_t TSAD_array[4] = { 0x20, 0x24, 0x28, 0x2C };
uint8_t TSD_array[4] = { 0x10, 0x14, 0x18, 0x1C };

uint32_t io_base = 0;
uint8_t mac[6] = { 0 };
uint8_t* rx_buffer = NULL;
uint32_t current_packet_ptr = 0;
uint8_t tx_cur = 0;

uint8_t rtl8139_bus, rtl8139_dev, rtl8139_func;

// Dummy user callback to process received packets (you can modify it to suit your needs)
void user_receive_callback(uint8_t* packet, uint16_t length) {
    // For now, just print the packet contents
    e9debugkf("Received packet of length %d: \n\r", length);

    for (int i = 0; i < length; i++) {
        e9debugkf("%02x ", packet[i]);
    }

    e9debugkf("\n\r");
}

void rtl8139_get_mac() {
    for (int i = 0; i < 6; i++) {
        mac[i] = inb(io_base + i);
    }
}

void* rtl8139_init(uint32_t base_address, uint8_t bus, uint8_t dev, uint8_t func) {
    rtl8139_bus = bus;
    rtl8139_dev = dev;
    rtl8139_func = func;

    io_base = base_address & ~0x3;

    // Removed ID check, as per your request
    rx_buffer = (uint8_t*) page_alloc_n((RX_BUF_SIZE + 0xFFF) / 0x1000 + 1);
    memset(rx_buffer, 0, RX_BUF_SIZE);
    
    e9debugkf("rtl8139: I/O base: %x\n\r", io_base);

    outb(io_base + 0x52, 0x00); // Disable config register 1
    outb(io_base + 0x37, 0x10); // Reset
    while (inb(io_base + 0x37) & 0x10) {
        __asm__ __volatile__("pause" ::: "memory");
    }

    outl(io_base + 0x30, (uintptr_t)rx_buffer);
    outw(io_base + 0x3C, 0x0005); // Enable TOK and ROK interrupts

    outl(io_base + 0x44, 0xf | (1 << 7)); // WRAP + RX config
    outb(io_base + 0x37, 0x0C); // Enable RX and TX

    rtl8139_get_mac();
    e9debugkf("rtl8139: mac: %x:%x:%x:%x:%x:%x\n\r", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    idt_set_descriptor(0x20 + pci_get_irq_line(bus, dev, func), &rtl8139_handler, 0x8E);

    return NULL;
}

void rtl8139_send(uint8_t* data, uint32_t size) {
    asm volatile("cli");

    outl(io_base + TSAD_array[tx_cur], (uint32_t)data);
    outl(io_base + TSD_array[tx_cur], size);

    tx_cur = (tx_cur + 1) & 0x3;

    asm volatile("sti");
}

__attribute__((interrupt)) void rtl8139_handler(int* __unused) {
    (void)__unused;

    uint16_t status = inw(io_base + 0x3E);
    outw(io_base + 0x3E, status); // Acknowledge handled interrupts

    if (status & RTL8139_ISR_TOK) {
        e9debugkf("rtl8139: Packet was sent\n\r");
    }

    if (status & RTL8139_ISR_ROK) {
        rtl8139_receive();
    }

    PIC_sendEOI(pci_get_irq_line(rtl8139_bus, rtl8139_dev, rtl8139_func));
}

void rtl8139_receive() {
    uint8_t* pkt = rx_buffer + current_packet_ptr;
    uint16_t status = *(uint16_t*)pkt;
    uint16_t len = *(uint16_t*)(pkt + 2);

    if (len < 64 || len > 1518) return; // Skip invalid packet sizes

    e9debugkf("rtl8139: Received packet of length %d\n\r", len);

    uint8_t* payload = pkt + 4;
    
    // Call the user-defined callback to process the packet
    user_receive_callback(payload, len - 4);

    current_packet_ptr = (current_packet_ptr + len + 4 + 3) & RX_READ_POINTER_MASK;

    outw(io_base + 0x38, current_packet_ptr - 0x10);
}

void TCPIPWrapper(uint8_t* data, uint32_t size, uint8_t* dest_ip, uint16_t dest_port) {
    uint8_t ip_header[IP_HEADER_SIZE] = {0};
    uint8_t tcp_header[TCP_HEADER_SIZE] = {0};
    
    // Fill in the IP header (assuming simple IPv4)
    ip_header[0] = 0x45; // IPv4 + header length (20 bytes)
    ip_header[1] = 0x00; // Type of service
    ip_header[2] = (size + TCP_HEADER_SIZE); // Total length (IP + TCP + payload)
    ip_header[3] = 0x00; // Total length (IP + TCP + payload)
    ip_header[8] = 64;    // TTL
    ip_header[9] = 6;     // Protocol: TCP

    // Source IP (for now, set to a placeholder value)
    ip_header[12] = 192;
    ip_header[13] = 168;
    ip_header[14] = 1;
    ip_header[15] = 100;

    // Destination IP (from function argument)
    ip_header[16] = dest_ip[0];
    ip_header[17] = dest_ip[1];
    ip_header[18] = dest_ip[2];
    ip_header[19] = dest_ip[3];

    // Fill in the TCP header
    tcp_header[0] = 0x00; // Source port (will be set later)
    tcp_header[1] = 0x50; // Destination port (e.g., 80 for HTTP)
    tcp_header[2] = 0x00; // Sequence number (set to 0 for simplicity)
    tcp_header[3] = 0x00;

    tcp_header[4] = 0x00; // Acknowledgment number (set to 0 for simplicity)
    tcp_header[5] = 0x00;

    tcp_header[12] = 0x50; // Data offset + reserved + flags
    tcp_header[13] = 0x02; // SYN flag

    // Use page_alloc_n to allocate memory for the full packet
    uint8_t* full_packet = (uint8_t*) page_alloc_n((IP_HEADER_SIZE + TCP_HEADER_SIZE + size + 0xFFF) / 0x1000 + 1);

    if (!full_packet) {
        e9debugkf("TCPIPWrapper: Memory allocation failed\n\r");
        return;
    }

    // Copy the IP header, TCP header, and data into the allocated memory
    memcpy(full_packet, ip_header, IP_HEADER_SIZE);
    memcpy(full_packet + IP_HEADER_SIZE, tcp_header, TCP_HEADER_SIZE);
    memcpy(full_packet + IP_HEADER_SIZE + TCP_HEADER_SIZE, data, size);

    // Send the packet using rtl8139_send
    rtl8139_send(full_packet, IP_HEADER_SIZE + TCP_HEADER_SIZE + size);

    // Use page_free_n to free the allocated memory
    page_free_n((void*)full_packet, (IP_HEADER_SIZE + TCP_HEADER_SIZE + size + 0xFFF) / 0x1000 + 1);
}


// Mock Test Data
uint8_t test_data[] = "Hello, world! This is a test packet.";
uint8_t test_dest_ip[] = {192, 168, 1, 100}; // Destination IP address (example: 192.168.1.100)
uint16_t test_dest_port = 80; // Destination port (example: 80 for HTTP)

void test_TCPIPWrapper() {
    e9debugkf("Running test_TCPIPWrapper...\n\r");

    // Initialize RTL8139 network interface (mock base address and PCI info)
    uint32_t base_address = 0x6000; // Example I/O base address
    uint8_t bus = 0, dev = 0, func = 0; // Mock PCI bus, device, function
    rtl8139_init(base_address, bus, dev, func);

    // Test the TCP/IP wrapper
    TCPIPWrapper(test_data, sizeof(test_data) - 1, test_dest_ip, test_dest_port);
    
    e9debugkf("Test completed.\n\r");
}