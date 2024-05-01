SYS_mmap = 9
PROT_READ_WRITE = 3
MAP_PRIVATE = 2
MAP_ANONYMOUS = 0x20

.section .text
.global halloc

halloc: # SIZE = RSI
    mov $0, %rdi
	mov $(PROT_READ_WRITE), %rdx
	mov $(MAP_PRIVATE | MAP_ANONYMOUS), %r10
	mov $-1, %r8
	mov $0, %r9
	mov $(SYS_mmap), %rax
	syscall
	ret

