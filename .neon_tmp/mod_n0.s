.intel_syntax noprefix
.global main
.section .rodata
CONST_STR_0:
	.string "Hello\n"
CONST_STR_1:
	.string "addr: %p\nstr: %s"


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
testprint:
	push rbp
	mov rbp, rsp
	sub rsp, 12
	movq [rbp-8], rdi
	movd [rbp-12], esi
	mov rax, 0x01
	mov rdi, 0x01
	mov rsi, [rbp-8]
	mov edx, [rbp-12]
	syscall
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov rax, +16
	mov rdi, rax
	call malloc
	movq [rbp-8], rax
	movd eax, OFFSET CONST_STR_0
	movq [rbp-16], rax
	movq rax, [rbp-16]
	mov rdx, rax
	movq rax, [rbp-8]
	mov rsi, rax
	movd eax, OFFSET CONST_STR_1
	mov rdi, rax
	call printf
	mov rax, +0
	leave
	ret

