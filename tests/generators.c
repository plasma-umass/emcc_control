#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    k_id a;
    k_id b;
    uint64_t c;
} GeneratorEnv;

typedef void (*gen_fn)(GeneratorEnv *);

k_id _after_lift;
gen_fn _to_lift;



void _save_k_restore(void *vmctx, k_id k, void *arg) {
    RESTORE(_after_lift, k);
}
DONT_DELETE_MY_HANDLER(_save_k_restore)

void _lift_handler(void *vmctx, k_id k, void *arg) {
    _after_lift = k;
    gen_fn tmp = _to_lift;
    
    tmp((GeneratorEnv *)CONTROL(_save_k_restore, NULL));
}
DONT_DELETE_MY_HANDLER(_lift_handler)


k_id lift(gen_fn f) {
    _to_lift = f;
    return CONTROL(_lift_handler, NULL);
}





GeneratorEnv *alloc_env() {
    return (GeneratorEnv *)malloc(sizeof(GeneratorEnv));
}

void yield_handler(void *vmctx, k_id rest, GeneratorEnv *env) {
    env->b = rest;
    RESTORE(env->a, env->c);
}
DONT_DELETE_MY_HANDLER(yield_handler)


void gen_yield(uint64_t v, GeneratorEnv *env) {
    env->c = v;
    CONTROL(yield_handler, env);
}

void next_handler(void *vmctx, k_id k, GeneratorEnv *env) {
    env->a = k;
    RESTORE(env->b, (uint64_t)env);
}
DONT_DELETE_MY_HANDLER(next_handler)


uint64_t gen_next(GeneratorEnv *env) {
    return CONTROL(next_handler, env);
}

GeneratorEnv *make_generator(gen_fn gf) {
    GeneratorEnv *env = alloc_env();
    env->a = 0;
    env->b = lift(gf);
    return env;
}


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

    for(int i = 0; i < 10; i++) {
        printf("%llu\n", gen_next(env));
    }

    return 0;
}
/*
(
 
    (func $exampleGenerator (param $env i64) (local $i i64)
        (local.set $i (i64.const 0))
        (loop
            (call $gen_yield (local.get $i) (local.get $env))
            (local.set $i (i64.add (local.get $i) (i64.const 1)))
            br 0
        )
    )

    (func $the_main (export "the_main") (result i64) (local $n i64) (local $sum i64) (local $gen_env i64)
        (local.set $n (i64.const 0))
        (local.set $sum (i64.const 0))
        (local.set $gen_env (call $makeGenerator (i32.const 0)))

        (loop
            (local.set $sum 
                (i64.add 
                    (local.get $sum) 
                    (call $gen_next (local.get $gen_env))))
            
            (local.set $n (i64.add (local.get $n) (i64.const 1)))
            (br_if 0 (i64.ne (local.get $n) (i64.const 10)))
        )

        (local.get $sum)
    )
    )

*/