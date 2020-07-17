#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

std::vector<uint64_t> queue;
uint64_t after_join;
uint64_t dequeue() {
    uint64_t next_k = queue.back(); queue.pop_back(); return next_k;
}

DEFINE_HANDLER(save_fk_restore, fk, create_k, {
    restore(create_k, fk);
})
DEFINE_HANDLER(create_handler, k, f, {    
    control(save_fk_restore, k);
    ((void (*)())f)();
    if(queue.size() > 0) {
        restore(dequeue(), 0);
    } else {
        restore(after_join, 0);
    }
})
void thread_create(void (*f)()) {
    queue.insert(queue.begin(), control(create_handler, (uint64_t)f));
}

DEFINE_HANDLER(join_handler, k, arg, {
    after_join = k;
    restore(dequeue(), 0);
})
void join_all_threads() {
    control(join_handler, 0);
}

DEFINE_HANDLER(yield_handler, k, arg, {
    queue.insert(queue.begin(), k);
    restore(dequeue(), 0);
})
void yield() {
    control(yield_handler, 0);
}

void thread_main() {
    std::cout << "A" << std::endl;
    yield();
    std::cout << "B" << std::endl;
}

int main() {
    initialize_continuations();
    thread_create(thread_main);
    thread_create(thread_main);
    join_all_threads();
}