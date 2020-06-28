#include <stdio.h>
#include <stdlib.h>

#include <setjmp.h>
// #endif

#include "uthread.h"

/* Size of the stack for a thread (in bytes) */
#define UTHREAD_STACK_SIZE 32768


void context_switch(SwapContext_ctx *from, SwapContext_ctx to) {
    if(swapcontext(&from->uctx, &to.uctx)) {
        perror("swapcontext");
		exit(1);
    }
}

void *uthread_ctx_alloc_stack(void) {
	return malloc(UTHREAD_STACK_SIZE);
}

void uthread_ctx_destroy_stack(void *top_of_stack) {
	// free(top_of_stack);
}

static void uthread_ctx_bootstrap(uthread_func_t func, void *arg)
{
	/*
	 * Enable interrupts right after being elected to run for the first time
	 */
	// preempt_enable();

	/* Execute thread and when done, exit with the return value */
    func(arg);
	uthread_exit(0);
}

void context_init(SwapContext_ctx *ctx, uthread_func_t func, void *arg) {
	
    if(getcontext(&ctx->uctx)) {
        return;
    }

    ctx->stack = uthread_ctx_alloc_stack();
    ctx->uctx.uc_stack.ss_sp = ctx->stack;
    ctx->uctx.uc_stack.ss_size = UTHREAD_STACK_SIZE;
    makecontext(&ctx->uctx, (void (*)(void)) uthread_ctx_bootstrap, 2, func, arg);
    
}


void context_initialize_lib() {}

void context_main(void (*f)(int, char**), int argc, char **argv) {
	f(argc, argv);
}