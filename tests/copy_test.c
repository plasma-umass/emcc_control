#include "../include/continuations.h"
#include <stdio.h>


DEFINE_HANDLER(handler, k_id k, uint64_t u) {
    RESTORE(CONTINUATION_COPY(k), 42);
}
DONT_DELETE_MY_HANDLER(handler)

DEFINE_HANDLER(handler2, k_id k, uint64_t u) {
    RESTORE(CONTINUATION_COPY(k), 24);
}
DONT_DELETE_MY_HANDLER(handler2)


DEFINE_HANDLER(the_main, k_id k, uint64_t u) {
    u = CONTROL(handler, 0) + CONTROL(handler2, 0);
    RESTORE(k, u);
}

DONT_DELETE_MY_HANDLER(the_main)



int main() {
    INIT_CONTINUATIONS_LIB();

    printf("Hello A!\n");


    printf("%llu\n", CONTROL(the_main, 0));

    printf("Ending print\n");

    return 0;
}
