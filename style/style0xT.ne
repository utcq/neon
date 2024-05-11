#conn "std/types.ne"
#conn "std/mem.ne"

extern proc [void] printf($str fmt, ...);

proc [int] add($int a, $int b) {
    let $int sum = a + b;
    ret sum;
}

proc [void] testprint($u64 addr, $int size) {
    asm "mov rax, 0x01";
    asm "mov rdi, 0x01";
    asm "mov rsi, {addr}";
    asm "mov edx, {size}";
    asm "syscall";
}

public proc [int] main() {
    let $u64 addr = malloc(0x10);
    let $u64 strc = "Hello\n";
    printf("addr: %p\nstr: %s", addr, strc);
    ret 0;
}
