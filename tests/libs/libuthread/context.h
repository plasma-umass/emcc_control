#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "uthread.h"


#if CONTEXT_IMPL == CONTS
    #include <emscripten/emscripten.h>
    #include "../../../include/continuations.h"
    #define context_t k_id
#elif CONTEXT_IMPL == ASYNCIFY

#elif CONTEXT_IMPL == SWAPCONTEXT
    #include <ucontext.h>
    typedef struct {
        ucontext_t uctx;
        void *stack;
    } SwapContext_ctx; 
    #define context_t SwapContext_ctx
#endif


void context_initialize_lib();
void context_init(context_t *ctx, uthread_func_t f, void *arg);
void context_switch(context_t *from, context_t *to);

#endif