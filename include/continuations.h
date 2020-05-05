#include <stdint.h>
#include <stdlib.h>

// #include <emscripten/emscripten.h>

typedef uint64_t k_id;
typedef void (*control_handler_fn)(k_id, uint64_t);

#ifdef __cplusplus
extern "C" {
#endif

// These get replaced with reads / writes to the global stack pointer
char * __prim_get_shadow_stack_ptr();
void __prim_set_shadow_stack_ptr(char *p);


char **cont_stack_table;
char *current_stack_top;

void table_set_stack_ptr(k_id k, char *sp) {
    cont_stack_table[2*k] = sp;
}
char * table_get_stack_ptr(k_id k) {
    return cont_stack_table[2*k];
}

void table_set_stack_top(k_id k, char *sp) {
    cont_stack_table[2*k+1] = sp;
}
char * table_get_stack_top(k_id k) {
    return cont_stack_table[2*k+1];
}



void EMSCRIPTEN_KEEPALIVE initialize_continuations() {
    cont_stack_table = malloc(sizeof(char *) * 100001);
}

void EMSCRIPTEN_KEEPALIVE __hook_control(k_id k, uint64_t arg) {
    // Need to: 		
    // 1. Save global stack pointer into a table at index k		
    // 2. Allocate new stack space
    // 3. Save `current_stack_top` into table at index k
    // 4. Set `current_stack_top` to the new stack top
    // 5. Set global stack pointer to new stack space top
    __prim_set_shadow_stack_ptr(__prim_get_shadow_stack_ptr());
}

void EMSCRIPTEN_KEEPALIVE __hook_restore(k_id k, uint64_t v) {
    // Need to: 
    // 1. Retrieve stack top from table at index k, and save in `current_stack_top`
    // 2. Set the global stack pointer to the stack pointer from the table at index k

}

void EMSCRIPTEN_KEEPALIVE __hook_copy(k_id k, k_id new_k) {
    // Need to:
    // 1. Allocate a new stack, save top into table at index new_k
    // 2. Retrieve global stack pointer from index k
    // 3. Compute new global stack pointer via offset (see C code), save in table at index new_k
    // 4. memcpy (see C code)
}

void EMSCRIPTEN_KEEPALIVE __hook_delete(k_id k) {
    // Need to: do nothing? free the stack?
}

// These get replaced with calls to the Wasm instructions
uint64_t __prim_control(uint64_t arg, control_handler_fn fn_ptr);
void __prim_restore(k_id k, uint64_t val);
uint64_t __prim_continuation_copy(k_id k);
void __prim_continuation_delete(k_id k);



// This is replaced with nop
int __prim_inhibit_optimizer();

void __shim_handler(k_id k, uint64_t arg);

#define DONT_DELETE_MY_HANDLER(handler_name) \
    void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
        control_handler_fn fptr = __prim_inhibit_optimizer() ? handler_name : 0; \
        fptr(0, 0); \
    } \


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


#define control(f, arg) __shim_control(f, arg)
#define restore(k, v) __shim_restore(k, v) // __prim_restore_pre_hook() __prim_restore(k, v)
#define continuation_copy(k) __shim_continuation_copy(k)
#define continuation_delete(k) __shim_continuation_delete(k)



#ifdef __cplusplus
}
#endif

// #define CONTROL(fn_ptr, arg)
