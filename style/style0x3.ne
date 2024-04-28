%imp io;

fn add($int x, $int y) = (x + y);

proc[int] main() {
    let $int sum = add(2,3);
    printf("%d", sum);
    %ret 0;
}

