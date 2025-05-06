global jump_usermode
extern UserMain

section .bss
align 16
UserStack:     resb 8192           ; Reserve 8 KB stack (grows downward)
UserStackTop:                          ; Symbol for stack top
              ; nothing here, this label just points to the next byte
              ; which is the top of the stack

section .text
jump_usermode:
    cli

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, 0x23
    push rax
    lea rax, [UserStackTop]
    push rax

    pushfq
    pop rax
    or rax, 0x200
    push rax

    mov rax, 0x1B
    push rax
    mov rax, UserMain
    push rax

    iretq
