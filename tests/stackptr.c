#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_stuff(int x) {
    printf("%p\n", x);
    return;
}

int handler1(k_id k, uint64_t _arg) {
    int x = 0x2;
    int *ptr = &x;
    print_stuff(x);
    print_stuff(ptr);
    *ptr = 0x4;
    print_stuff(x);
    return x;
}
DONT_DELETE_MY_HANDLER(handler1)
DONT_DELETE_MY_HANDLER(print_stuff)


int main() {
    handler1(0, 0);
    // control(handler1, 0);
}