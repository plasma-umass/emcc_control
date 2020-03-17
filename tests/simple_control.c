#include <emscripten/emscripten.h>
#include "continuations.h"

void handler(uint64_t k, uint64_t u) {
    RESTORE(k, 42);
}
void handler_two(uint64_t k, uint64_t u) {
    RESTORE(k, 987);
}

DONT_DELETE_MY_HANDLER(handler)
DONT_DELETE_MY_HANDLER(handler_two)


int EMSCRIPTEN_KEEPALIVE the_main() {
    return CONTROL(handler, 1234) + CONTROL(handler_two, 5678);
}

int main() {
    return 0;
}