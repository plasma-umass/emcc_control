// #include <emscripten/emscripten.h>
// #include "../../../include/continuations.h"

#include <stdint.h>
#include "uthread.h"
#include "../../include/asyncify.h"

char sleeping;
void context_initialize_lib() {
    sleeping = 0;
}

uint32_t active_thread_buf;
uint32_t next_thread_buf;

void context_main(void (*f)(void*), void *arg) {
    active_thread_buf = alloc_asyncify_buf();
    f(arg);
    while(sleeping) {
        // if(sleeping) {
            asyncify_stop_unwind();
            // active_thread = next_tid;
            active_thread_buf = next_thread_buf;
            asyncify_start_rewind(active_thread_buf);
            f(arg);
        // }
    }
}

void context_init(uint32_t *ctx, uthread_func_t f, void *arg) {
    *ctx = alloc_asyncify_buf();

    if(sleeping == 0) {
        sleeping = 1;
        asyncify_start_unwind(*ctx);
    } else { 
        asyncify_stop_rewind();
        sleeping = 0;
    }
    f(arg);
}

void context_switch(uint32_t *from, uint32_t to) {
    *from = active_thread_buf;
    next_thread_buf = to;
    if(sleeping == 0) {
        sleeping = 1;
        asyncify_start_unwind(active_thread_buf);
    } else { 
        asyncify_stop_rewind();
        sleeping = 0;
    }
}
