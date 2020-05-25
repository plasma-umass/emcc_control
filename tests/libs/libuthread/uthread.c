#include "config.h"

#if NEED_CONTEXT

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "uthread.h"
#include "context.h"


// Define the different thread state possibilities
enum ThreadState{Running, Ready, Blocked, Zombie};

struct TCB {
	uthread_t tid;
	context_t context;
	struct TCB *joiningThread;
	enum ThreadState state;
	int retVal; // Only valid if state == Zombie
};

struct TCB *newTCB(uthread_t tid, enum ThreadState state) {
	struct TCB *n = malloc(sizeof(struct TCB));
	n->joiningThread = NULL;
	n->tid = tid;
	n->state = state;
	return n;
}

void _print_TCB(struct TCB *tcb) {
	char *s;
	switch(tcb->state) {
		case Running: s = "Running"; break;
		case Ready: s = "Ready"; break;
		case Blocked: s = "Blocked"; break;
		case Zombie: s = "Zombie"; break;
	}
	printf("{ tid = %d, state = %s, retVal = %d }", tcb->tid, s, tcb->retVal);
}

int _debug_print_item(queue_t q, void *data, void *arg) {
	struct TCB *tcb = (struct TCB *)data;
	_print_TCB(tcb);
	printf(", ");
	return 0;
}

void _debug_print_queue(queue_t q) {
	queue_iterate(q, _debug_print_item, NULL, NULL);
}

queue_t readyQueue = NULL;
struct TCB *runningThread = NULL;
queue_t blockedOrZombieQueue = NULL;
uthread_t nextTID = 1;

void _debug_print_state(const char *funcName) {
	return;
	if(runningThread == NULL) {
		printf("%s:\n", funcName);
		printf("\tUnitialized!\n\n");
		return;
	}
	printf("%s:\n", funcName);
	printf("\tRunning thread = "); _print_TCB(runningThread);
	printf("\n\tReady queue = "); _debug_print_queue(readyQueue);
	printf("\n\tBl/Zmb queue = "); _debug_print_queue(blockedOrZombieQueue); printf("\n\n");
}

// Initialize the thread queues and set up the main thread
void uthread_init() {
	context_initialize_lib();

	// Create the thread queues
	readyQueue = queue_create();
	blockedOrZombieQueue = queue_create();

	// Create and enqueue TCB for the main thread (TID = 0)
	struct TCB *mainTCB = newTCB(0, Running);
	runningThread = mainTCB;
}

// Running thread yields to another thread to execute
void uthread_yield(void)
{
	_debug_print_state(__func__);

	struct TCB *toMakeReady = runningThread;
	struct TCB *toMakeRunning;

	// preempt_disable();

	int dequeueRet = queue_dequeue(readyQueue, (void **)&toMakeRunning);

	if(dequeueRet == -1) {
		// Then the queue was empty, so there are no other threads to schedule.
		// So just return to give execution back to the current thread.

		// preempt_enable();
		return;
	}

	toMakeReady->state = Ready;
	toMakeRunning->state = Running;
	runningThread = toMakeRunning;

	// Note: it is impossible for malloc() to fail in enqueue.
	// Enough memory was previously allocated in the queue, and was freed with queue_dequeue.
	// Since nothing was allocated in between, at least enough memory is available.
	queue_enqueue(readyQueue, toMakeReady);

	// preempt_enable();

	// Perform context switch from running (now ready) thread to the new thread
	context_switch(&toMakeReady->context, toMakeRunning->context);
}

// Get TID of running thread
uthread_t uthread_self(void)
{
	_debug_print_state(__func__);

	return runningThread->tid;
}

// Create a new thread running the function func with the argument arg
int uthread_create(uthread_t *out_t, uthread_func_t func, void *arg)
{
	_debug_print_state(__func__);

	// Initialize library if needed
	if(runningThread == NULL) {
		abort();
	}

	// Make sure there aren't too many threads!
	if(nextTID == 0) {
		return -1;
	}

	// Make sure the function pointer is non-null
	if(func == NULL) {
		return -1;
	}

	// Create the TCB
	struct TCB *threadTCB = newTCB(0, Ready);

	if(threadTCB == NULL) {
		// The malloc for the TCB failed, so sad...
		return -1;
	}

	// Initialize the context
	context_init(&threadTCB->context, func, arg);

	// preempt_disable();
	
	threadTCB->tid = nextTID;
	nextTID++;
	queue_enqueue(readyQueue, threadTCB);

	// preempt_enable();

	*out_t = threadTCB->tid;
	return 0;
}

