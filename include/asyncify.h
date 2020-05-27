#ifndef _ASYNCIFY_H
#define _ASYNCIFY_H

#include <stdint.h>

// #include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

// These get replaced with calls to the Asyncify instructions
void asyncify_start_unwind(uint32_t x);
void asyncify_stop_unwind();
void asyncify_start_rewind(uint32_t x);
void asyncify_stop_rewind();

#ifdef __cplusplus
}
#endif

#endif
