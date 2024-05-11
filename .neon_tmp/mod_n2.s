.intel_syntax noprefix
.global malloc
.global membset
.section .rodata


.section .text
malloc:
	push rbp
	mov rbp, rsp
	sub rsp, 4
	movd [rbp-4], edi
	mov esi, [rbp-4]
	mov rdi, 0x00
	mov rdx, 0x03
	mov r10, 0x22
	mov r8, -0x01
	mov r9,  0x00
	mov rax, 0x09
	syscall
	leave
	ret
membset:
	push rbp
	mov rbp, rsp
	sub rsp, 9
	movq [rbp-8], rdi
	movb [rbp-9], sil
	movb sil, [rbp-9]
	movq rdi, [rbp-8]
	movb [rdi], sil
	leave
	ret

