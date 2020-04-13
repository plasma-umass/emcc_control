#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    k_id after_next;
    k_id after_yield;
    uint64_t value;
} GeneratorEnv;

typedef void (*gen_fn)(GeneratorEnv *);

// Helpers for lifting a function up to a continuation
// Given a function f with 1 parameter, lift(f) returns a continuation id k
// such that restore(k, x) executes f(x).
void _return_lift_result(k_id k, uint64_t _after_lift_kid) {
    restore(_after_lift_kid, k);
}

void _lift_handler(k_id k, uint64_t f_ptr) {
    gen_fn f = (gen_fn)f_ptr;
    
    f((GeneratorEnv *)control(_return_lift_result, k));
}

k_id lift(gen_fn f) {
    return control(_lift_handler, (uint64_t)f);
}



// Yielding implementation

void yield_handler(k_id rest, uint64_t env_tmp) {
    GeneratorEnv *env = (GeneratorEnv *)env_tmp;
    env->after_yield = rest;
    restore(env->after_next, env->value);
}

void gen_yield(uint64_t v, GeneratorEnv *env) {
    env->value = v;
    control(yield_handler, (uint64_t)env);
}


// Next implementation

void next_handler(k_id k, uint64_t env_tmp) {
    GeneratorEnv *env = (GeneratorEnv *)env_tmp;
    env->after_next = k;
    restore(env->after_yield, (uint64_t)env);
}

uint64_t gen_next(GeneratorEnv *env) {
    return control(next_handler, (uint64_t)env);
}


// Allocating a generator
GeneratorEnv *make_generator(gen_fn gf) {
    GeneratorEnv *env = (GeneratorEnv *)malloc(sizeof(GeneratorEnv));
    env->after_yield = lift(gf);
    return env;
}



// **************** Example generator use *****************

void example_generator(GeneratorEnv *env) {
    uint64_t i = 0;
    while(1) {
        gen_yield(i, env);
        i++;
    }
}

int main() {
    GeneratorEnv *env = make_generator(example_generator);

    for(int i = 0; i < 100; i++) {
        printf("%llu\n", gen_next(env));
    }

    return 0;
}


DONT_DELETE_MY_HANDLER(_return_lift_result)
DONT_DELETE_MY_HANDLER(_lift_handler)
DONT_DELETE_MY_HANDLER(yield_handler)
DONT_DELETE_MY_HANDLER(next_handler)
