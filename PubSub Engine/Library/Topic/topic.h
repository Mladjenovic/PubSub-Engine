#ifndef _TOPIC_H
#define _TOPIC_H

#include "../Queue/queue.h"
#include "../SubscribersList/subscriberList.h"

typedef struct {
	unsigned long topicID;
	const char* TopicName;
	Queue* messageQueue;
	List subscribersList;
} TOPIC;

TOPIC * initTopic();
void storeMessageToQueue(TOPIC* t, char* message, NODE *pN);

#endif /* _TOPIC_H */