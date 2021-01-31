#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "conio.h"


#include "PublisherStruct.h"
#include "topic.h"
#include "SubscriberThreadStruct.h"
#include "subscriberList.h"
#include "../Structures/ConcreteSubscriberStruct.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SUBSCRIBER_IP_ADDRESS "127.0.0.1" //ovo nam i ne treba posto koristimo INADDR_ANY
#define SERVER_PORT 16000
#define SERVER_PORT2 16001

#define BUFFER_SIZE 256
#define MAX_STRING_SIZE 100

CRITICAL_SECTION cs; // Section for Topic structure
CRITICAL_SECTION cs2; // Section for subscriber list
CRITICAL_SECTION cs3; // Section for alerting subscribers


char* intToString(int num);
void initializeTopics();
void AlertSubscribers();

// Global

TOPIC* topics[6];
List subscribersList;  



DWORD WINAPI alertingSubscribers(LPVOID par) {

	EnterCriticalSection(&cs3);

	AlertSubscribers();

	LeaveCriticalSection(&cs3);

	return 0;
}

DWORD WINAPI alertConcreteSubscriber(LPVOID par) {

	SOCKET acceptedSocket = ((CONCRETE_SUBSCIBER*)par)->acceptedSocket;
	
	char mess[BUFFER_SIZE];
	sprintf(mess, "%s", ((CONCRETE_SUBSCIBER*)par)->message);

	// salji svakom pojedinacno
	int iResult = send(acceptedSocket, (char*)mess, (int)strlen(mess), 0);

	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

DWORD WINAPI processingPublisher(LPVOID par)  
{
	SOCKET acceptedSocket = (SOCKET)par;
	unsigned long mode = 1;
	if (ioctlsocket(acceptedSocket, FIONBIO, &mode) != 0)
	{
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
		closesocket(acceptedSocket);

		WSACleanup();
		return 0;
	}

	char dataBuffer[BUFFER_SIZE];


	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		// Add socket to set readfds
		FD_SET(acceptedSocket, &readfds);


		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		int sResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (sResult == 0)
		{
			Sleep(1000);
		}
		else if (sResult == SOCKET_ERROR)
		{
			printf("select failed with error: %d\n", WSAGetLastError());
			break;
		}
		else
		{
			if (FD_ISSET(acceptedSocket, &readfds))
			{
				int iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);
				if (iResult > 0)	// Check if message is successfully received
				{
					dataBuffer[iResult] = '\0';

					PUBLISHER* publisher = (PUBLISHER*)&dataBuffer;


					EnterCriticalSection(&cs);

					int topicID = publisher->topicID;
					char mess[BUFFER_SIZE];

					sprintf(mess, "%s", publisher->message);

					printf("Publisher sent: %d, %s\n", topicID, mess);
					NODE* messageNode = (NODE*)malloc(sizeof(NODE));
					strcpy(messageNode->data.message, mess);

					Enqueue(topics[topicID - 1]->messageQueue, messageNode);

					PrintQueue(topics[topicID - 1]->messageQueue);

					LeaveCriticalSection(&cs);
					
					// Sekcija za obavesatavanje subscribera

					DWORD alertThreadID;
					HANDLE hAlert;

					hAlert = CreateThread(NULL, 0, &alertingSubscribers, (LPVOID)NULL, 0, &alertThreadID);

				}
				else if (iResult == 0)	// Check if shutdown command is received
				{
					// Connection was closed successfully
					printf("Connection with Publisher closed.\n");
					closesocket(acceptedSocket);
					break;
				}
				//else	// There was an error during recv
				//{
				//	printf("recv failed with error: %d\n", WSAGetLastError());
				//	closesocket(acceptedSocket);
				//}
			}
		}

		// Receive data until the client shuts down the connection
	}
	return 0;
} 

