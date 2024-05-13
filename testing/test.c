#include <stdio.h>

#include <stdlib.h>

int main() {
    int i = 0x50;
    char *input = (char*)malloc(sizeof(char) * 100); // Allocate memory for input
    scanf("%s", input);
    printf(input);
    free(input); // Free the allocated memory
    return 0;
}