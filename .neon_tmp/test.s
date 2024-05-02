.section .text
.global _start

_start:
    call nproc_main

    movq $0x00, %rdi
    movq $0x3C, %rax
    syscall