DWORD WINAPI registerSubscriber(LPVOID par)
{
	char* subscriberAddress = ((SUBSCRIBER_THREAD*)par)->address;
	unsigned short subscriberPort = ((SUBSCRIBER_THREAD*)par)->port;
	SOCKET acceptedSocket = ((SUBSCRIBER_THREAD*)par)->acceptedSocket;


	unsigned long mode = 1;
	if (ioctlsocket(acceptedSocket, FIONBIO, &mode) != 0)
	{
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
		closesocket(acceptedSocket);

		WSACleanup();
		return 0;
	}

	char dataBuffer[BUFFER_SIZE];


	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		// Add socket to set readfds
		FD_SET(acceptedSocket, &readfds);


		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		int topicID = 0;
		SUBSCRIBER sub;


		int sResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (sResult == 0)
		{
			Sleep(1000);
		}
		else if (sResult == SOCKET_ERROR)
		{
			printf("select failed with error: %d\n", WSAGetLastError());
			break;
		}
		else
		{
			if (FD_ISSET(acceptedSocket, &readfds))
			{
				int iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);

				if (iResult > 0)	// Check if message is successfully received
				{
					dataBuffer[iResult] = '\0';

					sub.address = subscriberAddress;
					sub.port = subscriberPort;
					sub.acceptedSocket = acceptedSocket;

					topicID = atoi(dataBuffer);

					insertfront(&topics[topicID - 1]->subscribersList, &sub);

				}
				else if (iResult == 0)	// Check if shutdown command is received
				{
					// Connection was closed successfully
					printf("Connection with client closed.\n");
					break;
				}
				/*else	// There was an error during recv
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(*acceptedSocket);
				}*/
			}
		}

		// Receive data until the client shuts down the connection
	}

	// Shutdown the connection since we're done
	int iResult = shutdown(acceptedSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		//closesocket(acceptedSocket);
		//WSACleanup();
		return 1;
	}
	closesocket(acceptedSocket);

	return 0;
}

DWORD WINAPI acceptingSubscribers(LPVOID par)
{
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket = INVALID_SOCKET;
	int iResult;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT2);	// Use specific port



	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new Subscribers.\n");

	SUBSCRIBER_THREAD* subscriberParam = (SUBSCRIBER_THREAD*)malloc(sizeof(SUBSCRIBER_THREAD));
		
	do
	{
		sockaddr_in subscriberAddr;

		int subscriberAddrSize = sizeof(struct sockaddr_in);

		// Accept new connections from clients 
		acceptedSocket = accept(listenSocket, (struct sockaddr *)&subscriberAddr, &subscriberAddrSize);

		// Check if accepted socket is valid 
		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			closesocket(acceptedSocket);
			//WSACleanup();
			break;
		}
		printf("\nNew Subscriber request accepted. Subscriber address: %s : %d\n", inet_ntoa(subscriberAddr.sin_addr), ntohs(subscriberAddr.sin_port));

		
		SUBSCRIBER* subscriber = (SUBSCRIBER* )malloc(sizeof(SUBSCRIBER));
		subscriber->acceptedSocket = acceptedSocket;
		subscriber->address = inet_ntoa(subscriberAddr.sin_addr);
		subscriber->port = ntohs(subscriberAddr.sin_port);


		// Dodavanje subescribera u globalnu listu subscribera
		EnterCriticalSection(&cs2);

		insertfront(&subscribersList, subscriber);

		LeaveCriticalSection(&cs2);


		subscriberParam->address = inet_ntoa(subscriberAddr.sin_addr);
		subscriberParam->port = ntohs(subscriberAddr.sin_port);
		subscriberParam->acceptedSocket = acceptedSocket;

		DWORD regSubID;
		HANDLE hRegSub;

		hRegSub = CreateThread(NULL, 0, &registerSubscriber, (LPVOID)subscriberParam, 0, &regSubID);
										
	} while (true);


	// Shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_BOTH);

	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		closesocket(acceptedSocket);
		//WSACleanup();
		return 1;
	}

	iResult = shutdown(listenSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		closesocket(acceptedSocket);
		//WSACleanup();
		return 1;
	}

	free(subscriberParam);
	closesocket(acceptedSocket);
	closesocket(listenSocket);

}

