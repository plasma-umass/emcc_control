#include <emscripten/emscripten.h>
#include "../include/continuations.h"




int EMSCRIPTEN_KEEPALIVE the_main() {

    return control(6, stuff() ? add812 : add829);
    // return add829(123);
}