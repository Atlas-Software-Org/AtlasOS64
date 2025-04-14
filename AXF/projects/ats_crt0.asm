global _start
[BITS 64]

extern main

section .data
ARGV0: dq arg0_str
arg0_str: db "userapp", 0

section .text
_start:
    push rbp
    mov rbp, rsp

    mov rdi, 1          ; argc = 1
    mov rsi, ARGV0      ; argv = &ARGV0

    call main

    pop rbp
