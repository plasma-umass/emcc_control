#include "../include/continuations.h"
#include <stdio.h>

void handler(uint64_t k, uint64_t u) {
    RESTORE(k, 42);
}
void handler_two(uint64_t k, uint64_t u) {
    RESTORE(k, 987);
}

DONT_DELETE_MY_HANDLER(handler)
DONT_DELETE_MY_HANDLER(handler_two)


int main() {
    INIT_CONTINUATIONS_LIB();

    printf("%llu\n", CONTROL(handler, 0) + CONTROL(handler_two, 0));
    return 0;
}
