#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#ifndef _LIST_H
#define _LIST_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

typedef struct subscriber_t {
	unsigned short port;
	char* address;
	SOCKET acceptedSocket;
}SUBSCRIBER;


struct listitem {
	SUBSCRIBER subscriber;
	struct listitem *next;
};

typedef struct listitem Listitem;
struct list {
	Listitem *head;
};


typedef struct list List;
void initlist(List *); /* initialize an empty list */
void insertfront(List *, SUBSCRIBER* subscriber); /* insert val at front */
void insertback(List *, SUBSCRIBER subscriber); /* insert val at back */
int length(List); /* returns list length */
void destroy(List *); /* deletes list */
void setitem(List *, int n, SUBSCRIBER subscriber);/* modifies item at n to val*/
int getitem(List, int subscriberID); /* returns value at n*/
void displaylist(List*); /* displays the entire list*/

#endif /* _LIST_H */