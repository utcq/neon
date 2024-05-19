void test(int *x) {}

int main() {
    int x = 0;
    test(&x);
    return 0;
}