#include <emscripten/emscripten.h>
#include "continuations.h"




uint64_t EMSCRIPTEN_KEEPALIVE __prim_control(uint64_t arg, uint64_t (*fn_ptr)(uint64_t, uint64_t)) {
    return fn_ptr(arg, 9873);
}


int EMSCRIPTEN_KEEPALIVE the_main() {

    return __prim_control(6, stuff() ? add812 : add829);
    // return add829(123);
}