// Exit from running thread to complete its execution
// This thread enters a zombie state
// And is not deallocated until collected by a joining thread
void uthread_exit(int retval)
{
	_debug_print_state(__func__);

	struct TCB *toMakeZombie = runningThread;
	toMakeZombie->retVal = retval;
	
	// If thread has been joined, we need to unblock joining thread
	// And move it to the ready queue
	if(toMakeZombie->joiningThread != NULL) {
		// preempt_disable();

		queue_delete(blockedOrZombieQueue, toMakeZombie->joiningThread);
		toMakeZombie->joiningThread->state = Ready;

		queue_enqueue(readyQueue, toMakeZombie->joiningThread);

		// preempt_enable();
	}

	struct TCB *toMakeRunning;
	
	// preempt_disable();

	int dequeueRet = queue_dequeue(readyQueue, (void **)&toMakeRunning);

	// If the queue is empty
	if(dequeueRet == -1) {
		// There are no other threads to schedule.
		// So all threads are gone, so just exit?
		exit(retval);
	}

	toMakeZombie->state = Zombie;
	toMakeRunning->state = Running;
	runningThread = toMakeRunning;

	// Note: it is impossible for malloc() to fail in enqueue.
	// Enough memory was previously allocated in the queue, and was freed with queue_dequeue.
	// Since nothing was allocated in between, at least enough memory is available.
	queue_enqueue(blockedOrZombieQueue, toMakeZombie);

	// preempt_enable();

	// Perform context switch to next thread in ready queue
	context_switch(&toMakeZombie->context, toMakeRunning->context);
}

// Custom callback function for use with uthread_join()
// Provided as an argument to uthread_iterate() in uthread_join()
// To locate the TID of the thread to be joined
int _find_tid(queue_t q, void *data, void *arg) {
	if(((struct TCB *)data)->tid == *(uthread_t*)arg) {
		return 1;
	}

	return 0;
}

// Running thread joins another thread
// Running thread must wait until thread to join completes before joining it
int uthread_join(uthread_t tid, int *retval)
{
	_debug_print_state(__func__);

	struct TCB *threadToJoin = NULL;

	if(tid < 1 || tid == uthread_self()) {
		return -1;
	}

	// preempt_disable();

	// Search for thread to join in ready queue
	queue_iterate(readyQueue, _find_tid, (void*)&tid, (void**)&threadToJoin);
	
	// If thread to join was not in the ready queue, we also search in blocked/zombie queue
	if(threadToJoin == NULL) {
		queue_iterate(blockedOrZombieQueue, _find_tid, (void*)&tid, (void**)&threadToJoin);
	}

	// preempt_enable();
	
	// If we couldn't locate thread in either queue
	// Or if the thread is already being joined by another thread
	if(threadToJoin == NULL || threadToJoin->joiningThread != NULL) {
		return -1;
	}

	// If thread to join is a Zombie:
	// Collect its retVal, remove it from the queue, and free its TCB
	if(threadToJoin->state == Zombie) {
		if(retval != NULL) {
			*retval = threadToJoin->retVal;
		}

		// preempt_disable();
		queue_delete(blockedOrZombieQueue, threadToJoin);
		// preempt_enable();

		free(threadToJoin);
		return 0;
	}


	// preempt_disable();

	// Keep track of the joining thread
	threadToJoin->joiningThread = runningThread;
	
	// If threadToJoin is still active, we need to block the running thread
	runningThread->state = Blocked;
	struct TCB *toMakeBlocked = runningThread;

	queue_enqueue(blockedOrZombieQueue, toMakeBlocked);

	// While we wait for threadToJoin to become a Zombie, switch to another thread in the ready queue
	struct TCB *toMakeRunning;
	int dequeueRet = queue_dequeue(readyQueue, (void **)&toMakeRunning);

	// If the queue is empty
	if(dequeueRet == -1) {
		// There are no other threads to schedule.
		// This should not happen (?), so exit with error
		fprintf(stderr, "NO THREADS TO JOIN!!!!\n");
		exit(1);
	}

	runningThread = toMakeRunning;

	// preempt_enable();

	// Perform context switch from running (now blocked) thread to a new thread
	context_switch(&toMakeBlocked->context, toMakeRunning->context);

	// Once threadToJoin dies, we can unblock runningThread
	// And collect threadToJoin's retval, remove it from the queue, and free its TCB
	if(retval != NULL) {
		*retval = threadToJoin->retVal;
	}

	// preempt_disable();
	queue_delete(blockedOrZombieQueue, threadToJoin);
	// preempt_enable();
	
	free(threadToJoin);

	return 0;
}

#endif