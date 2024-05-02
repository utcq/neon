.global nproc_main


.section .text
nproc_add:
	popq %rbp
	movq $000024, %rsi
	call halloc
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 16(%rax)
	popq %rbx
	movl 0(%rbx), %edi
	movl %edi, 20(%rax)
	movl 16(%rax), %esi
	movq %rsi, %rdi
	movl 20(%rax), %esi
	pushq %rax
	pushq %rbp
	movl %edi, 0(%rax)
	leaq 0(%rax), %rdx
	pushq %rdx
	movl %esi, 4(%rax)
	leaq 4(%rax), %rdx
	pushq %rdx
	call nproc___primitive_int_int_add
	popq %rbp
	popq %rax
	movq %rsi, %rbx
	pushq %rbp
	ret


nproc_main:
	popq %rbp
	movq $000016, %rsi
	call halloc
	pushq %rax
	pushq %rbp
	movq $8, %rsi
	call halloc
	movq $5, %rsi
	movl %esi, 0(%rax)
	leaq 0(%rax), %rdx
	pushq %rdx
	movq $2, %rsi
	movl %esi, 4(%rax)
	leaq 4(%rax), %rdx
	pushq %rdx
	call nproc_add
	popq %rbp
	popq %rax
	movq $1, %rsi
	movq %rsi, %rdi
	movq $1, %rsi
	pushq %rax
	pushq %rbp
	movl %edi, 0(%rax)
	leaq 0(%rax), %rdx
	pushq %rdx
	movl %esi, 4(%rax)
	leaq 4(%rax), %rdx
	pushq %rdx
	call nproc___primitive_int_int_sub
	popq %rbp
	popq %rax
	movq %rsi, %rbx
	pushq %rbp
	ret



