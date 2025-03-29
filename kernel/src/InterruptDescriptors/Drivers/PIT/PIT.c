#include <HtKernelUtils/io.h>

#define PIT_BASE 0x40
#define PIT_CMD  0x43
#define PIT_CH0  0x40
#define PIT_CH1  0x41
#define PIT_CH2  0x42

#define PIT_FREQUENCY 1193180
#define TICK_INTERVAL  1000

volatile uint64_t TimeTicks = 0;

void Sleep(uint64_t milliseconds) {
    uint64_t startTime = TimeTicks;
    uint64_t endTime = startTime + (milliseconds * TICK_INTERVAL);
    while (TimeTicks < endTime) {
        asm("nop");
    }
}

void uSleep(uint64_t nanoseconds) {
    uint64_t startTime = TimeTicks;
    uint64_t endTime = startTime + (nanoseconds / 1000);
    while (TimeTicks < endTime) {
        asm("nop");
    }
}

#include <InterruptDescriptors/Drivers/PIT/Scheduler/Sched.h>

__attribute__((interrupt)) void PITInt_Hndlr(struct InterruptFrame* frame) {
    TimeTicks++;
    outb(0x20, 0x20);
}

void InitPitTimer() {
    uint16_t divisor = PIT_FREQUENCY / 1000;

    outb(PIT_CMD, 0x36);

    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));
}
