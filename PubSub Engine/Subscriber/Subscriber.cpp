#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
//#define no_init_all deprecated

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "list.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define TRUE 1
#define FALSE 0

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 16001
#define BUFFER_SIZE 256

int NetworkCommunication();

DWORD WINAPI processingPubSubEngine(LPVOID par)
{
	SOCKET connectedSocket = (SOCKET)par;

	char dataBuffer[BUFFER_SIZE];


	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		// Add socket to set readfds
		FD_SET(connectedSocket, &readfds);


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
			if (FD_ISSET(connectedSocket, &readfds))
			{
				int iResult = recv(connectedSocket, dataBuffer, BUFFER_SIZE, 0);
				if (iResult > 0)	// Check if message is successfully received
				{
					dataBuffer[iResult] = '\0';

					printf("%s\n", dataBuffer);

				}
				else if (iResult == 0)	// Check if shutdown command is received
				{
					// Connection was closed successfully
					printf("Connection with server closed.\n");
					//closesocket(connectedSocket);
					break;
				}
				else	// There was an error during recv
				{
					//printf("recv failed with error: %d\n", WSAGetLastError());
					printf("Connection with PubSubEngine closed!");
					//closesocket(connectedSocket);
					break;
				}
			}
		}

		// Receive data until the client shuts down the connection
	}

	int iResult = shutdown(connectedSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		//closesocket(acceptedSocket);
		//WSACleanup();
		return 1;
	}
	closesocket(connectedSocket);


	return 0;
}

void main()
{
	NetworkCommunication();

	_getch();
}

int NetworkCommunication()
{
	// Socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult = 0;

	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// Create a socket
	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	unsigned long mode = 1; //non-blocking mode
	iResult = ioctlsocket(connectSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", iResult);
		return 1;
	}

	printf("Successfully connected to PubSubEngine.\n");

	// start listening thread

	DWORD pubSubengineThreadID;
	HANDLE hpubSubengine;

	hpubSubengine = CreateThread(NULL, 0, &processingPubSubEngine, (LPVOID)connectSocket, 0, &pubSubengineThreadID);

	// Communication loop

	int answer;
	int selectedTopic;
	bool topicArray[6];
	for (int  i = 0; i < 6; i++)
	{
		topicArray[i] = false;
	}

	while (TRUE)
	{
		printf("\n1) Subscribe to Topic");
		printf("\n2) Exit program\n");

		scanf_s("%d", &answer);

		switch (answer)
		{
		case 1:
			printf("\nSelect topic: \n");
			printf("-------------------------------------------------------------\n");
			printf("1) Music\n2) Movies\n3) Sports\n4) Kids\n5) News\n6) Trending");
			printf("\n-------------------------------------------------------------");

			scanf_s("%d", &selectedTopic);

			printf("\nSelected Topic: ");

			switch (selectedTopic)
			{
			case 1:
				if (topicArray[0] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic
				
				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 1);
				topicArray[0] = true;

				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Music topic\n");

				break;
			case 2:
				if (topicArray[1] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic

				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 2);
				topicArray[1] = true;

				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Movies topic\n");

				break;
			case 3:
				if (topicArray[2] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic

				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 3);
				topicArray[2] = true;

				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Sports topic\n");

				break;
			case 4:
				if (topicArray[3] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic

				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 4);
				topicArray[3] = true;

				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Kids topic\n");

				break;
			case 5:
				if (topicArray[4] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic

				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 5);
				topicArray[4] = true;


				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Trending topic\n");

				break;
			case 6:
				if (topicArray[5] != false)
				{
					printf("You already subscribed to that topic.\n");
					break;
				}

				// Subscribe logic

				sprintf_s(dataBuffer, BUFFER_SIZE, "%d", 6);
				topicArray[5] = true;

				// Send message to server using connected socket
				iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				printf("Successfully subscribed to Trending topic\n");

				break;
			default:
				printf("Option does not exist\n");
				break;
			}
			break;
		case 2:
			// Shutdown the connection since we're done
			iResult = shutdown(connectSocket, SD_BOTH);

			// Check if connection is succesfully shut down.
			if (iResult == SOCKET_ERROR)
			{
				printf("Shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			// For demonstration purpose
			printf("\nPress any key to exit: ");
			_getch();

			// Close connected socket
			closesocket(connectSocket);
			CloseHandle(hpubSubengine);

			// Deinitialize WSA library
			WSACleanup();

			return 0;
		default:
			printf("Option does not exist\n");
			break;
		}
	}
}