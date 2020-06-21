#ifndef _UTHREAD_H
#define _UTHREAD_H

#include "config.h"

int null_uthread_create(int *t, void *f, void *arg);
void null_uthread_yield(void);
int null_uthread_join(int t, int *retval);
void null_uthread_init_main(void (*f)(void *), void *arg);

#if NEED_CONTEXT

/*
 * uthread_t - Thread identifier (TID) type
 *
 * Each user thread is assigned a different TID. TID are assigned in increasing
 * order and numbered starting from 1 (apart from the 'main' thread who
 * automatically gets TID #0). Overflowing the current TID value is considered a
 * case of failure (in other words, it is impossible to create more than
 * USHRT_MAX threads).
 */
typedef unsigned short uthread_t;

/*
 * uthread_func_t - Thread function type
 * @arg: Argument to be passed to the thread
 *
 * Return: Integer value
 */

typedef void* (*uthread_func_t)(void *arg);

void uthread_init_main(void (*f)(void *), void *arg);

/*
 * uthread_create - Create a new thread
 * @func: Function to be executed by the thread
 * @arg: Argument to be passed to the thread
 *
 * This function creates a new thread running the function @func to which
 * argument @arg is passed, and returns the TID of this new thread.
 *
 * Return: -1 in case of failure (memory allocation, context creation, TID
 * overflow, etc.). The TID of the new thread otherwise.
 */

int uthread_create(uthread_t *t, uthread_func_t func, void *arg);

/*
 * uthread_self - Get thread identifier
 *
 * Return: The TID of the currently running thread
 */
uthread_t uthread_self(void);

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void);

/*
 * uthread_exit - Exit from currently running thread
 * @retval: Return value
 *
 * This function is to be called from the currently active and running thread in
 * order to finish its execution. The return value @retval is to be collected
 * from a joining thread.
 *
 * A thread which has not been 'collected' should stay in a zombie state. This
 * means that until collection, the resources associated to a zombie thread
 * should not be freed.
 *
 * This function shall never return.
 */
void uthread_exit(int retval);

/*
 * uthread_join - Join a thread
 * @tid: TID of the thread to join
 * @retval: Address of an integer that will receive the return value
 *
 * This function makes the calling thread wait for the thread @tid to complete
 * and assign the return value of the finished thread to @retval (if @retval is
 * not NULL).
 *
 * A thread can be joined by only one other thread.
 *
 * Return: -1 if @tid is not a positive and non-null integer (the 'main' thread
 * cannot be joined), if @tid is the TID of the calling thread, if thread @tid
 * cannot be found, or if thread @tid is already being joined. 0 otherwise.
 */
int uthread_join(uthread_t tid, int *retval);

#elif CONTEXT_IMPL == NATIVE_PTHREAD

#include <pthread.h>
#define uthread_t pthread_t
#define uthread_create(t, f, x) pthread_create(t, NULL, f, x) 
#define uthread_join(t, rv) pthread_join(t, NULL)
#define uthread_yield() null_uthread_yield()
#define uthread_init_main(f) null_uthread_init(f)

#elif (CONTEXT_IMPL == NATIVE_SERIAL || CONTEXT_IMPL == WASMTIME_SERIAL)

#define uthread_t int
#define uthread_create(t, f, x) f(x)
#define uthread_join(t, rv) null_uthread_join(t, rv)
#define uthread_yield() null_uthread_yield()
#define uthread_init_main(f) null_uthread_init(f)

#endif

#endif /* _THREAD_H */