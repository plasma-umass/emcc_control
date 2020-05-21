#include <stdint.h>
#include <stdlib.h>

#include "queue.h"

/*
 *  Implementation of a simple FIFO queue
 *  All operations besides queue_iterate() and queue_delete()
 *  Are performed in O(1) time
 */

struct queue_node {
	void *data;
	struct queue_node *next;
};

struct queue {
	struct queue_node *head;
	struct queue_node *tail;
	int len;
};

// Initialize a new queue
queue_t queue_create(void)
{
	queue_t q = malloc(sizeof(struct queue));
	
	if(q == NULL) {
		return NULL;
	}

	q->head = NULL;
	q->tail = NULL;
	q->len = 0;

	return q;
}

// Deallocate the memory of a queue object
int queue_destroy(queue_t queue)
{
	if(queue == NULL || queue->len != 0) {
		return -1;
	}
	free(queue);
	return 0;
}

// Add an item to the queue in O(1) time
// As the queue is FIFO, items are added to the "back" of the queue
int queue_enqueue(queue_t queue, void *data)
{
	struct queue_node *new = malloc(sizeof(struct queue_node));

	if(queue == NULL || new == NULL || data == NULL) {
		return -1;
	}

	// Special case for if queue is empty
	// In this case, we need to set enqueued node to head and tail
	if(queue->len == 0) {
		queue->head = new;
		queue->tail = new;
	} else {
		queue->tail->next = new;
	}

	queue->tail = new; // Enqueued node will be new tail
	new->next = NULL;
	new->data = data;

	queue->len++;

	return 0;
}

// Remove an item from the front of the queue in O(1) time
int queue_dequeue(queue_t queue, void **data)
{
	if(queue == NULL || queue->len == 0 || data == NULL) {
		return -1;
	}

	struct queue_node *toDequeue = queue->head;

	queue->head = toDequeue->next;

	// Special case for if queue has only one item
	// In this case, we need to set the tail to NULL
	// Setting the head to NULL was taken care of in previous instruction
	if(queue->len == 1) {
		queue->tail = NULL;
	}

	*data = toDequeue->data;
	free(toDequeue);

	queue->len--;

	return 0;
}

// Delete a specified data item from the queue
// Locates the oldest (nearest to front) instance of the data item and deletes it
int queue_delete(queue_t queue, void *data)
{
	if(queue == NULL || data == NULL) {
		return -1;
	}

	// Special case for a list of length 1
	// Since this needs to set head and tail to NULL
	if(queue->len == 1) {
		if(queue->head->data == data) {
			free(queue->head);

			queue->head = NULL;
			queue->tail = NULL;
			queue->len--;

			return 0;
		} else {
			return -1;
		}
	}

	struct queue_node *currentNode = queue->head;
	struct queue_node *prevNode = NULL;
	
	// Search queue for data item to delete
	while(currentNode != NULL) {
		if(currentNode->data == data) {
			// We found the match, so we need to do the delete
			if(prevNode == NULL) {
				// Special case for first item in list
				queue->head = currentNode->next;
			} else if(currentNode == queue->tail) {
				// Special case for last item in list
				queue->tail = prevNode;
				prevNode->next = NULL;
			} else {
				prevNode->next = currentNode->next;
			}
			free(currentNode);
			queue->len--;

			return 0;
		}

		prevNode = currentNode;
		currentNode = currentNode->next;
	}

	return -1;
}

// Iterate through the queue, calling the callback function func on each item
// The iteration terminates when func returns 1
int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	if(queue == NULL || func == NULL) {
		return -1;
	}

	struct queue_node *currentNode = queue->head;

	// Iterate over each item of queue until ret is 1
	while(currentNode != NULL) {
		int ret = func(queue, currentNode->data, arg);
		if(ret != 0) {
			if(data != NULL) {
				*data = currentNode->data;
			}
			break;
		}

		currentNode = currentNode->next;
	}
	return 0;
}

// Get length of the queue
int queue_length(queue_t queue)
{
	if(queue == NULL) {
		return -1;
	}
	return queue->len;
}
