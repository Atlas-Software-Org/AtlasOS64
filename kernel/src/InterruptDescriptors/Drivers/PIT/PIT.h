#pragma once

void Sleep(uint64_t milliseconds);
void uSleep(uint64_t nanoseconds);
__attribute__((interrupt)) void PITInt_Hndlr(struct InterruptFrame* frame);
void InitPitTimer();
