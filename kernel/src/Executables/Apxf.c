#include "Apxf.h"

uint8_t ApxfStack[4096];
uint64_t Registers[4];        // 4 general-purpose registers R0..R3
uint64_t PopRegister;
uint64_t IEP;                 // Instruction Execution Pointer
uint64_t StackPointer;

uint64_t ApxfCallFrameStack[4096];
uint64_t CallFramePointer;

typedef struct {
    uint8_t opcode;
    uint8_t has_imm;
    uint8_t imm_size; // in bytes
} InstrDef;

static const InstrDef instr_table[256] = {
    [0x00] = {0x00, 0, 0},    // NOP
    [0x01] = {0x01, 1, 8},    // PUSH_IMM
    [0x02] = {0x02, 0, 0},    // POP
    [0x03] = {0x03, 0, 0},    // DUP
    [0x04] = {0x04, 0, 0},    // SWAP
    [0x05] = {0x05, 0, 0},    // ADD
    [0x06] = {0x06, 0, 0},    // SUB
    [0x07] = {0x07, 0, 0},    // MUL
    [0x08] = {0x08, 0, 0},    // DIV
    [0x09] = {0x09, 0, 0},    // MOD
    [0x0A] = {0x0A, 0, 0},    // AND
    [0x0B] = {0x0B, 0, 0},    // OR
    [0x0C] = {0x0C, 0, 0},    // XOR
    [0x0D] = {0x0D, 0, 0},    // NOT
    [0x0E] = {0x0E, 0, 0},    // SHL
    [0x0F] = {0x0F, 0, 0},    // SHR

    [0x10] = {0x10, 0, 0},    // CMP
    [0x11] = {0x11, 1, 8},    // JMP
    [0x12] = {0x12, 1, 8},    // JZ
    [0x13] = {0x13, 1, 8},    // JNZ
    [0x14] = {0x14, 1, 8},    // CALL
    [0x15] = {0x15, 0, 0},    // RET

    [0x20] = {0x20, 1, 8},    // LOAD
    [0x21] = {0x21, 1, 8},    // STORE
    [0x22] = {0x22, 1, 8},    // MOV_REG
    [0x23] = {0x23, 1, 8},    // SET_REG
    [0x24] = {0x24, 0, 0},    // GET_REG

    [0x30] = {0x30, 1, 8},    // WRITE_CHAR
    [0x31] = {0x31, 1, 8},    // READ_CHAR
    [0x32] = {0x32, 1, 8},    // OPEN
    [0x33] = {0x33, 1, 8},    // CLOSE
    [0x34] = {0x34, 1, 8},    // EXIT

    [0x40] = {0x40, 0, 0},    // gpx1_CRT_WINDOW_PROC
    [0x41] = {0x41, 0, 0},    // gpx1_WINDOW_PUT_PX
    [0x42] = {0x42, 0, 0},    // gpx1_WINDOW_GET_PX

    [0xFF] = {0xFF, 1, 1},    // END (imm8 status)
};

#include <TTY/AtsTty.h>

#include <HtKernelUtils/io.h>

