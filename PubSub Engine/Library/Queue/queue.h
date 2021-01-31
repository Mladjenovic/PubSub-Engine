#ifndef _QUEUE_H
#define _QUEUE_H


#define TRUE  1
#define FALSE	0

/* a link in the queue, holds the info and point to the next Node*/
typedef struct {
	char message[256];
} DATA;

typedef struct Node_t {
	DATA data;
	struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct Queue {
	NODE *head;
	NODE *tail;
	int size;
	int limit;
} Queue;

Queue *ConstructQueue();
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, NODE *item);
NODE *Dequeue(Queue *pQueue);
int isEmpty(Queue* pQueue);
void PrintQueue(Queue *queue);

#endif /* _QUEUE_H */