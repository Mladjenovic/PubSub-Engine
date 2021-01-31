#include <stdlib.h>
#include "topic.h"


TOPIC* initTopic()
{
	TOPIC* t = (TOPIC*)malloc(sizeof(TOPIC));

	initlist(&t->subscribersList);

	t->messageQueue = ConstructQueue();

	return t;
}

//void storeMessageToQueue(TOPIC* t, char* message, NODE *pN)
//{
//	//pN = (NODE*)malloc(sizeof(NODE));
//	pN->data.message = message;
//	Enqueue(t->messageQueue, pN);
//}
