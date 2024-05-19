.intel_syntax noprefix
.section .rodata
CONST_STR_0:
	.string "Hello"
CONST_STR_1:
	.string "S: %p\nL: %d\n"
CONST_STR_2:
	.string "S: %p\n"
CONST_STR_3:
	.string "TS: %p\n"


.section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 20
	movd eax, OFFSET CONST_STR_0
	movq [rbp-8], rax
	movq rax, [rbp-8]
	mov rdi, rax
	xor rax, rax
	call strlen
	movd [rbp-12], eax
	movd eax, [rbp-12]
	mov rdx, rax
	movq rax, [rbp-8]
	mov rsi, rax
	movd eax, OFFSET CONST_STR_1
	mov rdi, rax
	xor rax, rax
	call printf
	movq rax, [rbp-8]
	mov rsi, rax
	movd eax, OFFSET CONST_STR_2
	mov rdi, rax
	xor rax, rax
	call printf
	lea rax, [rbp-12]
	movq [rbp-20], rax
	movq rax, [rbp-20]
	mov rsi, rax
	movd eax, OFFSET CONST_STR_3
	mov rdi, rax
	xor rax, rax
	call printf
	mov rax, +0
	leave
	ret

