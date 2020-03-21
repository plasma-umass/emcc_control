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
        } else {
            f->second += 1.0 / v->size();
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
    RESTORE(answer_k, (uint64_t)p);
}

#define FAILURE() RESTORE(answer_k, 0);

DEFINE_HANDLER(choose_handler, k_id k, uint64_t args_ptr_tmp) {
    std::vector<uint64_t> *args = (std::vector<uint64_t> *)args_ptr_tmp;
    
    for(int i = 1; i < args->size(); i++) {
        ContinuationThunk *t = new ContinuationThunk();
        t->continuation = CONTINUATION_COPY(k);
        t->value = args->at(i);
        t->ans_cont = CONTINUATION_COPY(answer_k);
        rest.push_back(t);
    }

    uint64_t v = args->at(0);
    delete args;
    
    RESTORE(k, v);
}
DONT_DELETE_MY_HANDLER(choose_handler)

uint64_t choose_impl(std::vector<uint64_t> *args) {
    if(args->size() == 0) {
        FAILURE();
    }

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


typedef void (*body_fn)();


DEFINE_HANDLER(driver_handler, k_id k, uint64_t body_func_tmp) {
    body_fn body = (body_fn)body_func_tmp;
    answer_k = k;
    // body(k);
    body();
}
DONT_DELETE_MY_HANDLER(driver_handler)



std::map<uint64_t, double> *driver(body_fn body) {
    std::vector<uint64_t> *results = new std::vector<uint64_t>();

    uint64_t *vp = (uint64_t *)CONTROL(driver_handler, (uint64_t)body);
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
        RESTORE(k, v);
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
    // for(int i = 0; i < v.size(); i++) {
    //     os << v[i];
    //     if(i != v.size() - 1) {
    //         os << ", ";
    //     }
    // }
    // os << "]";
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


k_id get_answer_k() {
    return answer_k;
}

// *************** EXAMPLE USAGE *******************

uint64_t F() {
    return CHOOSE(1, 2, 3, 4, 5, 6);
}

void even_dice() {
    uint64_t x = F();
    uint64_t y = F();
    if((x+y)%2 != 0) {
        FAILURE();
    }
    SUCCESS(x + y);
}




DEFINE_HANDLER(the_main, k_id k, uint64_t u) {
    RESTORE(k, (uint64_t)driver((body_fn)u));
}
DONT_DELETE_MY_HANDLER(the_main)


template<class T> void print_and_free(T *v) {
    std::cout << *v << std::endl;
    delete v;
}


int main() {
    INIT_CONTINUATIONS_LIB();

    // print_and_free((std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)mult_ex));
    print_and_free((std::map<uint64_t, double> *)CONTROL(the_main, (uint64_t)even_dice));

    // std::vector<uint64_t> *trips_raw = (std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)pythag);
    // std::cout << cast_vec<PythagTriple *>(trips_raw) << std::endl;

    // std::vector<uint64_t> *queens_sol_raw = (std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)queens);
    
    // // print_and_free(queens_sol_raw);
    // int n = queens_sol_raw->size();
    // std::cout << cast_vec<std::vector<Coord *> *>(queens_sol_raw) << std::endl;
    // std::cout << n << std::endl;

    return 0;
}