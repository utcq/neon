public proc [u64] malloc($int size) {
    asm "mov esi, {size}";
    asm "mov rdi, 0x00";
    asm "mov rdx, 0x03";
    asm "mov r10, 0x22";
    asm "mov r8, -0x01";
    asm "mov r9,  0x00";
    asm "mov rax, 0x09";
    asm "syscall";
}

public proc [void] membset($u64 addr, $char byte) {
    asm "movb sil, {byte}";
    asm "movq rdi, {addr}";
    asm "movb [rdi], sil";
}