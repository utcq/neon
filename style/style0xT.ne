#conn "std/types.ne"
#conn "std/mem.ne"

extern proc [void] printf($str fmt, ...);
extern proc [int] strlen($str str);

proc [int] main() {
    let $str strc = "Hello";
    let $int length = strlen(strc);
    printf("S: %s\nL: %d\n", strc, length);
    ret 1+2;
}
