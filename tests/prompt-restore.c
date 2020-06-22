#include <emscripten/emscripten.h>
#include "../include/continuations.h"
#include <stdio.h>

k_id k1;
k_id k2;

DEFINE_HANDLER(h2, k, arg, {
    k2 = k;

    printf("Restoring!\n");
    restore(k1, 0); // <--- THIS LINE OF CODE TRIGGERS A TRAP, SINCE ITS NOT ALLOWED
    // restore(k2, 0); // This line of code is ok.
})

void bad() {
    control(h2, 0);
}

void bar() {
    printf("Call to `bad` from `bar`.\n");
    prompt(bad());
    printf("Return from `bad` to `bar`.\n");
}

DEFINE_HANDLER(h1, k, arg, {
    k1 = k;
    bar();
})

int main() {
    initialize_continuations();
    
    control(h1, 0);

    printf("All done with main!\n");

    return 0;
}
