#include <stdio.h>
#include <stdlib.h>

#include "libs/libuthread/uthread.h"

int hello(void* arg) {
	printf("Hello world!%p\n", arg);
	return 0;
}

int main(void) {
    uthread_init();

	uthread_t tid;

	tid = uthread_create(hello, 0x123);
	printf("Created thread\n");
    uthread_join(tid, NULL);
    printf("All done\n");


	return 0;
}