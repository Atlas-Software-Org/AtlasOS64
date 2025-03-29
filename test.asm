_start:
    mov rdi, 25
    mov rsi, 25
    mov rdx, 75
    mov rcx, 75
    mov r8, 0xFF00FF00
    call DrawRect

DrawRect:
    mov r9, rdi
    lea r10, [rdi + rdx]
outer_loop:
    cmp r9, r10
    jge DrawRect_end_outer_loop
    mov r11, rsi
    lea r12, [rsi + rcx]
inner_loop:
    cmp r11, r12
    jge DrawRect_end_inner_loop
    mov rdi, r9
    mov rsi, r11
    mov rdx, r8
    call PutPx
    inc r11
    jmp inner_loop
DrawRect_end_inner_loop:
    inc r9
    jmp outer_loop
DrawRect_end_outer_loop:
    ret

PutPx:
    call SysGpx1IOWrite
    ret

SysGpx1IOWrite:
    int 128
    ret
