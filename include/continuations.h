#include <stdint.h>

#include <emscripten/emscripten.h>

typedef uint64_t k_id;
typedef void (*control_handler_fn)(k_id, uint64_t);

#ifdef __cplusplus
extern "C" {
#endif

uint64_t __prim_control(uint64_t arg, control_handler_fn fn_ptr);
void __prim_restore(k_id k, uint64_t val);
uint64_t __prim_continuation_copy(k_id k);

int __prim_inhibit_optimizer();

#define DONT_DELETE_MY_HANDLER(handler_name) \
    void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
        control_handler_fn fptr = __prim_inhibit_optimizer() ? handler_name : 0; \
        fptr(0, 0); \
    } \

#define control(f, arg) __prim_control(arg, f)
#define restore(k, v) __prim_restore(k, v)
#define continuation_copy(k) __prim_continuation_copy(k)



#ifdef __cplusplus
}
#endif

// #define CONTROL(fn_ptr, arg)
