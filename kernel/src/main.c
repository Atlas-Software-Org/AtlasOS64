
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdarg.h>
#include <HtKernelUtils/debug.h>
#include <HtKernelUtils/io.h>
#include <gpx1.h>
#include <paging/paging.h>

#include <GDT/gdt.h>
#include <IDT/idt.h>

// Drivers
#include <InterruptDescriptors/Drivers/PIT/PIT.h>
#include <InterruptDescriptors/Drivers/PIT/Scheduler/Sched.h>
#include <InterruptDescriptors/Drivers/KeyboardDev/KbdDev.h>
#include <InterruptDescriptors/Drivers/MouseDev/MouseDev.h>

#include <InterruptDescriptors/Syscall.h>

#pragma region LIMINE
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}
#pragma endregion

void Test1() {
    outb(0xE9, 't');
        e9debugkf("Counted to 1000: 1\n\r");
}

void Test2() {
    outb(0xE9, 't');
        e9debugkf("Counted to 1000: 2\n\r");
}

void Test3() {
    outb(0xE9, 't');
        e9debugkf("Counted to 1000: 3\n\r");
}

void Test4() {
    outb(0xE9, 't');
        e9debugkf("Counted to 1000: 4\n\r");
}

void Test5() {
    outb(0xE9, 't');
        e9debugkf("Counted to 1000: 5\n\r");
}

void kmain(void) {
    #pragma region Step1K
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    if (module_request.response == NULL || module_request.response->module_count < 3) {
        hcf();
    }
    if (rsdp_request.response == NULL) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_file *initrd = module_request.response->modules[0]; // INITRD
    struct limine_file *fontFile = module_request.response->modules[1]; // ZAP LIGHT 16 FONT
    struct limine_file *osCfg = module_request.response->modules[2]; // OS CFG
    if (((uint8_t *)osCfg->address)[0] == '0') {
        e9 = 0x80;
    }      
    
    PSF1_HEADER* fontHeader = (PSF1_HEADER*)fontFile->address;
    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1) {
        hcf();
    }
    
    uint64_t glyphBufferSize = fontHeader->charsize * 256;
    if (fontHeader->mode == 1) {
        glyphBufferSize = fontHeader->charsize * 512;
    }
    
    void* glyphBuffer = (void*)((uint8_t*)fontFile->address + sizeof(PSF1_HEADER));
    
    PSF1_FONT finishedFont_ins;
    PSF1_FONT* finishedFont = &finishedFont_ins;
    finishedFont->psf1_Header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;

    InitGfx(framebuffer);
    main_psf1_font = finishedFont;
    if (finishedFont == NULL) {
        hcf();
    } else {
        main_psf1_font = finishedFont;
    }
    
    DrawRect(0, 0, GetFb()->width, GetFb()->height, 0xFF4C565E);
    uint64_t dash_width = GetFb()->width * 65 / 1920;
    DrawRect(0, 0, dash_width, GetFb()->height, 0xFF3A444C);
    
    ClearColor = 0xFF282828;

    pool_init();
    
    #pragma endregion

    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDescriptor);

    asm volatile ("cli");

    PIC_remap(0x20, 0x28);

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    outb(0x20, 0x20);
    outb(0xA0, 0x20);

    idt_set_descriptor(0x20, &PITInt_Hndlr, 0x8E);
    idt_set_descriptor(0x21, &KeyboardInt_Hndlr, 0x8E);
    idt_set_descriptor(0x2C, &MouseInt_Hndlr, 0x8E);
    idt_set_descriptor(0x80, &SyscallInt_Hndlr, 0x8F);

    init_exceptions();
    idtr_t idtr_ = idt_init();
    
    uint16_t limit;
    uint64_t base;
    asm volatile ("sidt %0" : "=m" (idtr_));

    asm volatile ("sti");
    
    outb(0x21, 0b11111000);
    outb(0xA1, 0b11011111);
    IRQ_clear_mask(0);
    IRQ_clear_mask(1);
    IRQ_clear_mask(2);
    IRQ_clear_mask(12);
    IRQ_clear_mask(32);

    InitPS2Mouse();
    InitPitTimer();

    e9debugkf("Testing threading\n\r");
    Thread* thrd1 = (Thread*)page_alloc();
    Thread* thrd2 = (Thread*)page_alloc();
    Thread* thrd3 = (Thread*)page_alloc();
    Thread* thrd4 = (Thread*)page_alloc();
    Thread* thrd5 = (Thread*)page_alloc();
    
    e9debugkf("Initalizing Threads\n\r");
    thrd1->Handler = Test1;
    thrd2->Handler = Test2;
    thrd3->Handler = Test3;
    thrd4->Handler = Test4;
    thrd5->Handler = Test5;

    e9debugkf("Creating threads\n\r");
    int result = CreateThread(thrd1);
    result = CreateThread(thrd2);
    result = CreateThread(thrd3);
    result = CreateThread(thrd4);
    result = CreateThread(thrd5);

    while (1) {
        ProcessMousePacket();
    }
    
    hcf();
}
