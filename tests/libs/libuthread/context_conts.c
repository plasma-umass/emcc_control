#include <emscripten/emscripten.h>
#include "../../../include/continuations.h"

void save_k_restore(k_id k, uint64_t after_capture) {
	restore(after_capture, k);
}

uthread_func_t _to_capture;
void init_handler(k_id k, uint64_t arg) {
	uthread_func_t my_to_capture = _to_capture;

	control(save_k_restore, k);
	uthread_exit(my_to_capture((void *)arg));

}

k_id context_init_helper(uthread_func_t f, void *arg) {
	_to_capture = f;
	return control(init_handler, (uint64_t)arg);
}

void context_init(k_id *k, uthread_func_t f, void *arg) {
	*k = context_init_helper(f, arg);
}

k_id restore_to;
void switch_handler(k_id k, uint64_t arg) {
	*((k_id *)arg) = k;
	restore(restore_to, 0);
}

void context_switch(k_id *from, k_id *to) {
	restore_to = *to;
	control(switch_handler, (uint64_t)from);
}

void context_initialize_lib() {
	initialize_continuations();
}

DONT_DELETE_MY_HANDLER(switch_handler);
DONT_DELETE_MY_HANDLER(save_k_restore);
DONT_DELETE_MY_HANDLER(init_handler);
