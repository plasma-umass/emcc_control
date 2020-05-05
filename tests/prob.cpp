#include <emscripten/emscripten.h>
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
    k_id ans_cont;
};


std::vector<ContinuationThunk *> rest;
k_id answer_k;


void SUCCESS(uint64_t x) {
    uint64_t *p = (uint64_t *)malloc(sizeof(uint64_t));
    *p = x;
    restore(answer_k, (uint64_t)p);
}

#define FAILURE() restore(answer_k, 0);

void trials_handler(k_id k, uint64_t n) {    
    for(int i = 1; i < n; i++) {
        ContinuationThunk *t = new ContinuationThunk();
        t->continuation = continuation_copy(k);
        t->value = 0;
        t->ans_cont = continuation_copy(answer_k);
        rest.push_back(t);
    }
    restore(k, 0);
}
DONT_DELETE_MY_HANDLER(trials_handler)

void trials(uint64_t n) {
    if(n == 0) {
        FAILURE();
    } else if(n == 1) {
        return; // small optimization
    }
    control(trials_handler, n);
}

void choose_handler(k_id k, uint64_t args_ptr_tmp) {
    std::vector<uint64_t> *args = (std::vector<uint64_t> *)args_ptr_tmp;
    
    for(int i = 1; i < args->size(); i++) {
        ContinuationThunk *t = new ContinuationThunk();
        t->continuation = continuation_copy(k);
        t->value = args->at(i);
        t->ans_cont = continuation_copy(answer_k);
        rest.push_back(t);
    }

    uint64_t v = args->at(0);
    delete args;
    
    restore(k, v);
}
DONT_DELETE_MY_HANDLER(choose_handler)

uint64_t choose_impl(std::vector<uint64_t> *args) {
    if(args->size() == 0) {
        FAILURE();
    }

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


typedef void (*body_fn)();


void driver_handler(k_id k, uint64_t body_func_tmp) {
    body_fn body = (body_fn)body_func_tmp;
    answer_k = k;
    // body(k);
    body();
}
DONT_DELETE_MY_HANDLER(driver_handler)



std::map<uint64_t, double> *driver(body_fn body) {
    std::vector<uint64_t> *results = new std::vector<uint64_t>();

    uint64_t *vp = (uint64_t *)control(driver_handler, (uint64_t)body);
    if(vp) {
        results->push_back(*vp);
        delete vp;
    }

    if(rest.size() > 0) {
        ContinuationThunk *t = rest.back();
        rest.pop_back();
        k_id k = t->continuation;
        uint64_t v = t->value;
        answer_k = t->ans_cont;

        delete t;
        restore(k, v);
    }

    auto results_prob = count_probs(results);
    delete results;
    return results_prob;
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

// typedef template<class T> uint64_t (*sample_fn)(T v);

template<class T> uint64_t sample(uint64_t n, T v, uint64_t (*f)(T)) {
    trials(n);
    return f(v);
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

void even_dice_plus_geom() {
    uint64_t x = F();
    uint64_t y = F();
    if((x+y)%2 != 0) {
        FAILURE();
    }

    uint64_t g_05 = sample(100, 0.5, geom);
    uint64_t g_08 = sample(100, 0.8, geom);
    
    SUCCESS(x + y + g_05 + g_08);
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