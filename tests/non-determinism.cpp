#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

struct ContinuationThunk {
    k_id continuation;
    uint64_t value;
};

std::vector<ContinuationThunk> rest;

DEFINE_HANDLER(choose_handler, k_id k, uint64_t args_ptr_tmp) {
    std::vector<uint64_t> *args = (std::vector<uint64_t> *)args_ptr_tmp;
    for(int i = 1; i < args->size(); i++) {
        ContinuationThunk t;
        t.continuation = k;
        t.value = args->at(i);
        rest.push_back(t);
    }
    
    RESTORE(k, args->at(0));
}
DONT_DELETE_MY_HANDLER(choose_handler)

uint64_t choose(std::vector<uint64_t> args) {
    return CONTROL(choose_handler, (uint64_t)&args);
}


typedef uint64_t (*body_fn)();

std::vector<uint64_t> driver(body_fn body) {
    std::vector<uint64_t> results;
    results.push_back(body());

    if(rest.size() > 0) {
        ContinuationThunk t = rest.back();
        rest.pop_back();
        RESTORE(t.continuation, t.value);
    }
    return results;
}

uint64_t example() {
    return 234;
}

void print_vec(std::vector<uint64_t> v) {
    for(int i = 0; i < v.size(); i++) {
        std::cout << v[i] << ' ';
    }
    std::cout << std::endl;
}

int main() {
    print_vec(driver(example));
    return 0;
}