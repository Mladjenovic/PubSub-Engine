#define BUFFER_SIZE 256


typedef struct publisher_t {
	int topicID;
	char message[128];
}PUBLISHER;