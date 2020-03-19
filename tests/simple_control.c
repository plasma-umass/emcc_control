#include "../include/continuations.h"
#include <stdio.h>

DEFINE_HANDLER(handler, k_id k, uint64_t u) {
    // printf("hi");
    RESTORE(CONTINUATION_COPY(k), 42);
}

DEFINE_HANDLER(handler_two, k_id k, uint64_t u) {
    // printf("hi");
    RESTORE(CONTINUATION_COPY(k), 987);
}

DONT_DELETE_MY_HANDLER(handler)
DONT_DELETE_MY_HANDLER(handler_two)

DEFINE_HANDLER(handler_three, k_id k, uint64_t u) {
    // printf("Hello B!\n");
    // printf("%llu\n", CONTROL(handler, 0) + CONTROL(handler_two, 0));

    // pracint

    RESTORE(k, CONTROL(handler, 0));
}

DONT_DELETE_MY_HANDLER(handler_three)

void other_func() {
    printf("Hello C!\n");
}

int main() {
    INIT_CONTINUATIONS_LIB();

    printf("Hello A!\n");

    other_func();

    printf("%llu\n", CONTROL(handler_three, 0));

    printf("Ending print\n");

    return 0;
}
