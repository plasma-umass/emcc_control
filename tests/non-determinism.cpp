#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include <iostream>


struct ContinuationThunk {
    k_id continuation;
    uint64_t value;
};


std::vector<ContinuationThunk *> rest;

DEFINE_HANDLER(choose_handler, k_id k, uint64_t args_ptr_tmp) {
    std::vector<uint64_t> *args = (std::vector<uint64_t> *)args_ptr_tmp;
    for(int i = 1; i < args->size(); i++) {
        ContinuationThunk *t = new ContinuationThunk();
        t->continuation = CONTINUATION_COPY(k);
        t->value = args->at(i);
        rest.push_back(t);
    }

    uint64_t v = args->at(0);
    delete args;
    
    RESTORE(k, v);
}
DONT_DELETE_MY_HANDLER(choose_handler)

uint64_t choose_impl(std::vector<uint64_t> *args) {
    return CONTROL(choose_handler, (uint64_t)args);
}

std::vector<uint64_t> *copy_array_vec(uint64_t *p, int n) {
    std::vector<uint64_t> *v = new std::vector<uint64_t>();
    for(int i = 0; i < n; i++) {
        v->push_back(p[i]);
    }

    return v;
}

#define CHOOSE(...) ([=]() -> uint64_t { \
        uint64_t __choices_tmp[] = {__VA_ARGS__}; \
        std::vector<uint64_t> *__choices = copy_array_vec(__choices_tmp, sizeof(__choices_tmp) / sizeof(uint64_t)); \
        return choose_impl(__choices); \
    }())


typedef uint64_t* (*body_fn)();

std::vector<uint64_t> *driver(body_fn body) {
    std::vector<uint64_t> *results = new std::vector<uint64_t>();
    uint64_t *vp = body();
    if(vp) {
        results->push_back(*vp);
        delete vp;
    }

    if(rest.size() > 0) {
        ContinuationThunk *t = rest.back();
        rest.pop_back();
        k_id k = t->continuation;
        uint64_t v = t->value;
        delete t;
        RESTORE(k, v);
    }
    return results;
}

uint64_t* SUCCESS(uint64_t x) {
    uint64_t *p = (uint64_t *)malloc(sizeof(uint64_t));
    *p = x;
    return p;
}

#define FAILURE nullptr;

std::ostream& operator<<(std::ostream& os, const std::vector<uint64_t>& v) {
    os << "[";
    for(int i = 0; i < v.size(); i++) {
        os << v[i];
        if(i != v.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}




// *************** EXAMPLE USAGE *******************

uint64_t* mult_ex() {
    return SUCCESS(CHOOSE(10, 20));
}

uint64_t F() {
    return CHOOSE(1, 2, 3, 4, 5, 6);
}

uint64_t* even_dice() {
    uint64_t x = F();
    uint64_t y = F();
    if((x+y)%2 != 0) {
        return FAILURE;
    }
    return SUCCESS(x + y);
}


// uint64_t choose_int(uint64_t low, uint64_t high) {
//     if(low > high) {
//         return FAILURE;
//     }
// }
// uint64_t *squares() {

// }

DEFINE_HANDLER(the_main, k_id k, uint64_t u) {
    RESTORE(k, (uint64_t)driver((body_fn)u));
}
DONT_DELETE_MY_HANDLER(the_main)


void print_and_free(std::vector<uint64_t> *v) {
    std::cout << *v << std::endl;
    delete v;
}
int main() {
    INIT_CONTINUATIONS_LIB();

    print_and_free((std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)mult_ex));
    print_and_free((std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)even_dice));

    return 0;
}