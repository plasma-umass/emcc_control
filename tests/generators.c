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
k_id _after_lift;
gen_fn _to_lift;

DEFINE_HANDLER(_save_k_restore, k_id k, uint64_t arg) {
    RESTORE(_after_lift, k);
}
DONT_DELETE_MY_HANDLER(_save_k_restore)

DEFINE_HANDLER(_lift_handler, k_id k, uint64_t arg) {
    _after_lift = k;
    gen_fn tmp = _to_lift;
    
    tmp((GeneratorEnv *)CONTROL(_save_k_restore, 0));
}
DONT_DELETE_MY_HANDLER(_lift_handler)

k_id lift(gen_fn f) {
    _to_lift = f;
    return CONTROL(_lift_handler, 0);
}



// Yielding implementation

DEFINE_HANDLER(yield_handler, k_id rest, uint64_t env_tmp) {
    GeneratorEnv *env = (GeneratorEnv *)env_tmp;
    env->after_yield = rest;
    RESTORE(env->after_next, env->value);
}
DONT_DELETE_MY_HANDLER(yield_handler)

void gen_yield(uint64_t v, GeneratorEnv *env) {
    env->value = v;
    CONTROL(yield_handler, (uint64_t)env);
}


// Next implementation

DEFINE_HANDLER(next_handler, k_id k, uint64_t env_tmp) {
    GeneratorEnv *env = (GeneratorEnv *)env_tmp;
    env->after_next = k;
    RESTORE(env->after_yield, (uint64_t)env);
}
DONT_DELETE_MY_HANDLER(next_handler)

uint64_t gen_next(GeneratorEnv *env) {
    return CONTROL(next_handler, (uint64_t)env);
}


// Allocating a generator
GeneratorEnv *make_generator(gen_fn gf) {
    GeneratorEnv *env = malloc(sizeof(GeneratorEnv));
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
    INIT_CONTINUATIONS_LIB();

    GeneratorEnv *env = make_generator(example_generator);

    for(int i = 0; i < 100; i++) {
        printf("%llu\n", gen_next(env));
    }

    return 0;
}
