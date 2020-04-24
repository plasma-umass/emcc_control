#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    k_id after_next;
    k_id after_yield;
    uint64_t value;
} Generator;

typedef void (*gen_fn)(Generator *);

// Helpers for lifting a function up to a continuation
// Given a function f with 1 parameter, lift(f) returns a continuation id k
// such that restore(k, x) executes f(x).
void _return_lift_result(k_id k, uint64_t _after_lift_kid) {
    restore(_after_lift_kid, k);
}

void _lift_handler(k_id k, uint64_t f_ptr) {
    gen_fn f = (gen_fn)f_ptr;
    
    f((Generator *)control(_return_lift_result, k));
}

k_id lift(gen_fn f) {
    return control(_lift_handler, (uint64_t)f);
}



// Allocating a generator
Generator *make_generator(gen_fn gf) {
    Generator *g = (Generator *)malloc(sizeof(Generator));
    g->after_yield = lift(gf);
    return g;
}
void free_generator(Generator *g) {
    // continuation_delete(g->after_yield);
    free(g);
}


// Yielding implementation
void yield_handler(k_id k, Generator *g) {
    g->after_yield = k;
    restore(g->after_next, g->value);
}
void gen_yield(uint64_t v, Generator *g) {
    g->value = v;
    control(yield_handler, g);
}


// Next implementation
void next_handler(k_id k, Generator *g) {
    g->after_next = k;
    restore(g->after_yield, 0);
}

uint64_t gen_next(Generator *g) {
    return control(next_handler, g);
}




// **************** Example generator use *****************

void example_generator(Generator *g) {
    uint64_t i = 0;
    while(1) {
        gen_yield(i++, g);
    }
}
int main() {
    Generator *g = make_generator(example_generator);
    for(int i = 0; i < 10; i++) {
        printf("%llu\n", gen_next(g));
    }
    free_generator(g);
    return 0;
}


DONT_DELETE_MY_HANDLER(_return_lift_result)
DONT_DELETE_MY_HANDLER(_lift_handler)
DONT_DELETE_MY_HANDLER(yield_handler)
DONT_DELETE_MY_HANDLER(next_handler)
