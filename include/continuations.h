#include <stdint.h>

// #include <emscripten/emscripten.h>

typedef uint64_t k_id;
typedef void (*control_handler_fn)(k_id, uint64_t);

#ifdef __cplusplus
extern "C" {
#endif

// These get replaced with calls to the Wasm instructions
uint64_t __prim_control(uint64_t arg, control_handler_fn fn_ptr);
void __prim_restore(k_id k, uint64_t val);
uint64_t __prim_continuation_copy(k_id k);
void __prim_continuation_delete(k_id k);

// These get replaced with inline Wasm code
void __prim_hook_control_post();
void __prim_hook_restore_pre();
void __prim_hook_copy_post();
void __prim_hook_delete_post();

// These get replaced with calls to corresponding hook
void __noinline_hook_control(k_id k, uint64_t arg);
void __noinline_hook_restore(k_id k, uint64_t v);
void __noinline_hook_copy(k_id k, k_id new_k);
void __noinline_hook_delete(k_id k);


// This is replaced with nops
int __prim_inhibit_optimizer();

void __shim_handler(k_id k, uint64_t arg);

#define DONT_DELETE_MY_HANDLER(handler_name) \
    void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
        control_handler_fn fptr = __prim_inhibit_optimizer() ? handler_name : 0; \
        fptr(0, 0); \
    } \

void EMSCRIPTEN_KEEPALIVE __hook_control(k_id k, uint64_t arg) {
    __prim_hook_control_post();
}


control_handler_fn __global_control_tmp_f;
void __shim_handler(k_id k, uint64_t arg) {
    __noinline_hook_control(k, arg);  
    __global_control_tmp_f(k, arg);
}
DONT_DELETE_MY_HANDLER(__shim_handler)

uint64_t __shim_control(control_handler_fn f, uint64_t arg) {
    __global_control_tmp_f = f;
    return __prim_control(arg, __shim_handler);
}

void EMSCRIPTEN_KEEPALIVE __hook_restore(k_id k, uint64_t v) {
    __prim_hook_restore_pre();
}

void __shim_restore(k_id k, uint64_t v) {
    __noinline_hook_restore(k, v);
    __prim_restore(k, v);
}

void EMSCRIPTEN_KEEPALIVE __hook_copy(k_id k, k_id new_k) {
    __prim_hook_copy_post();
}


k_id __shim_continuation_copy(k_id k) {
    uint64_t new_k = __prim_continuation_copy(k);
    __noinline_hook_copy(k, new_k);
    return new_k;
}

void EMSCRIPTEN_KEEPALIVE __hook_delete(k_id k) {
    __prim_hook_delete_post();
}

void __shim_continuation_delete(k_id k) {
    __prim_continuation_delete(k);
    __noinline_hook_delete(k);
}

#define control(f, arg) __shim_control(f, arg)
#define restore(k, v) __shim_restore(k, v) // __prim_restore_pre_hook() __prim_restore(k, v)
#define continuation_copy(k) __shim_continuation_copy(k)
#define continuation_delete(k) __shim_continuation_delete(k)



#ifdef __cplusplus
}
#endif

// #define CONTROL(fn_ptr, arg)
