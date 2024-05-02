.global nproc___primitive_int_int_add
.global nproc___primitive_int_int_sub
.global nproc___primitive_i64_i64_add


.section .text
nproc___primitive_int_int_add:
	popq %rbp
	movq $000024, %rsi
	call halloc
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 16(%rax)
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 20(%rax)
	movl 16(%rax), %edi
	movl 20(%rax), %esi
	addq %rdi, %rsi
	pushq %rbp
	ret


nproc___primitive_int_int_sub:
	popq %rbp
	movq $000024, %rsi
	call halloc
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 16(%rax)
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 20(%rax)
	movl 16(%rax), %edi
	movl 20(%rax), %esi
	subq %rdi, %rsi
	pushq %rbp
	ret


nproc___primitive_i64_i64_add:
	popq %rbp
	movq $000032, %rsi
	call halloc
	popq %rbx
	movq 0(%rbx), %rdi
	movq %rdi, 16(%rax)
	popq %rbx
	movq 0(%rbx), %rdi
	movq %rdi, 24(%rax)
	movq 16(%rax), %rdi
	movq 24(%rax), %rsi
	subq %rdi, %rsi
	pushq %rbp
	ret



