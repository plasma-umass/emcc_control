
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
    char *copied_stack_top;
    char *copied_stack_bottom;
    char *saved_sp;
    char *saved_current_stack_top;
    uint64_t num_bytes;
} Continuation;

Continuation *cont_table;

char *current_stack_top;


void set_current_stack_top(char *x) {
    // printf("Setting current stack top to: %p\n", x);
    current_stack_top = x;
}


void EMSCRIPTEN_KEEPALIVE initialize_continuations() {
    cont_table = (Continuation *)malloc(sizeof(Continuation) * CONT_TABLE_SIZE);
    // printf("alloc table: %d\n", cont_stack_table);
    set_current_stack_top(__prim_get_shadow_stack_ptr());
}

void EMSCRIPTEN_KEEPALIVE __hook_control(k_id k, uint64_t arg) {
    // Need to:
    // 0. Allocate new stack in continuation k.
    // 1. Copy memory in range [current_stack_top, current SP) to continuation structure k.
    // 2. Set length in continuation k
    // 3. Set new current_stack_top
    
    char *sp = __prim_get_shadow_stack_ptr();

    uint64_t num_bytes = (uint64_t)current_stack_top - (uint64_t)sp; // + 1 ?
    char *new_stack_bottom = (char *)malloc(sizeof(char) * num_bytes);
    char *new_stack_top = new_stack_bottom + num_bytes - 1;
    memcpy(new_stack_bottom, sp + 1 /* + 0 ? */, num_bytes);
    cont_table[k].copied_stack_bottom = new_stack_bottom;
    cont_table[k].copied_stack_top = new_stack_top;
    cont_table[k].num_bytes = num_bytes;
    cont_table[k].saved_sp = sp;
    cont_table[k].saved_current_stack_top = current_stack_top;

    set_current_stack_top(sp); // - 1 ? 
}




void EMSCRIPTEN_KEEPALIVE __hook_restore(k_id kid, uint64_t v) {
    // Need to:
    // 1. Copy copied stack back to (saved_sp, ...]
    // 2. Set sp to saved_sp
    // 3. Set current stack top to saved_current_stack_top
    // 4. Free stuff

    Continuation *k = &cont_table[kid];
    char *sp_to_restore = k->saved_sp;

    memcpy(sp_to_restore + 1 /* + 0 ? */, k->copied_stack_bottom, k->num_bytes);

    // set_current_stack_top(sp_to_restore + k->num_bytes);
    set_current_stack_top(k->saved_current_stack_top);

    __prim_set_shadow_stack_ptr(sp_to_restore);

    free(k->copied_stack_bottom);
}


void EMSCRIPTEN_KEEPALIVE __hook_copy(k_id kid, k_id new_kid) {
    // Need to:
    // 1. Allocate new stack
    // 2. Memcpy stack over
    // 3. Copy metadata

    Continuation *k = &cont_table[kid];
    Continuation *new_k = &cont_table[new_kid];
    new_k->copied_stack_bottom = (char *)malloc(sizeof(char) * k->num_bytes);
    new_k->copied_stack_top = new_k->copied_stack_bottom + k->num_bytes - 1;
    new_k->saved_sp = k->saved_sp;
    new_k->saved_current_stack_top = k->saved_current_stack_top;
    new_k->num_bytes = k->num_bytes;
    memcpy(new_k->copied_stack_bottom, k->copied_stack_bottom, k->num_bytes);

}

void EMSCRIPTEN_KEEPALIVE __hook_delete(k_id kid) {
    Continuation *k = &cont_table[kid];
    free(k->copied_stack_bottom);
}



control_handler_fn __global_control_tmp_f;
void __shim_handler(k_id k, uint64_t arg) {
    __hook_control(k, arg); 
    __global_control_tmp_f(k, arg);
}
DONT_DELETE_MY_HANDLER(__shim_handler)

uint64_t __shim_control(control_handler_fn f, uint64_t arg) {
    __global_control_tmp_f = f;
    return __prim_control(arg, __shim_handler);
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

