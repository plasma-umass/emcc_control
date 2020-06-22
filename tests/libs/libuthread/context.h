#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "config.h"
#include "uthread.h"


#if CONTEXT_IMPL == WASMTIME_CONTS
    #include <emscripten/emscripten.h>
    #include "../../../include/continuations.h"
    #define context_t k_id
#elif CONTEXT_IMPL == WASMTIME_ASYNCIFY
    #include <stdint.h>
    typedef struct {
        uint32_t async_buf;
        char is_async;
        // void (*f)(void*, void*);
        void *f;
        char is_two_args;
        void *arg1;
        void *arg2;
    } asyncify_context; 
    #define context_t asyncify_context
#elif CONTEXT_IMPL == NATIVE_SWAPCONTEXT
    #include <ucontext.h>
    typedef struct {
        ucontext_t uctx;
        void *stack;
    } SwapContext_ctx; 
    #define context_t SwapContext_ctx
#endif


#if NEED_CONTEXT

void context_initialize_lib();
void context_main(void (*f)(void*), void *arg);
void context_init(context_t *ctx, uthread_func_t f, void *arg);
void context_switch(context_t *from, context_t to);

#endif

#endif