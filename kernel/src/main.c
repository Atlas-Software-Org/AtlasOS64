#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdarg.h>
#include <HtKernelUtils/debug.h>
#include <HtKernelUtils/io.h>
#include <gpx1.h>
#include <memory/paging.h>
#include <memory/heap.h>

#include <TTY/printf/printf.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>
#include <PCI/PCI.h>

// Interrupt Drivers
#include <InterruptDescriptors/Drivers/PIT/PIT.h>
#include <InterruptDescriptors/Drivers/PIT/Scheduler/Sched.h>
#include <InterruptDescriptors/Drivers/KeyboardDev/KbdDev.h>
#include <InterruptDescriptors/Drivers/MouseDev/MouseDev.h>
#include <InterruptDescriptors/Drivers/Net/RTL8139/RTL8139Dev.h>

// Misc Drivers

#include <RD/RamDisk.h>
#include <TTY/AtsTty.h>
#include <Executables/Apxf.h>
#include <Notification/notify.h>
#include <Drivers/ATA/ATA.h>
#include <Drivers/FAT32/Fat32.h>

// PCI Drivers
#include <PCI/Drivers/RTL8139/rtl8139.h>

#include <InterruptDescriptors/Syscall.h>

#include <Language/language.h>

#pragma region LIMINE
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;)
    {
#if defined(__x86_64__)
        asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm("wfi");
#elif defined(__loongarch64)
        asm("idle 0");
#endif
    }
}
#pragma endregion

int EnabledNet = 0;

void kmain(void) {
#pragma region Step1K
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    if (module_request.response == NULL || module_request.response->module_count < 6) {
        hcf();
    }
    if (rsdp_request.response == NULL) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    struct limine_file *initrd = module_request.response->modules[0];   // INITRD
    struct limine_file *fontFile = module_request.response->modules[1]; // ZAP LIGHT 16 FONT
    struct limine_file *fontFileAr = module_request.response->modules[2]; // ARABIC FONT
    struct limine_file *osCfg = module_request.response->modules[3];    // OS CFG
    struct limine_file *AtlasOS256_LoadImg = module_request.response->modules[4]; // AtlasOS256.bmp - Startup logo    // Retrieve memory map from Limine
    struct limine_file *elfBin = module_request.response->modules[5];

    struct limine_memmap_response *LimineMemMapRecv = memmap_request.response;
    SetLmMap(LimineMemMapRecv);

    if (((uint8_t *)osCfg->address)[0] == '0') {
        e9 = 0x80;
    }

    if (((uint8_t *)osCfg->address)[1] == '1') {
        EnabledNet = true;
    }

    switch (((uint8_t *)osCfg->address)[2]) {
        case '0':
            SetLanguage(ENGLISH);
            break;
        case '1':
            SetLanguage(FRENCH);
            break;
        case '2':
            SetLanguage(GERMAN);
            break;
        case '3':
            SetLanguage(SPANISH);
            break;
        default:
            SetLanguage(ENGLISH);  // Default to English if value is invalid
            break;
    }

    void* UserNameStartupStart = (void*)((uint64_t)osCfg->address+3);
    SetUsername((char*)UserNameStartupStart);

    PSF1_HEADER *fontHeader = (PSF1_HEADER *)fontFile->address;
    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1) {
        hcf();
    }

    PSF1_HEADER *fontHeaderAr = (PSF1_HEADER *)fontFileAr->address;
    if (fontHeaderAr->magic[0] != PSF1_MAGIC0 || fontHeaderAr->magic[1] != PSF1_MAGIC1) {
        hcf();
    }

    uint64_t glyphBufferSize = fontHeader->charsize * 256;
    if (fontHeader->mode == 1) {
        glyphBufferSize = fontHeader->charsize * 512;
    }
    
    uint64_t glyphBufferSizeAr = fontHeaderAr->charsize * 256;
    if (fontHeaderAr->mode == 1) {
        glyphBufferSizeAr = fontHeaderAr->charsize * 512;
    }

    void *glyphBuffer = (void *)((uint8_t *)fontFile->address + sizeof(PSF1_HEADER));
    
    void *glyphBufferAr = (void *)((uint8_t *)fontFileAr->address + sizeof(PSF1_HEADER));

    PSF1_FONT finishedFont_ins;
    PSF1_FONT *finishedFont = &finishedFont_ins;
    finishedFont->psf1_Header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;
    
    PSF1_FONT finishedFontAr_ins;
    PSF1_FONT *finishedFontAr = &finishedFontAr_ins;
    finishedFontAr->psf1_Header = fontHeaderAr;
    finishedFontAr->glyphBuffer = glyphBufferAr;

    InitGfx(framebuffer);
    main_psf1_font = finishedFont;
    mainAr_psf1_font = finishedFontAr;
    if (finishedFont == NULL) {
        hcf();
    }
    else {
        main_psf1_font = finishedFont;
    }

    DrawBmp(AtlasOS256_LoadImg->address, AtlasOS256_LoadImg->size, GetFb()->width/2-128, GetFb()->height/2-128);

    ClearColor = 0x000000;

    pool_init();
    heap_init();

    InitRD();

    g_DiskDriver = InitAtaDriver();

    if (!FAT_Init()) {
        e9debugkf("FAT Init failed\n");
    }

