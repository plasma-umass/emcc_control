#include <emscripten/emscripten.h>
#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include <iostream>


std::vector<k_id> rest;

DEFINE_HANDLER(fork_handler, k, _args, {
    rest.push_back(continuation_copy(k)); // <--- THIS LINE OF CODE TRIGGERS A TRAP, SINCE ITS NOT ALLOWED
    restore(k, 0);
})

void fork() {
    control(fork_handler, 0);
}

typedef void (*body_fn)();


void driver(body_fn body) {
    body();

    if(rest.size() > 0) {
        printf("Restoring thunk...\n");
        k_id rk = rest.back();
        rest.pop_back();
        restore(rk, 0);
    }
}

void bad_forker() {
    printf("Start of forker\n");
    fork();
    printf("End of forker\n");
}

void bad() {
    printf("Start of bad\n");
    prompt(bad_forker());
    // bad_forker();
    printf("End of bad\n");
    
}

void bar() {
    printf("Starting bar <--------- \n");
    bad();
    printf("Ending bar <--------- \n");
}


void example() {
    printf("Before foreign call\n");
    bar();
    printf("After foreign call\n");
}


DEFINE_HANDLER(the_main, k, u, {
    driver((body_fn)u);
    restore(k, 0);
})


int main() {
    initialize_continuations();
    control(the_main, (uint64_t)example);

    return 0;
}