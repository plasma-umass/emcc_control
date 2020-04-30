#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void print_stuff(char c, int x) {
    printf("%c: %d\n", c, x);
    return;
}



k_id *stack = 0;
k_id *stack_top = 0;
int num_k = 0;

void pushk(k_id k) {
    *stack_top = k;
    stack_top++;
    num_k++;
}

k_id popk() {
    stack_top--;
    num_k--;
    return *stack_top;
}

void fork_handler(k_id k, uint64_t _arg) {
    pushk(continuation_copy(k));
    restore(k, 0);
}

void fork() {
    control(fork_handler, 0);
}

int make_int() {
    return rand() % 10;
}

void the_main() {
    int x = make_int();
    // int *p = &x;

    print_stuff('a', x);
    x++;
    // fork();
    print_stuff('b', x);
}

int driver(k_id k, uint64_t _arg) {
    
    the_main();
    if(num_k > 0) {
        k_id next = popk();
        restore(next, 0);
    }

    restore(k, 0);
}



int main() {
    // handler1(0, 0);
    srand(1234);
    stack = malloc(sizeof(k_id) * 10);
    stack_top = stack;

    control(driver, 0);

}


DONT_DELETE_MY_HANDLER(pushk)
DONT_DELETE_MY_HANDLER(popk)
DONT_DELETE_MY_HANDLER(print_stuff)
DONT_DELETE_MY_HANDLER(fork)
DONT_DELETE_MY_HANDLER(fork_handler)

DONT_DELETE_MY_HANDLER(the_main)

DONT_DELETE_MY_HANDLER(driver)
DONT_DELETE_MY_HANDLER(main)
DONT_DELETE_MY_HANDLER(make_int)