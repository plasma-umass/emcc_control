
#include "config.h"
#include "context.h"

#if CONTEXT_IMPL == WASMTIME_CONTS
#include "context_conts.c"
#elif CONTEXT_IMPL == WASMTIME_ASYNCIFY
#include "context_asyncify.c"
#elif CONTEXT_IMPL == NATIVE_SWAPCONTEXT
#include "context_swapcontext.c"
#endif