global _start

_start:
    mov rax, 12
    lea rsi, [string]
    mov rdx, 100
    int 0x80
    ret

string:
    db 'Hello, World! from an executable', 0