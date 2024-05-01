proc [int] __primitive_int_int_add($int a, $int b) {
    #opt pub
    %asm "movl {a}, %edi";
    %asm "movl {b}, %esi";
    %asm "addq %rdi, %rsi";
    %ret;
}

proc [int] __primitive_int_int_sub($int a, $int b) {
    #opt pub
    %asm "movl {a}, %edi";
    %asm "movl {b}, %esi";
    %asm "subq %rdi, %rsi";
    %ret;
}


proc [i64] __primitive_i64_i64_add($i64 a, $i64 b) {
    #opt pub
    %asm "movq {a}, %rdi";
    %asm "movq {b}, %rsi";
    %asm "subq %rdi, %rsi";
    %ret;
}