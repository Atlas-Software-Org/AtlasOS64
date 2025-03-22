#pragma once

struct InterruptFrame;

__attribute__((interrupt)) void SyscallInt_Hndlr(struct InterruptFrame* frame);