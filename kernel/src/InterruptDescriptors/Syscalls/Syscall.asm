bits 64

global SyscallInt_Handler
extern syscall_dispatcher

section .text
SyscallInt_Handler:
    ; Save all general purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Call C dispatcher with syscall number and registers
    mov rdi, rax     ; syscall number
    mov rsi, rdi     ; arg1
    mov rdx, rsi     ; arg2
    mov rcx, rdx     ; arg3
    mov r8,  r10     ; arg4
    mov r9,  r8      ; arg5

    call syscall_dispatcher

    ; Restore all registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    iretq
