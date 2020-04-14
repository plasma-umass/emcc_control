#include "../include/continuations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include <map>
#include <iostream>

std::map<uint64_t, double> *count_probs(std::vector<uint64_t> *v) {
    auto m = new std::map<uint64_t, double>();
    for(auto it = v->begin(); it != v->end(); ++it) {
        auto f = m->find(*it);
        if(f == m->end()) {
            m->insert(std::pair<uint64_t, double>(*it, 1.0 / v->size()));
            // m->insert(std::pair<uint64_t, double>(*it, 1.0));
        } else {
            f->second += 1.0 / v->size();
            // f->second += 1.0;
        }
    }
    return m;
}

struct ContinuationThunk {
    k_id continuation;
    uint64_t value;
};

std::vector<ContinuationThunk *> rest;



void choose_handler(k_id k, uint64_t args_ptr_tmp) {
    std::vector<uint64_t> *args = (std::vector<uint64_t> *)args_ptr_tmp;
    
    for(auto it = std::next(args->begin()); it != args->end(); ++it) {
        rest.push_back(new ContinuationThunk {.continuation = continuation_copy(k), .value = *it});
    }

    uint64_t v = args->at(0);
    // delete args;
    
    restore(k, v);
}
DONT_DELETE_MY_HANDLER(choose_handler)

uint64_t choose_impl(std::vector<uint64_t> *args) {
    return control(choose_handler, (uint64_t)args);
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


typedef uint64_t (*body_fn)();


std::map<uint64_t, double> *driver(uint64_t (*body)()) {
    std::vector<uint64_t> *results = new std::vector<uint64_t>();
    results->push_back(body());

    if(rest.size() > 0) {
        ContinuationThunk *t = rest.back(); rest.pop_back();
        restore(t->continuation, t->value);
    }

    auto results_distr = count_probs(results);
    return results_distr;
}



template<class K, class V> std::ostream& operator<<(std::ostream& os, const std::map<K,V>& m) {
    os << "{";
    uint64_t c = 0;
    for(auto it = m.begin(); it != m.end(); ++it) {
        os << it->first << ": " << it->second;
        c++;
        if(c != m.size()) {
            os << ", ";
        }
    }
    os << "}";
    return os;
}

template<class T> std::ostream& operator<<(std::ostream& os, const std::vector<T *>& v) {
    os << "[";
    for(int i = 0; i < v.size(); i++) {
        os << *v[i];
        if(i != v.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

// *************** EXAMPLE USAGE *******************

uint64_t F() {
    return CHOOSE(1, 2, 3, 4, 5, 6);
}

uint64_t geom(double p) {
    uint64_t g = 1;
    while(true) {
        double r = (double)rand() / RAND_MAX;
        if(r < p) {
            break;
        }
        g++;
    }
    return g;
}

uint64_t even_dice_plus_geom() {
    auto *d6 = new std::vector<uint64_t> {1, 2, 3, 4, 5, 6};
    // uint64_t x = choose_impl(d6);
    // uint64_t y = choose_impl(d6);
    // if((x+y)%2 != 0) {
    //     FAILURE();
    // }

    // uint64_t g_05 = sample(100, 0.5, geom);
    // uint64_t g_08 = sample(100, 0.8, geom);
    
    // success(x + y + g_05 + g_08);
    return choose_impl(d6) + choose_impl(d6);
}


void the_main(k_id k, uint64_t u) {
    restore(k, (uint64_t)driver((body_fn)u));
}
DONT_DELETE_MY_HANDLER(the_main)


template<class T> void print_and_free(T *v) {
    std::cout << *v << std::endl;
    delete v;
}


int main() {
    print_and_free((std::map<uint64_t, double> *)control(the_main, (uint64_t)even_dice_plus_geom));

    return 0;
}