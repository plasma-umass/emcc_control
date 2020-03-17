#include <stdint.h>

// pub fn control(fn_ptr: *mut u64, arg: u64, vm: *mut u64) -> u64;

uint64_t __prim_control(uint64_t arg, void (*fn_ptr)(uint64_t, uint64_t));
void __prim_restore(uint64_t k, uint64_t val);
uint64_t __prim_continuation_copy(uint64_t kid);

// pub fn restore(k: u64, val: u64, vm: *mut u64);
//uint64_t continuation_copy(uint64_t kid) {

int __prim_inhibit_optimizer();

#define DONT_DELETE_MY_HANDLER(handler_name) \
    void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
        void (*fptr)(uint64_t, uint64_t) = __prim_inhibit_optimizer() ? handler_name : 0; \
        fptr(0, 1); \
    }

#define CONTROL(f, arg) __prim_control(arg, f)
#define RESTORE(k, v) __prim_restore(k, v)
#define CONTINUATION_COPY(k) __prim_continuation_copy(k)

// #define CONTROL(fn_ptr, arg)
