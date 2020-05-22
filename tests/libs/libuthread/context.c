
#include "config.h"
#include "context.h"

#if CONTEXT_IMPL == CONTS
#include "context_conts.c"
#elif CONTEXT_IMPL == ASYNCIFY
#include "context_asyncify.c"
#elif CONTEXT_IMPL == SWAPCONTEXT
#include "context_swapcontext.c"
#endif