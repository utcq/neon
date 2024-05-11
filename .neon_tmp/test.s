.intel_syntax noprefix
.global _start

.section .text

_start:
	call main
	mov rax, +60
	xor rdi, rdi
	syscall
