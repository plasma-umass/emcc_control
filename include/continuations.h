#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif


typedef void (*control_handler_fn)(uint64_t, void *);
typedef uint64_t k_id;


#ifdef __EMSCRIPTEN__

uint64_t __prim_control(void *arg, control_handler_fn fn_ptr);
void __prim_restore(k_id k, uint64_t val);
uint64_t __prim_continuation_copy(k_id k);

int __prim_inhibit_optimizer();

#define DONT_DELETE_MY_HANDLER(handler_name) \
    void EMSCRIPTEN_KEEPALIVE __garbage_please_delete_me_##handler_name() { \
        void (*fptr)(uint64_t, uint64_t) = __prim_inhibit_optimizer() ? handler_name : 0; \
        fptr(0, 1); \
    }

#define CONTROL(f, arg) __prim_control(arg, (control_handler_fn)f)
#define RESTORE(k, v) __prim_restore(k, v)
#define CONTINUATION_COPY(k) __prim_continuation_copy(k)
#define INIT_CONTINUATIONS_LIB()

#else

extern uint64_t control(control_handler_fn fn_ptr, void *arg, void *vmctx);
extern void restore(k_id k, uint64_t val, void *vmctx);
extern uint64_t continuation_copy(k_id k, void *vmctx);
extern void init_table(void);

#define DONT_DELETE_MY_HANDLER(handler_name)

#define CONTROL(f, arg) control((control_handler_fn)f, arg, 0)
#define RESTORE(k, v) restore(k, v, 0)
#define CONTINUATION_COPY(k) continuation_copy(k, 0)
#define INIT_CONTINUATIONS_LIB() init_table()

#endif

// #define CONTROL(fn_ptr, arg)
