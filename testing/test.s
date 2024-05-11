.intel_syntax noprefix

.global _start
.section .text

add: # (int a, int b)
    push rbp
    mov rbp, rsp

    # a=rsp+16, b=rsp+20
    movd edi, [rsp+16]
    movd esi, [rsp+20]
    movd edx, [rsp+24]
    movd eax, [rsp+28]

    pop rbp
    ret

main:
    push rbp
    mov rbp, rsp

    sub rsp,0x10           # Alloc
    movd [rsp+0],  0xa     # rsp + param_offset
    movd [rsp+4],  0xb     # rsp + param_offset
    movd [rsp+8],  0xc     # rsp + param_offset
    movd [rsp+12], 0xd     # rsp + param_offset
    call add               # add(0xd,0xc,0xb,0xa)
    add rsp,0x10           # Dealloc

    pop rbp
    ret

_start:
    call main
    mov rax, 0x3C
    mov rdi, 0x00
    syscall
