#ifndef _CONTINUATIONS_H
#define _CONTINUATIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #include <emscripten/emscripten.h>

typedef uint64_t k_id;
typedef void (*control_handler_fn)(k_id, uint64_t);

#ifdef __cplusplus
extern "C" {
#endif

// These get replaced with reads / writes to the global stack pointer
char * __prim_get_shadow_stack_ptr();
void __prim_set_shadow_stack_ptr(char *p);

#define CONT_TABLE_SIZE 100001
#define STACK_SIZE 1048576 // 1024, 2^23, 8388608, 1048576


// These get replaced with calls to the Wasm instructions
uint64_t __prim_control(uint64_t arg, control_handler_fn fn_ptr);
void __prim_restore(k_id k, uint64_t val);
uint64_t __prim_continuation_copy(k_id k);
void __prim_continuation_delete(k_id k);
void __prim_prompt_begin();
void __prim_prompt_end();



// This is replaced with nop
int __prim_inhibit_optimizer();

// uint64_t __shim_control(control_handler_fn f, uint64_t arg);
void __shim_restore(k_id k, uint64_t v);
k_id __shim_continuation_copy(k_id k);
void __shim_continuation_delete(k_id k);


// #define DONT_DELETE_MY_HANDLER(handler_name) \
//     void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
//         control_handler_fn fptr = __prim_inhibit_optimizer() ? handler_name : 0; \
//         fptr(0, 0); \
//     } \

#define DONT_DELETE_MY_HANDLER(handler_name)


#if NO_C_STACK == 1
#define control(f, arg) __prim_control(arg, f)
#define restore(k, v) __prim_restore(k, v) // __prim_restore_pre_hook() __prim_restore(k, v)
#define continuation_copy(k) __prim_continuation_copy(k)
#define continuation_delete(k) __prim_continuation_delete(k)
#define DEFINE_HANDLER(name, k_name, arg_name, body) void name(k_id k_name, uint64_t arg_name) { body } DONT_DELETE_MY_HANDLER(name);

#else
#define control(f, arg) __prim_control(arg, f)
#define restore(k, v) __shim_restore(k, v)
#define continuation_copy(k) __shim_continuation_copy(k)
#define continuation_delete(k) __shim_continuation_delete(k)

#define DEFINE_HANDLER(name, k_name, arg_name, body) void name(k_id k_name, uint64_t arg_name) { __hook_control(k_name); body } DONT_DELETE_MY_HANDLER(name);

// void save_k_restore(k_id k, uint64_t after_capture) {
// 	__hook_control(k, after_capture);

// 	restore(after_capture, k);
// }
#endif

#define prompt(x) __prim_prompt_begin(); x; __prim_prompt_end();

void initialize_continuations();
void __hook_control(k_id k);


#ifdef __cplusplus
}
#endif

#endif
