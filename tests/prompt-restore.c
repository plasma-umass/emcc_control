#include <emscripten/emscripten.h>
#include "../include/continuations.h"
// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>

k_id k1;
k_id k2;

void h2(k_id k, uint64_t arg) {
    k2 = k;

    printf("Restoring!\n");
    restore(k1, 0); // <--- THIS LINE OF CODE TRIGGERS A TRAP, SINCE ITS NOT ALLOWED
    // restore(k2, 0); // This line of code is ok.
}

void bad() {
    control(h2, 0);
}

void bar() {
    printf("Call to `bad` from `bar`.\n");
    prompt(bad());
    printf("Return from `bad` to `bar`.\n");
}

// Next implementation
void h1(k_id k, uint64_t arg) {
    k1 = k;
    bar();
}

int main() {
    initialize_continuations();
    
    control(h1, 0);

    printf("All done with main!\n");

    return 0;
}


DONT_DELETE_MY_HANDLER(h1)
DONT_DELETE_MY_HANDLER(h2)
