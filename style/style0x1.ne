#conn "std/types.ne"
#conn "std/mem.ne"

proc [int] add($int a, $int b) {
    %ret (a + b);
}

proc[int] main() {
    #opt pub
    add(5,2);
    let $int xy = add(1,2);
    let $u64 addr = malloc(12);
    membset(addr, 'h');
    membset(addr+1, 'e');
    %ret addr;
}