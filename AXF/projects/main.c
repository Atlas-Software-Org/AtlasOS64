#include "axf.h"

#include <stdint.h>

// ats_crt0.asm included

void Syscall_Dispatch(uint64_t sys_vector, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    asm volatile (
        ".intel_syntax noprefix\n"
        "mov rax, %0\n"
        "mov rsi, %1\n"
        "mov rdx, %2\n"
        "mov r10, %3\n"
        "mov r8, %4\n"
        "int 0x80\n"
        ".att_syntax prefix\n"
        :
        : "r"(sys_vector), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3)
        : "rax", "rsi", "rdx", "r10", "r8"
    );
}

int main(int argc, char** argv) {
	(void)argc;(void)argv;

	// Shell main

	while (1) {
		char* prompt = "user@atlas_os:~$ \0";
		int sizeof_prompt = 18;

		char* unknown = "Unknown prompt\0";
		int sizeof_unknown = 15;

		Syscall_Dispatch(12, (uint64_t)prompt, sizeof_prompt, 0, 0);
	
		char buffer[256] = {0};
		Syscall_Dispatch(13, (uint64_t)buffer, 256, 0, 0);
		for (int i = 0; i < 256; i++) {
			if (buffer[i] == 0 || buffer[i] == '\n') {
				buffer[i] = 0;
				break;
			}
		}

		if (buffer[0] == 'e') {
			if (buffer[1] == 'x') {
				if (buffer[2] == 'i') {
					if (buffer[3] == 't') {
						return 0;
					}
				}
			}
		} else {
			Syscall_Dispatch(12, (uint64_t)unknown, sizeof_unknown, 0, 0);
		}
	}

	return 0;		
}
