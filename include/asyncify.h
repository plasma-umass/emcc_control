#ifndef _ASYNCIFY_H
#define _ASYNCIFY_H

#include <stdint.h>
#include <stdlib.h>

// #include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASYNCIFY_STACK_SIZE 16384

uint32_t alloc_asyncify_buf() {
    uint32_t stack = (uint32_t)malloc(ASYNCIFY_STACK_SIZE);
    uint32_t stack_end = stack + ASYNCIFY_STACK_SIZE - 8;
    uint32_t *buf = (uint32_t *)malloc(sizeof(uint32_t)*2);
    buf[0] = stack;
    buf[1] = stack_end;
    return (uint32_t)buf;
}

// These get replaced with calls to the Asyncify instructions
void asyncify_start_unwind(uint32_t x);
void asyncify_stop_unwind();
void asyncify_start_rewind(uint32_t x);
void asyncify_stop_rewind();

#ifdef __cplusplus
}
#endif

#endif
