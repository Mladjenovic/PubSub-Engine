typedef struct subscriberThreadStruct_t {
	unsigned short port;
	char* address;
	SOCKET acceptedSocket;
}SUBSCRIBER_THREAD;