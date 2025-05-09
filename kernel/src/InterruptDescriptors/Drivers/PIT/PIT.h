#ifndef PIT_H
#define PIT_H 1

#include <stdint.h>

void Sleep(uint64_t milliseconds);
void uSleep(uint64_t nanoseconds);
__attribute__((interrupt)) void PITInt_Hndlr(struct InterruptFrame* frame);
void InitPitTimer();

#endif /* PIT_H */