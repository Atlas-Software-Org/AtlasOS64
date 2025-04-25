#include "idt.h"
#include <gpx1.h>
#include <HtKernelUtils/debug.h>
#include <memory/paging.h>
#include <HtKernelUtils/debug.h>
#include <Language/language.h>

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

static idtr_t idtr;

char error_codes[80][128] = {
    // English Errors (0-19)
    "Division by 0", "Reserved1", "NMI Interrupt", "Breakpoint (INT3)", "Overflow (INTO)", "Bounds range exceeded (BOUND)", "Invalid opcode (UD2)",
    "Device not available (WAIT/FWAIT)", "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present", "Stack-segment fault",
    "General protection fault (GPFault)", "Page fault", "Reserved2", "x87 FPU error", "Alignment check", "SIMD Floating-Point Exception", "Reserved3",

    // French Errors (20-39)
    "Division par 0", "Reserve1", "Interruption NMI", "Point d'arret (INT3)", "Debordement (INTO)", "Plage de limites depassee (BOUND)", "Code d'operation non valide (UD2)",
    "Appareil non disponible (WAIT/FWAIT)", "Double faute", "Depassement de segment du coprocesseur", "TSS invalide", "Segment non present", "Defaut de segment de pile",
    "Defaut de protection generale (GPFault)", "Defaut de page", "Reserve2", "Erreur FPU x87", "Controle d'alignement", "Exception a virgule flottante SIMD", "Reserve3",

    // German Errors (40-59)
    "Division durch 0", "Reserviert1", "NMI-Interrupt", "Breakpoint (INT3)", "Ueberlauf (INTO)", "Bereichsueberschreitung (BOUND)", "Ungueltiger Opcode (UD2)",
    "Geraet nicht verfuegbar (WAIT/FWAIT)", "Doppelfehler", "Coprocessor-Segmentueberlauf", "Ungueltiger TSS", "Segment nicht vorhanden", "Stack-Segmentfehler",
    "Allgemeiner Schutzfehler (GPFault)", "Seitenfehler", "Reserviert2", "x87 FPU-Fehler", "Ausrichtungspruefung", "SIMD Gleitkomma-Fehler", "Reserviert3",

    // Spanish Errors (60-79)
    "Division por 0", "Reservado1", "Interrupcion NMI", "Punto de interrupcion (INT3)", "Desbordamiento (INTO)", "Rango de limites excedido (BOUND)", "Operacion no valida (UD2)",
    "Dispositivo no disponible (WAIT/FWAIT)", "Fallo doble", "Desbordamiento de segmento coprocesador", "TSS invalido", "Segmento no presente", "Fallo de segmento de pila",
    "Fallo de proteccion general (GPFault)", "Fallo de pagina", "Reservado2", "Error FPU x87", "Chequeo de alineacion", "Excepcion de punto flotante SIMD", "Reservado3"
};

int SkipPf = 0;

void exception_handler(int exception) {
    DrawRect(0, 0, GetFb()->width, GetFb()->height, 0xFF357EC7);
    FontPutStr(":( An error just occured. Halting...", 6, 6, 0xFF000000);
    FontPutStr(":( An error just occured. Halting...", 4, 4, 0xFF000000);
    FontPutStr(":( An error just occured. Halting...", 4, 6, 0xFF000000);
    FontPutStr(":( An error just occured. Halting...", 6, 4, 0xFF000000);
    FontPutStr(":( An error just occured. Halting...", 5, 5, 0xFFFFFFFF);
    int langauge = GetLanguage();
    int index = langauge * 20 + exception;

    if (0 <= exception && exception <= 20) {
        FontPutStr(error_codes[index], 26, 26, 0xFF000000);
        FontPutStr(error_codes[index], 24, 24, 0xFF000000);
        FontPutStr(error_codes[index], 24, 26, 0xFF000000);
        FontPutStr(error_codes[index], 26, 24, 0xFF000000);
        FontPutStr(error_codes[index], 25, 25, 0xFFFFFFFF);
    }
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    while (1);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

extern void* isr_stub_table[];

void init_exceptions() {
    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
}

idtr_t idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT

    return idtr;
}

#define PIC_EOI		0x20		/* End-of-interrupt command code */

void PIC_sendEOI(uint8_t irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	IOWait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	IOWait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	IOWait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	IOWait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	IOWait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	IOWait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	IOWait();
	outb(PIC2_DATA, ICW4_8086);
	IOWait();

	// Unmask both PICs.
	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}
