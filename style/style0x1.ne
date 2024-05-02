#conn "std/types.ne"

proc [int] add($int a, $int b) {
    %ret (a + b);
}

proc[int] main() {
    #opt pub
    add(5,2);
    %ret (1-1);
}