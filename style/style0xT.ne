#conn "std/types.ne"
#conn "std/mem.ne"

extern proc [void] printf($str fmt, ...);
extern proc [int] strlen($str str);
//extern proc [str] strtok($str)

proc [int] main() {
    let $str strc = "Hello";
    let $int length = strlen(strc);
    printf("S: %p\nL: %d\n", strc, length);
    printf("S: %p\n", strc);
    let $u64 ts = &length;
    printf("TS: %p\n", ts);
    ret 0;
}