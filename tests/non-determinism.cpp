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



std::vector<uint64_t> *driver(body_fn body) {
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
    return results;
}



template<class T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
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


// void ret_success(uint64_t x) {
//     RESTORE(answer_k, x);
// }

// *************** EXAMPLE USAGE *******************

void mult_ex() {
    // return SUCCESS(1234);
    // return SUCCESS(CHOOSE(10, 20));
    // RESTORE(k, (uint64_t)SUCCESS(CHOOSE(10, 20)));
    // k_id _a_tmp = answer_k;
    // answer_k = CONTINUATION_COPY(answer_k);
    // uint64_t x = (uint64_t)SUCCESS(CHOOSE(10, 20));
    // RESTORE(get_answer_k(), (uint64_t)SUCCESS(CHOOSE(10, 20)));
    SUCCESS(CHOOSE(10, CHOOSE(20, 30)) * CHOOSE(30, 40));
    // RESTORE(answer_k, (uint64_t)SUCCESS(1234));

}

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


uint64_t choose_int(uint64_t low, uint64_t high) {
    if(low > high) {
        FAILURE();
    }
    // int count = high - low + 1;
    std::vector<uint64_t> *choices = new std::vector<uint64_t>();
    for(uint64_t i = low; i <= high; i++) {
        choices->push_back(i);
    }
    return choose_impl(choices);
}

uint64_t find_sqrt(uint64_t x) {
    for(uint64_t i = 0; i <= x; i++) {
        if(i*i == x) {
            return i;
        }
    }
    FAILURE();
    return 0;
}

struct PythagTriple {
    uint64_t a, b, c;

    PythagTriple(uint64_t aa, uint64_t bb, uint64_t cc) : a(aa), b(bb), c(cc) {}
};


std::ostream& operator<<(std::ostream& os, const PythagTriple p) {
    // os << "[";
    // for(int i = 0; i < v.size(); i++) {
    //     os << v[i];
    //     if(i != v.size() - 1) {
    //         os << ", ";
    //     }
    // }
    os << "(" << p.a << ", " << p.b << ", " << p.c << ")";
    return os;
}

void pythag() {
    uint64_t m = 5;
    uint64_t n = 15;

    uint64_t a = choose_int(m, n-1);
    uint64_t b = choose_int(a, n);
    uint64_t c = find_sqrt(a*a + b*b);
    SUCCESS((uint64_t)(new PythagTriple(a, b, c)));
}


#define N_QUEENS 8

struct Coord {
    uint64_t x;
    uint64_t y;
};

std::ostream& operator<<(std::ostream& os, const Coord c) {
    os << "(" << c.x << ", " << c.y << ")";
    return os;
}


template<class T> struct LList {
    T value;
    LList<T> *next;
};



bool no_attack(uint64_t x, uint64_t y, uint64_t qx, uint64_t qy) {
    return x != qx && y != qy && abs((int)x - (int)qx) != abs((int)y - (int)qy);
}

std::vector<uint64_t> *available(uint64_t x, LList<Coord *> *qs) {
    std::vector<uint64_t> *a = new std::vector<uint64_t>(); // this gets deallocated inside choose_impl()
    for(uint64_t y = 1; y <= N_QUEENS; y++) {
        bool is_free = true;

        for(LList<Coord *> *it = qs; it != nullptr; it = it->next) {
            Coord *q = it->value;
            if(!no_attack(x, y, q->x, q->y)) {
                is_free = false;
                break;
            }
        }

        if(is_free) {
            a->push_back(y);
        }
    }

    return a;
}

template<class T> LList<T>* append(T q, LList<T>* qs) {
    LList<T> *n = new LList<T>();
    n->value = q;
    n->next = qs;
    return n;
}

void queens() {
    LList<Coord *> *qs = nullptr;

    for(uint64_t x = 1; x <= N_QUEENS; x++) {
        uint64_t y = choose_impl(available(x, qs));
        Coord *q = new Coord();
        q->x = x;
        q->y = y;
        qs = append(q, qs);
    }

    std::vector<Coord *> *qs_v = new std::vector<Coord *>();
    for(LList<Coord *> *it = qs; it != nullptr; it = it->next) {
        qs_v->push_back(it->value);
    }

    SUCCESS((uint64_t)qs_v);
}


DEFINE_HANDLER(the_main, k_id k, uint64_t u) {
    RESTORE(k, (uint64_t)driver((body_fn)u));
}
DONT_DELETE_MY_HANDLER(the_main)


template<class T> void print_and_free(std::vector<T> *v) {
    std::cout << *v << std::endl;
    delete v;
}

template<class U, class T> std::vector<U> cast_vec(std::vector<T> *v) {
    std::vector<U> n;
    for(auto i = v->begin(); i != v->end(); ++i) {
        n.push_back((U)*i);
    }
    delete v;
    return n;
}

int main() {
    INIT_CONTINUATIONS_LIB();

    print_and_free((std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)mult_ex));
    print_and_free((std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)even_dice));

    std::vector<uint64_t> *trips_raw = (std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)pythag);
    std::cout << cast_vec<PythagTriple *>(trips_raw) << std::endl;

    std::vector<uint64_t> *queens_sol_raw = (std::vector<uint64_t> *)CONTROL(the_main, (uint64_t)queens);
    
    // print_and_free(queens_sol_raw);
    int n = queens_sol_raw->size();
    std::cout << cast_vec<std::vector<Coord *> *>(queens_sol_raw) << std::endl;
    std::cout << n << std::endl;

    return 0;
}