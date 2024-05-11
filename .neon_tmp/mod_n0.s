.intel_syntax noprefix
.section .rodata
CONST_STR_0:
	.string "Hello\n"
CONST_STR_1:
	.string "G: %d\n"


.section .text
add:
	push rbp
	mov rbp, rsp
	sub rsp, 12
	movd [rbp-4], edi
	movd [rbp-8], esi
	movd eax, [rbp-4]
	movd ebx, [rbp-8]
	add rax,rbx
	movd [rbp-12], eax
	movd eax, [rbp-12]
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 12
	movd eax, OFFSET CONST_STR_0
	movq [rbp-8], rax
	movq rax, [rbp-8]
	mov rdi, rax
	xor rax, rax
	call strlen
	movd [rbp-12], eax
	movd eax, [rbp-12]
	mov rsi, rax
	movd eax, OFFSET CONST_STR_1
	mov rdi, rax
	xor rax, rax
	call printf
	mov rax, +0
	leave
	ret

