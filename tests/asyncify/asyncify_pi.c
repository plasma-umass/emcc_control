#include "../../include/asyncify.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../libs/libuthread/queue.h"

#define NUM_THREADS 4


uint64_t termsPerYield = 1;
uint64_t termsPerThread;

int active_thread;
char sleeping = 0;

uint32_t asyncify_bufs[NUM_THREADS];
double results[NUM_THREADS];

queue_t Q;

struct TCB {
	int tid;
};

void enqueue(int tid) {
    struct TCB *tcb = malloc(sizeof(struct TCB));
    tcb->tid = tid;
    queue_enqueue(Q, tcb);
}

int dequeue() {
    struct TCB *tcb;
    queue_dequeue(Q, (void **)&tcb);
    int tid = tcb-> tid;
    free(tcb);
    return tid;
}

int queue_len() {
    return queue_length(Q);
}


void my_sleep(int tid) {
    if(sleeping == 0) {
        sleeping = 1;
        asyncify_start_unwind(asyncify_bufs[tid]);
    } else { 
        asyncify_stop_rewind();
        sleeping = 0;
    }
}

double term(double k, int tid, uint64_t ki) {

    int64_t sign = 2 * -((int64_t)ki % 2 ) + 1;
    double res = 4 * sign / (2*k + 1);

    if(ki % termsPerYield == 0) {
        my_sleep(tid);
    }
    return res;
}

void terms() {

    int tid = active_thread;
    uint64_t from = active_thread * termsPerThread;
    uint64_t to = termsPerThread + (active_thread * termsPerThread) - 1;
    double f = 0;

    for(uint64_t k = from; k <= to; k++) {
        f += term((double)k, tid, k);
        // printf("%d,%d\n", tid, (int)k);
        // my_sleep(tid);
        // printf("   %d,%d\n", tid, (int)k);
    }

    results[tid] = f;
}

void the_main() {
    terms();
}


void scheduler() {
    if(sleeping) {
        enqueue(active_thread);
    }
    active_thread = dequeue();
}

uint64_t exp2_int(uint64_t x) {
    uint64_t y = 1;
    for(uint64_t i = 0; i < x; i++) {
        y *= 2;
    }
    return y;
}

int main(int argc, char ** argv) {
    uint64_t NUM_TERMS = exp2_int(atoi(argv[1]));
    termsPerThread = NUM_TERMS / NUM_THREADS;
    termsPerYield = exp2_int(atoi(argv[2]));

    Q = queue_create();
    for(int tid = 0; tid < NUM_THREADS; tid++) {
        asyncify_bufs[tid] = alloc_asyncify_buf();
    }

    

    for(int tid = 0; tid < NUM_THREADS; tid++) {
        active_thread = tid;
        sleeping = 0;
        the_main();
        asyncify_stop_unwind();
        enqueue(tid);   
    }


    while(queue_len() != 0) {
        int next_tid = dequeue();
        active_thread = next_tid;
        asyncify_start_rewind(asyncify_bufs[active_thread]);
        the_main();
        if(sleeping) {
            asyncify_stop_unwind();
            enqueue(active_thread);
        }
    }


    double sum = 0;
    for(int tid = 0; tid < NUM_THREADS; tid++) {
        sum += results[tid];
    }

    // if(sum > 0.5) {
    //     printf("Big!\n");
    // }

    printf("%f\n", sum);
}