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


char* intToString(int num);
void initializeTopics();


/// Threads

TOPIC* topics[6];
List subscribersList;



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

					printf("Publisher sent: %d, %s\n", publisher->topicID, publisher->message);
					NODE* messageNode = (NODE*)malloc(sizeof(NODE));
					messageNode->data.message = publisher->message;
					for (int i = 0; i < 6; i++)
					{
						if (publisher->topicID == 1) {
							Enqueue(topics[0]->messageQueue, messageNode);
							break;
						}
						else if (publisher->topicID == 2) {
							Enqueue(topics[1]->messageQueue, messageNode);
							break;
						}
						else if (publisher->topicID == 3) {
							Enqueue(topics[2]->messageQueue, messageNode);
							break;
						}
						else if (publisher->topicID == 4) {
							Enqueue(topics[3]->messageQueue, messageNode);
							break;
						}
						else if (publisher->topicID == 5) {
							Enqueue(topics[4]->messageQueue, messageNode);
							break;
						}
						else if (publisher->topicID == 6) {
							Enqueue(topics[5]->messageQueue, messageNode);
							break;
						}
					}

					LeaveCriticalSection(&cs);

				}
				else if (iResult == 0)	// Check if shutdown command is received
				{
					// Connection was closed successfully
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
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

					SUBSCRIBER sub;
					sub.address = subscriberAddress;
					sub.port = subscriberPort;
					sub.acceptedSocket = acceptedSocket;

					EnterCriticalSection(&cs2);

					insertfront(&subscribersList, sub);
					displaylist(&subscribersList);

					LeaveCriticalSection(&cs2);

				}
				else if (iResult == 0)	// Check if shutdown command is received
				{
					// Connection was closed successfully
					printf("Connection with client closed.\n");
					closesocket(acceptedSocket);
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
			WSACleanup();
			return 1;
		}
		printf("\nNew Subscriber request accepted. Subscriber address: %s : %d\n", inet_ntoa(subscriberAddr.sin_addr), ntohs(subscriberAddr.sin_port));

		subscriberParam->address = inet_ntoa(subscriberAddr.sin_addr);
		subscriberParam->port = ntohs(subscriberAddr.sin_port);
		subscriberParam->acceptedSocket = acceptedSocket;

		DWORD regSubID;
		HANDLE hRegSub;

		// Sada ide thread da u kome ce da se izdvoji logika registrovanje (dodavanje u listu subscribera) prosledjivace se struktura sa (subscriberAddr.sin_addr i subscriberAddr.sin_port)
		// A u tredu ce se primati na koji topic hoce + kredencijali (add i port) iz strukture

		hRegSub = CreateThread(NULL, 0, &registerSubscriber, (LPVOID)subscriberParam, 0, &regSubID);
										

	} while (true);
}


int main()
{

	initializeTopics();
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&cs2);

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

		//// Treba dodati handlove u neku listu pa onda iterirati kroz nju i oslobadnjati svaki pojedinacan element sa CloseHandle
		////CloseHandle(hClient);


	} while (true);

	// Shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}

	//Close listen and accepted sockets
	closesocket(listenSocket);
	closesocket(acceptedSocket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;


	/*for (int i = 0; i < 6; i++)
	{
		printf("%s\n", topics[i]->TopicName);
	}*/


	//TOPIC* t = initTopic();

	//t->messageQueue = ConstructQueue();
	//NODE *pN;
	//
	//char* result;

	//for (int i = 0; i < 10; i++)
	//{
	//	result = intToString(i);
	//	pN = (NODE*)malloc(sizeof(NODE));
	//	storeMessageToQueue(t, result, pN);
	//}

	//while (!isEmpty(t->messageQueue)) {
	//	pN = Dequeue(t->messageQueue);
	//	printf("\nDequeued: %s", pN->data.message);
	//	free(pN->data.message); // Oslobadanje zauzete memorije za result!
	//	free(pN);
	//}
	//DestructQueue(t->messageQueue);
	//printf("\n\n%s\n", result);

	//return (EXIT_SUCCESS);
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
	}

	topics[0]->TopicName = "Music";
	topics[1]->TopicName = "Movies";
	topics[2]->TopicName = "Sports";
	topics[3]->TopicName = "Kids";
	topics[4]->TopicName = "News";
	topics[5]->TopicName = "Trending";
}