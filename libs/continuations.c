
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <emscripten/emscripten.h>

#include "../include/continuations.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *copied_stack_bottom;
    char *saved_sp;
    char *saved_current_stack_top;
    uint64_t num_bytes;
} Continuation;

Continuation *cont_table;

char *current_stack_top;


char *c_stacks_area = NULL;

#define C_SMALL_STACK_SIZE 32 // 1024, 2^23, 8388608, 1048576
#define C_SMALL_STACK_TABLE_SIZE 64

uint64_t free_c_stack_id_list[C_SMALL_STACK_TABLE_SIZE];
uint64_t free_c_stack_id_list_top = 0; // From this index we will alloc the next stack id


void set_current_stack_top(char *x) {
    // printf("Setting current stack top to: %p\n", x);
    current_stack_top = x;
}


void initialize_continuations() {
    cont_table = (Continuation *)malloc(sizeof(Continuation) * CONT_TABLE_SIZE);

    c_stacks_area = malloc(C_SMALL_STACK_SIZE * C_SMALL_STACK_TABLE_SIZE);
    for(int i = 0; i < C_SMALL_STACK_TABLE_SIZE; i++) {
        free_c_stack_id_list[i] = i;
    }
    // printf("alloc table: %d\n", cont_stack_table);
    set_current_stack_top(__prim_get_shadow_stack_ptr());
}

void *stack_alloc(size_t size) {
    if(size > 1024) {
        // printf("fast1\n");
        return malloc(size);
    } else {
        if(free_c_stack_id_list_top == C_SMALL_STACK_TABLE_SIZE) {
            // printf("Error: out of stacks to allocate.\n");
            // abort();
            // printf("fast1\n");
            return malloc(size);
        } else {
            uint64_t id = free_c_stack_id_list[free_c_stack_id_list_top++];
            uint64_t stack_base = (uint64_t)((void *)c_stacks_area) + C_SMALL_STACK_SIZE * id;
            return (void *)stack_base;
        }        
    }
}



void stack_free(void *p) {
    if(p >= (void *)c_stacks_area && p < (void *)c_stacks_area + C_SMALL_STACK_SIZE * C_SMALL_STACK_TABLE_SIZE) {
        uint64_t id = ((uint64_t)p - (uint64_t)((void *)c_stacks_area)) / C_SMALL_STACK_SIZE;
        free_c_stack_id_list[--free_c_stack_id_list_top] = id;
    } else {
        // printf("fast2\n");
        free(p);
    }
}

void __hook_control(k_id k, uint64_t arg) {
    #if NO_C_STACK == 1
    return;
    #endif
    // Need to:
    // 0. Allocate new stack in continuation k.
    // 1. Copy memory in range [current_stack_top, current SP) to continuation structure k.
    // 2. Set length in continuation k
    // 3. Set new current_stack_top
    
    char *sp = __prim_get_shadow_stack_ptr();

    uint64_t num_bytes = (uint64_t)current_stack_top - (uint64_t)sp; // + 1 ?

    char *new_stack_bottom = (char *)stack_alloc(sizeof(char) * num_bytes);
    char *new_stack_top = new_stack_bottom + num_bytes - 1;
    memcpy(new_stack_bottom, sp + 1 /* + 0 ? */, num_bytes);

    // for(int i = 0; i < num_bytes; i++) {
    //     new_stack_bottom[i] = sp[i+1];
    // }

    cont_table[k].copied_stack_bottom = new_stack_bottom;
    cont_table[k].num_bytes = num_bytes;
    cont_table[k].saved_sp = sp;
    cont_table[k].saved_current_stack_top = current_stack_top;

    // set_current_stack_top(sp); // - 1 ? 
    current_stack_top = sp;
}




void __hook_restore(k_id kid, uint64_t v) {
    // Need to:
    // 1. Copy copied stack back to (saved_sp, ...]
    // 2. Set sp to saved_sp
    // 3. Set current stack top to saved_current_stack_top
    // 4. Free stuff

    Continuation *k = &cont_table[kid];
    char *sp_to_restore = k->saved_sp;

    memcpy(sp_to_restore + 1 /* + 0 ? */, k->copied_stack_bottom, k->num_bytes);
    // int nb = k->num_bytes;
    // char *sb = k->copied_stack_bottom;
    // for(int i = 0; i < nb; i++) {
    //     sp_to_restore[i+1] = sb[i];
    // }

    // set_current_stack_top(sp_to_restore + k->num_bytes);
    // set_current_stack_top(k->saved_current_stack_top);
    current_stack_top = k->saved_current_stack_top;

    __prim_set_shadow_stack_ptr(sp_to_restore);

    stack_free(k->copied_stack_bottom);
}


void __hook_copy(k_id kid, k_id new_kid) {
    // Need to:
    // 1. Allocate new stack
    // 2. Memcpy stack over
    // 3. Copy metadata

    Continuation *k = &cont_table[kid];
    Continuation *new_k = &cont_table[new_kid];
    new_k->copied_stack_bottom = (char *)stack_alloc(sizeof(char) * k->num_bytes);
    new_k->saved_sp = k->saved_sp;
    new_k->saved_current_stack_top = k->saved_current_stack_top;
    new_k->num_bytes = k->num_bytes;
    memcpy(new_k->copied_stack_bottom, k->copied_stack_bottom, k->num_bytes);

}

void __hook_delete(k_id kid) {
    Continuation *k = &cont_table[kid];
    stack_free(k->copied_stack_bottom);
}



// control_handler_fn __global_control_tmp_f;
// void __shim_handler(k_id k, uint64_t arg) {
//     __hook_control(k, arg); 
//     __global_control_tmp_f(k, arg);
// }
// DONT_DELETE_MY_HANDLER(__shim_handler)

uint64_t __shim_control(control_handler_fn f, uint64_t arg) {
    // __global_control_tmp_f = f;
    // return __prim_control(arg, __shim_handler);
    return __prim_control(arg, f);
}


void __shim_restore(k_id k, uint64_t v) {
    __hook_restore(k, v);
    __prim_restore(k, v);
}


k_id __shim_continuation_copy(k_id k) {
    uint64_t new_k = __prim_continuation_copy(k);
    __hook_copy(k, new_k);
    return new_k;
}

void __shim_continuation_delete(k_id k) {
    __prim_continuation_delete(k);
    __hook_delete(k);
}


#ifdef __cplusplus
}
#endif

