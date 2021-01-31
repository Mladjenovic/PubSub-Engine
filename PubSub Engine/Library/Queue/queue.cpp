#include <stdlib.h>
#include <stdio.h>
#include "queue.h"


Queue *ConstructQueue() {
	Queue *queue = (Queue*)malloc(sizeof(Queue));
	if (queue == NULL) {
		return NULL;
	}
	queue->limit = 65535;
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;

	return queue;
}

void DestructQueue(Queue *queue) {
	NODE *pN;
	while (!isEmpty(queue)) {
		pN = Dequeue(queue);
		free(pN);
	}
	free(queue);
}

int Enqueue(Queue *pQueue, NODE *item) {
	if ((pQueue == NULL) || (item == NULL)) {
		return FALSE;
	}
	// if(pQueue->limit != 0)
	if (pQueue->size >= pQueue->limit) {
		return FALSE;
	}
	item->prev = NULL;

	if (pQueue->tail != NULL)
		pQueue->tail->prev = item;

	pQueue->tail = item;

	if (pQueue->head == NULL)
		pQueue->head = item;

	pQueue->size++;
	return TRUE;

	///* Bad parameter */
	//if ((pQueue == NULL) || (item == NULL)) {
	//	return FALSE;
	//}
	//// if(pQueue->limit != 0)
	//if (pQueue->size >= pQueue->limit) {
	//	return FALSE;
	//}
	///*the queue is empty*/
	//item->prev = NULL;
	//if (pQueue->size == 0) {
	//	pQueue->head = item;
	//	pQueue->tail = item;
	//}
	//else {
	//	/*adding item to the end of the queue*/
	//	pQueue->tail->prev = pQueue->tail;
	//	pQueue->tail = item;
	//	
	//}
	//pQueue->size++;
	//return TRUE;
}
 
NODE * Dequeue(Queue *pQueue) {
	printf("\n%s", pQueue->head->data.message);
	/*the queue is empty or bad param*/
	NODE *item = NULL;
	if (isEmpty(pQueue))
		return NULL;
	item = pQueue->head;
	pQueue->head = (pQueue->head)->prev;
	pQueue->size--;
	return item;
}

int isEmpty(Queue* pQueue) {
	if (pQueue == NULL) {
		return FALSE;
	}
	if (pQueue->size == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void PrintQueue(Queue *queue) {
	NODE* current = queue->head;
	while (current != NULL) {
		printf("%s | ", current->data.message);
		current = current->prev;
	}
}