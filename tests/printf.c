#include <stdio.h>
#include <stdlib.h>

int __attribute__ ((noinline)) add829(int x) {
    return x + 829;
}

int main(int argc, char **argv) {
    int y = atoi(argv[1]);
    // scanf("%d\n", &y);
    // printf("Hello!\n");
    printf("%d\n", add829(y));
    return 0;
}