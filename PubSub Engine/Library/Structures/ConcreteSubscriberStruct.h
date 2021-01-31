#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 256


typedef struct concreteSubscriberThreadStruct_t {
	char message[BUFFER_SIZE];
	SOCKET acceptedSocket;
}CONCRETE_SUBSCIBER;