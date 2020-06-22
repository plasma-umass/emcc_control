// #include <emscripten/emscripten.h>
// #include "../../../include/continuations.h"

#include <stdint.h>
#include "uthread.h"
#include "../../../include/asyncify.h"

#include <stdio.h>

char sleeping;
void context_initialize_lib() {
    sleeping = 0;
}

// struct asyncify_context {
    // uint32_t async_buf;
    // char is_async;
    // void (*f)(void*, void*);
    // char is_two_args;
    // void *arg1;
    // void *arg2;
// };

asyncify_context active_context;
asyncify_context next_context;

void context_main(void (*f)(void*), void *arg) {
    active_context.async_buf = alloc_asyncify_buf();
    active_context.f = f;
    active_context.arg1 = arg;
    active_context.is_two_args = 0;
    active_context.is_async = 0;

    // printf("A\n");
    f(arg);
    while(sleeping) {
        // if(sleeping) {
            asyncify_stop_unwind();
            // active_thread = next_tid;
            active_context = next_context;
            if(active_context.is_async) {
                asyncify_start_rewind(active_context.async_buf);
                if(active_context.is_two_args) {
                    ((void (*)(void*,void*))active_context.f)(active_context.arg1, active_context.arg2);
                } else {
                    ((void (*)(void*))active_context.f)(active_context.arg1);
                }
            } else {
                sleeping = 0;
                if(active_context.is_two_args) {
                    ((void (*)(void*,void*))active_context.f)(active_context.arg1, active_context.arg2);
                } else {
                    ((void (*)(void*))active_context.f)(active_context.arg1);
                }           
            }
        // }
    }
}

static void asyncify_bootstrap(void *func, void *arg)
{
	/*
	 * Enable interrupts right after being elected to run for the first time
	 */
	// preempt_enable();

	/* Execute thread and when done, exit with the return value */
    ((uthread_func_t)func)(arg);
	uthread_exit(0);
}

void context_init(asyncify_context *ctx, uthread_func_t f, void *arg) {
    ctx->async_buf = alloc_asyncify_buf();
    ctx->is_async = 0;
    ctx->f = asyncify_bootstrap;
    ctx->arg1 = f;
    ctx->is_two_args = 1;
    ctx->arg2 = arg;
}

void context_switch(asyncify_context *from, asyncify_context to) {
    if(sleeping == 0) {
        sleeping = 1;
        *from = active_context;
        from->is_async = 1;
        next_context = to;
        asyncify_start_unwind(active_context.async_buf);
    } else { 
        asyncify_stop_rewind();
        sleeping = 0;
    }
}
