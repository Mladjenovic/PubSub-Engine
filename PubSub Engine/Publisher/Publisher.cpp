#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
//#define no_init_all deprecated

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#include "PublisherStruct.h"
#include "list.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define TRUE 1
#define FALSE 0

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 16000
#define BUFFER_SIZE 256

int NetworkCommunication();

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

	printf("Successfully connected to PubSubEngine.\n");


	int answer;
	int selectedTopic;
	PUBLISHER publisher;
	char* promenljiva;

	while (TRUE)
	{
		printf("\n1) Publish message");
		printf("\n2) Exit program\n");

		scanf_s("%d", &answer, BUFFER_SIZE);

		switch (answer)
		{
		case 1:
			printf("\nSelect topic: \n");
			printf("-------------------------------------------------------------\n");
			printf("1) Music\n2) Movies\n3) Sports\n4) Kids\n5) News\n6) Trending\n7) Send milion messages");
			printf("\n-------------------------------------------------------------");

			scanf_s("%d", &selectedTopic, BUFFER_SIZE);

			printf("\nSelected Topic: ");

			switch (selectedTopic)
			{
			case 1:
				printf("Music\n");
				publisher.topicID = 1;

				printf("\nEnter message:\n");

				while ((getchar()) != '\n');

				fgets(publisher.message, 256, stdin);

				// Send message to server using connected socket

 				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 2:
				printf("Movies\n");
				publisher.topicID = 2;

				printf("\nEnter message:\n");

				while ((getchar()) != '\n');
				fgets(publisher.message, 256, stdin);


				// Send message to server using connected socket
				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);


				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 3:
				printf("Sports\n");
				publisher.topicID = 3;

				printf("\nEnter message:\n");

				while ((getchar()) != '\n');
				fgets(publisher.message, 256, stdin);


				// Send message to server using connected socket
				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 4:
				printf("Kids\n");
				publisher.topicID = 4;

				printf("\nEnter message:\n");

				while ((getchar()) != '\n');
				fgets(publisher.message, 256, stdin);


				// Send message to server using connected socket
				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 5:
				printf("News\n");
				publisher.topicID = 5;

				printf("\nEnter message:\n");

				while ((getchar()) != '\n');
				fgets(publisher.message, 256, stdin);


				// Send message to server using connected socket
				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 6:
				printf("Trending\n");
				publisher.topicID = 6;

				printf("\nEnter message:\n");
				scanf_s("%s", publisher.message, 128);


				// Send message to server using connected socket
				iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

				// Check result of send function
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}

				break;
			case 7:

				publisher.topicID = 1;
				for (int i = 0; i <= 1000000; i++)
				{
					//sprintf(publisher.message, "Poruka %d", i);
					sprintf(publisher.message, "Poruka %d", i);

					iResult = send(connectSocket, (char*)&publisher, sizeof(PUBLISHER), 0);

					// Check result of send function
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(connectSocket);
						WSACleanup();
						return 1;
					}
					Sleep(50);
				}
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

			// Deinitialize WSA library
			WSACleanup();

			return 0;
		default:
			printf("Option does not exist\n");
			break;
		}
	}
}