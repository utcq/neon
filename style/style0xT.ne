#conn "std/types.ne"
#conn "std/mem.ne"

extern proc [void] printf($str fmt, ...);
extern proc [int] strlen($str str);

proc [int] add($int a, $int b) {
    let $int sum = a + b;
    ret sum;
}

proc [int] main() {
    let $u64 strc = "Hello\n";
    let $int length = strlen(strc);
    printf("L: %d\n", length);
    ret 0;
}
