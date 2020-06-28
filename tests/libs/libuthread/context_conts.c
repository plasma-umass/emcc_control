// #include <emscripten/emscripten.h>
#include "../../../include/continuations.h"

DEFINE_HANDLER(save_k_restore, k, after_capture, {
	restore(after_capture, k);
})

uthread_func_t _to_capture;
DEFINE_HANDLER(init_handler, k, arg, {
	uthread_func_t my_to_capture = _to_capture;

	control(save_k_restore, k);
	my_to_capture((void *)arg);
	uthread_exit(0);
})

k_id context_init_helper(uthread_func_t f, void *arg) {
	_to_capture = f;
	return control(init_handler, (uint64_t)arg);
}

void context_init(k_id *k, uthread_func_t f, void *arg) {
	*k = context_init_helper(f, arg);
}

k_id restore_to;
DEFINE_HANDLER(switch_handler, k, arg, {
	*((k_id *)arg) = k;
	restore(restore_to, 0);
})

void context_switch(k_id *from, k_id to) {
	restore_to = to;
	control(switch_handler, (uint64_t)from);
}

void context_initialize_lib() {
	initialize_continuations();
}

// DONT_DELETE_MY_HANDLER(switch_handler);
// DONT_DELETE_MY_HANDLER(save_k_restore);
// DONT_DELETE_MY_HANDLER(init_handler);

void context_main(void (*f)(int, char**), int argc, char **argv) {
	f(argc, argv);
}