int ExecApxf(void *ptr) {
    ApxfHeader *header = (ApxfHeader*)ptr;
    outb(0xE9, (char)(ptr+1));
    // Validate header
    if (header->magic[0] == 0xAB &&
        header->magic[1] == 'P' &&
        header->magic[2] == 'X' &&
        header->magic[3] == 'F' &&
        header->version == 0x0001 &&
        header->bitness == 0x0040
    ) {} else {
        return -1;
    }
    outb(0xE9, 't');

    // Initialize state
    outb(0xE9, 't');
    for (int i = 0; i < 4096; i++) {
        ApxfStack[i] = 0;
    }
    outb(0xE9, 't');

    outb(0xE9, 't');
    Registers[0] = Registers[1] = Registers[2] = Registers[3] = 0;
    outb(0xE9, 't');

    outb(0xE9, 't');
    StackPointer = 4095;  // stack grows down
    outb(0xE9, 't');
    IEP = header->entry_point;
    outb(0xE9, 't');
    CallFramePointer = -1;
    outb(0xE9, 't');

    outb(0xE9, 't');
    uint8_t *code = (uint8_t*)ptr + header->code_offset;
    outb(0xE9, 't');
    uint8_t comparisonFlag = 0;
    outb(0xE9, 't');

    // Execution loop
    outb(0xE9, 't');
    while (1) {
        uint8_t opcode = code[IEP];
        const InstrDef def = instr_table[opcode];
        uint64_t imm = 0;
        if (def.has_imm) {
            // fetch immediate (little-endian)
            for (int i = 0; i < def.imm_size; i++) {
                imm |= ((uint64_t)code[IEP++]) << (8 * i);
            }
        }
        switch (opcode) {
            case 0x00: // NOP
                break;
            case 0x01: // PUSH_IMM
                outb(0xE9, 'P');
                outb(0xE9, '\n');
                ApxfStack[StackPointer--] = imm;
                break;
            case 0x02: // POP
                PopRegister = ApxfStack[++StackPointer];
                break;
            case 0x03: // DUP
                ApxfStack[StackPointer] = ApxfStack[StackPointer+1];
                --StackPointer;
                break;
            case 0x04: { // SWAP
                uint64_t a = ApxfStack[StackPointer+1];
                ApxfStack[StackPointer+1] = ApxfStack[StackPointer];
                ApxfStack[StackPointer] = a;
                break;
            }
            case 0x05: // ADD
            case 0x06: // SUB
            case 0x07: // MUL
            case 0x08: // DIV
            case 0x09: // MOD
            case 0x0A: // AND
            case 0x0B: // OR
            case 0x0C: // XOR
            case 0x0D: // NOT
            case 0x0E: // SHL
            case 0x0F: { // SHR and arithmetic ops
                uint64_t b = ApxfStack[++StackPointer];
                uint64_t a = ApxfStack[++StackPointer];
                uint64_t res = 0;
                switch(opcode) {
                    case 0x05: res = a + b; break;
                    case 0x06: res = a - b; break;
                    case 0x07: res = a * b; break;
                    case 0x08: res = a / b; break;
                    case 0x09: res = a % b; break;
                    case 0x0A: res = a & b; break;
                    case 0x0B: res = a | b; break;
                    case 0x0C: res = a ^ b; break;
                    case 0x0D: res = ~a; break;
                    case 0x0E: res = a << b; break;
                    case 0x0F: res = a >> b; break;
                }
                ApxfStack[StackPointer--] = res;
                break;
            }
            case 0x10: { // CMP
                int64_t v2 = (int64_t)ApxfStack[++StackPointer];
                int64_t v1 = (int64_t)ApxfStack[++StackPointer];
                if (v1 == v2) comparisonFlag = 1;
                else if (v1 < v2) comparisonFlag = 2;
                else comparisonFlag = 4;
                break;
            }
            case 0x11: // JMP
                IEP = imm;
                break;
            case 0x12: // JZ
                if (comparisonFlag == 1) IEP = imm;
                break;
            case 0x13: // JNZ
                if (comparisonFlag != 1) IEP = imm;
                break;
            case 0x14: // CALL
                ApxfCallFrameStack[++CallFramePointer] = IEP;
                IEP = imm;
                break;
            case 0x15: // RET
                IEP = ApxfCallFrameStack[CallFramePointer--];
                break;
            case 0x20: // LOAD
                ApxfStack[StackPointer--] = *(uint64_t*)(uintptr_t)imm;
                break;
            case 0x21: // STORE
                *(uint64_t*)(uintptr_t)imm = ApxfStack[++StackPointer];
                break;
            case 0x22: // MOV_REG
                if (imm < 4) Registers[imm] = ApxfStack[++StackPointer];
                break;
            case 0x23: // SET_REG
                ApxfStack[StackPointer--] = Registers[imm < 4 ? imm : 0];
                break;
            case 0x24: // GET_REG
                ApxfStack[StackPointer--] = Registers[0];
                break;
            case 0x30: // WRITE_CHAR
            outb(0xE9, 'W');
            outb(0xE9, '\n');
                tty_putchar((char)imm);
                break;
            case 0x31: // READ_CHAR
                //ApxfStack[StackPointer--] = (uint8_t)getchar();
                break; // NIY
            case 0x32: // OPEN
                //ApxfStack[StackPointer--] = (uint64_t)open((char*)imm, O_RDONLY);
                break; // NIY
            case 0x33: // CLOSE
                //close((int)imm);
                break; // NIY
            case 0x34: // EXIT
                return (int)(imm & 0xFF);
            case 0x40: // gpx1_CRT_WINDOW_PROC
            case 0x41: // gpx1_WINDOW_PUT_PX
            case 0x42: // gpx1_WINDOW_GET_PX
                break;
            case 0xFF: // END
                return (int8_t)imm;
            default:
                // unknown opcode fault
                return -3;
        }
    }
    return 0;
}