#pragma endregion

    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDescriptor);

    for (uint32_t i = 0; i < 2500; i++) {
        asm volatile ("nop;nop;nop;nop;nop");
    }

    if (!EnabledNet) {
        goto SkipFindRtl;
    }

    uint8_t rtl8139_bus, rtl8139_device, rtl8139_function, rtl8139_irq;
    if (pci_find_device(0x10EC, 0x8139, &rtl8139_bus, &rtl8139_device, &rtl8139_function) && EnabledNet) {
        rtl8139_irq = pci_get_irq_line(rtl8139_bus, rtl8139_device, rtl8139_function);
        uint8_t mac[6];
        rtl8139_get_mac(mac);
    
        printf(" [ RTL8139 ] MAC  Address: %x:%x:%x:%x:%x:%x\n\r", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        printf(" [ RTL8139 ] RTL8139 found at %d:%d:%d with IRQ %d\n\rmake -C kernel", rtl8139_bus, rtl8139_device, rtl8139_function, rtl8139_irq);
        
        rtl8139_init(pci_get_bar0(rtl8139_bus, rtl8139_device, rtl8139_function), rtl8139_bus, rtl8139_device, rtl8139_function);
    } else {
        if (EnabledNet) {
            printf(" [ RTL8139 ! ] RTL8139 not found\n\r");
        } else {
            printf(" [ RTL8139 ! ] User disabled networking\n\r");
        }
    }

    goto SkipFindRtl;

    uint8_t ahci_bus, ahci_device, ahci_function;
    pci_device_t AhciDev;
    if (!pci_find_device_class(0x01, 0x06, &AhciDev)) {
        ClearScreenColor(0x000000);
        int text_width = sizeof("Could not find AHCI device on current hardware!") - 1;  // Subtract 1 to exclude the null terminator
        int text_height = 16;  // Assuming the font height is 16 pixels, adjust if needed
        int x_pos = (GetFb()->width - text_width * 8) / 2;  // Assuming each character is 8px wide
        int y_pos = (GetFb()->height - text_height) / 2;  // Center vertically
        FontPutStr("Could not find AHCI device on current hardware!", x_pos, y_pos, 0xFFFF0000); // Red
        asm volatile ("cli");
        hcf();
    }

    SkipFindRtl:

    asm volatile("cli");

    PIC_remap(0x20, 0x28);

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    outb(0x20, 0x20);
    outb(0xA0, 0x20);

    extern __attribute__((interrupt)) void SyscallInt_Handler(int* __unused);
    
    idt_set_descriptor(0x20, &PITInt_Hndlr, 0x8E);
    idt_set_descriptor(0x21, &KeyboardInt_Hndlr, 0x8E);
    idt_set_descriptor(0x2C, &MouseInt_Hndlr, 0x8E);
    idt_set_descriptor(0x80, &SyscallInt_Handler, 0xEE);

    init_exceptions();
    idtr_t idtr_ = idt_init();

    uint16_t limit;
    uint64_t base;
    asm volatile("sidt %0" : "=m"(idtr_));

    outb(0x21, 0b11111000);
    outb(0xA1, 0b11011111);
    IRQ_clear_mask(0);
    IRQ_clear_mask(1);
    IRQ_clear_mask(2);
    if (EnabledNet) {
        IRQ_clear_mask(rtl8139_irq);
    }
    IRQ_clear_mask(12);
    
    InitPS2Mouse();
    InitPitTimer();

    tty_init();

    ClearScreenColor(0x000000);

    asm volatile("sti");

    Window* window = CreateWindow("Hello, World!\0", 640, 400, NULL);

    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 400; j++) {
            WinPutPx(window, i, j, 0x282828);
        }
    }

    for (int i = 0; i < 400; i++) {
        for (int j = 0; j < 4; j++) {
            WinPutPx(window, 125+j, i, 0x000000);
        }
    }

    WinPutStr(window, 2, 2, "Hello!", 0xFFFFFF);

    window->Repaint(window);

    hcf();
}