int main()
{
	initializeTopics();
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&cs2);
	InitializeCriticalSection(&cs3);

	initlist(&subscribersList);

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Socket used for communication with Publisher
	SOCKET acceptedSocket = INVALID_SOCKET;

	// Socket used to communicate with Subscriber
	SOCKET connectSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}


	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new Publishers.\n");


	// Accepting Subscribers thread

	DWORD subscriberThreadID;
	HANDLE hSubscriber;

	hSubscriber = CreateThread(NULL, 0, &acceptingSubscribers, (LPVOID)acceptedSocket, 0, &subscriberThreadID);


	do
	{
		// Struct for information about connected client
		sockaddr_in clientAddr;

		int clientAddrSize = sizeof(struct sockaddr_in);

		// Accept new connections from clients 
		acceptedSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);

		// Check if accepted socket is valid 
		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		printf("\nNew Publisher request accepted. Publisher address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));


		DWORD threadID;
		HANDLE hClient;

		hClient = CreateThread(NULL, 0, &processingPublisher, (LPVOID)acceptedSocket, 0, &threadID);

	} while (true);

	// Shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		closesocket(acceptedSocket);
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	//Close listen and accepted sockets
	closesocket(listenSocket);
	closesocket(acceptedSocket);
	closesocket(connectSocket);

	// Destroying all lists that were alocated in the memory
	destroy(&subscribersList); 

	for (int i = 0; i < 6; i++)
	{
		destroy(&topics[i]->subscribersList);
	}

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}

void AlertSubscribers()
{
	NODE *pN;
	List* list;
	List temp;

	for (int i = 0; i < 6; i++)
	{
		while (!isEmpty(topics[i]->messageQueue))
		{
			// isprazni element
			pN = Dequeue(topics[i]->messageQueue);
			char mess[BUFFER_SIZE];
			strcpy(mess, pN->data.message);

			char fullMessage[BUFFER_SIZE];

			switch (i + 1)
			{
			case 1:
				sprintf(fullMessage, "Music: %s", mess);
				break;
			case 2:
				sprintf(fullMessage, "Movies: %s", mess);
				break;
			case 3:
				sprintf(fullMessage, "Sports: %s", mess);
				break;
			case 4:
				sprintf(fullMessage, "Kids: %s", mess);
				break;
			case 5:
				sprintf(fullMessage, "News: %s", mess);
				break;
			case 6:
				sprintf(fullMessage, "Trending: %s", mess);
				break;
			default:
				printf("\n!!!ERROR!!!\n");
				return;
			}

			// posalji ga svim subscriberima
			list = &topics[i]->subscribersList;
			temp = *list;
			while (temp.head != NULL)
			{
				//alertConcreteSubscriber

				CONCRETE_SUBSCIBER* sub = (CONCRETE_SUBSCIBER*)malloc(sizeof(CONCRETE_SUBSCIBER));
				sub->acceptedSocket = temp.head->subscriber.acceptedSocket;

				sprintf(sub->message, "%s", fullMessage);

				DWORD concreteSubscriberID;
				HANDLE hConcreteSubscriber;

				hConcreteSubscriber = CreateThread(NULL, 0, &alertConcreteSubscriber, (LPVOID)sub, 0, &concreteSubscriberID);

				
				temp.head = temp.head->next;         // advances the position of current node
			}
		}
	}
}

char* intToString(int num)
{
	char* result = (char*)malloc(sizeof(char)*MAX_STRING_SIZE);
	sprintf_s(result, sizeof(result), "%d", num);
	return result;
}

void initializeTopics()
{
	for (int i = 0; i < 6; i++)
	{
		topics[i] = initTopic();
		topics[i]->topicID = i + 1;
		topics[i]->messageQueue = ConstructQueue();
		initlist(&topics[i]->subscribersList);
	}

	topics[0]->TopicName = "Music";
	topics[1]->TopicName = "Movies";
	topics[2]->TopicName = "Sports";
	topics[3]->TopicName = "Kids";
	topics[4]->TopicName = "News";
	topics[5]->TopicName = "Trending";
}