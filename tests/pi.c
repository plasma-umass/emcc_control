#include "libs/libuthread/uthread.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//                268435456
#define NUM_THREADS 16
#define TERMS_PER_YIELD 1

typedef struct {
    uint64_t from;
    uint64_t to;
    double result;
    uthread_t tid;
} TermsArg;

double term(double kf, uint64_t ki) {
    int64_t sign = 2 * -((int64_t)ki % 2 ) + 1;
    double res = 4 * sign / (2*kf + 1);;

    if(ki % TERMS_PER_YIELD == 0) {
        uthread_yield();
    }

    return res;
}

int terms(void *arg_tmp) {
    TermsArg *arg = (TermsArg *)arg_tmp;
    double f = 0;
    uint64_t from = arg->from;
    uint64_t to = arg->to;

    for(uint64_t k = from; k <= to; k++) {
        f += term(k, k);
        // printf("tid = %d\n", arg->tid);
    }

    arg->result = f;

    return 0;
}


uint64_t exp2_int(uint64_t x) {
    uint64_t y = 1;
    for(uint64_t i = 0; i < x; i++) {
        y *= 2;
    }
    return y;
}


void the_main(void *argv_ptr) {
    char **argv = argv_ptr;

    uint64_t NUM_TERMS = exp2_int(atoi(argv[1]));
    

    TermsArg threads[NUM_THREADS];
    int termsPerThread = NUM_TERMS / NUM_THREADS;


    for(int thread = 0; thread < NUM_THREADS; thread++) {
        threads[thread].from = thread * termsPerThread;
        threads[thread].to = termsPerThread + thread*termsPerThread - 1;
        uthread_create(&threads[thread].tid, terms, &threads[thread]);
    }

    double pi = 0;
    for(int thread = 0; thread < NUM_THREADS; thread++) {
        uthread_join(threads[thread].tid, NULL);
        // printf("Done with tid = %d\n", threads[thread].tid);
        pi += threads[thread].result;
    }

    printf("%f\n", pi);

    // say_hi();
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Expected 1 arg: log2(# terms)\n");
        return 1;
    }

    uthread_init_main(the_main, (void *)argv);
}