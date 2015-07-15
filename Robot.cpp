/* *
	Robot.cpp
		The Implementation of Robot Motion and Control class

	Authors: Ricky Mason(ricky.mason@uky.edu)
		Department of Electrical and Computer Engineering
		University of Kentucky
* */

//#include "StdAfx.h"
#include "Robot.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <Windows.h>
#include <time.h>

#define NETWORK_ERROR -1
#define NETWORK_OK     0

SOCKET initializeSocket(const char * IPaddress, int port)
{
	WSADATA wsaData;   
	WORD sockVersion;
	int nret;
	sockVersion = MAKEWORD(1, 1);

    if (WSAStartup(sockVersion, &wsaData) != 0) 
	{
        printf("WSAStartup failed.\n") ;
		//return NETWORK_ERROR;
		return NULL;
	}
	else
		printf("Initialization Success.\n");
	// Initializing Winsock
	WSAStartup(sockVersion, &wsaData);
	LPHOSTENT hostEntry;
	// Create the listening socket

	//////// For Website (TEST ONLY)
	hostEntry = gethostbyname(IPaddress);
	//////// End of For Website (TEST ONLY)

	//////// For Static IP address
	//in_addr iaHost;
	//iaHost.s_addr = inet_addr("192.168.0.0");
	//hostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	//////// End of For Static IP address

	if (!hostEntry)
	{
		nret = WSAGetLastError();
		printf("No Host Found!\n");
		WSACleanup();
		//return NETWORK_ERROR;
		return NULL;
	}
	else
		printf("Successfully Found Host!\n");
	// Create the socket
	SOCKET theSocket;
	theSocket = socket(AF_INET,			// Go over TCP/IP
			   SOCK_STREAM,			// This is a stream-oriented socket
			   IPPROTO_TCP);		// Use TCP rather than UDP
	if (theSocket == INVALID_SOCKET)
	{
		nret = WSAGetLastError();
		printf("Can't Create Socket!\n");
		WSACleanup();
		//return NETWORK_ERROR;
		return NULL;
	}
	else
		printf("Successfully Create Socket!\n");
	// Fill a SOCKADDR_IN struct with address information
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(port);   // define port
	// Connect to the server
	nret = connect(theSocket,
		       (LPSOCKADDR)&serverInfo,
		       sizeof(struct sockaddr));
	if (nret == SOCKET_ERROR)
	{
		nret = WSAGetLastError();
		printf("Can't Connect Socket!\n");
		WSACleanup();
		//return NETWORK_ERROR;
		return NULL;
	}
	else
	{
		printf("Successfully Connect Socket!\n");
		return theSocket;
	}
}

SOCKET initializeSocket_New(int port)
{
    WSADATA wsaData;//creating a WSADATA object called wsaData
   

	//char *sendbuf1 = "(0.639, 0.12753, 0.46554, -0.4498, 2.1959, -0.2090)";
	//movel(p[0.516, -0.118, 0.41422, 0.0877, 2.677, -0.116], a = 0.002, v=0.001)\n
    //char *sendbuf2 = "(0.516, 0.14289, 0.63727, 0.1544, 2.4503, -0.227)";
	
	int iResult;//used to initialize winsock
    
	// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
	{
        printf("WSAStartup failed with error: %d\n", iResult);
        return NULL;
    }
    SOCKET ListenSocket = INVALID_SOCKET; //declare socket ListenSocket to listen for client connections
    SOCKET ClientSocket = INVALID_SOCKET; //declare socket ClientSocket to aceept connections from clients

    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return NULL;
    }
	else
		printf("Successfully Create Socket!\n");
    // The socket address to be passed to bind
    sockaddr_in service;
	// The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("192.168.0.5");
    service.sin_port = htons(port);
    //----------------------
    // Bind the socket.
    iResult = bind(ListenSocket, (SOCKADDR *) &service, sizeof (service));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error %u\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return NULL;
    }
	else
		printf("Successfully Bind Socket!\n");
    //listen for client cconnections
	iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) 
	{
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return NULL;
    }
	else
		printf("Successfully Listening Socket!\n");
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) 
	{
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return NULL;
    }
	else
		printf("Successfully Accept Client Socket!\n");
    // No longer need server socket
    closesocket(ListenSocket);
	return ClientSocket;
}

int sendCoordinates(float x, float y, float z, float rx, float ry, float rz, float a, float v, SOCKET theSocket)
{
		// Send Data
	int status;
//	char buffer[256];		// Declaring a buffer on the stack
	char *buffer = new char[DEFAULT_BUFLEN];	// or on the heap
	ZeroMemory(buffer, DEFAULT_BUFLEN);
	// strcpy(buffer, "Pretend this is important data.");
	int n=0;
	n = sprintf(buffer, "movel(p[%f, %f, %f, %f, %f, %f], a=%f, v=%f)\n", x,y ,z,rx,ry,rz,a,v);
	printf("Sending... ");
	cout<<buffer<<endl;
	status = send(theSocket,
	    buffer,
	    strlen(buffer),	// Note that this specifies the length of the string; not the size of the entire buffer
	    0);			// Most often is zero, but see MSDN for other options
	delete [] buffer;		// If and only if the heap declaration was used
	if (status == SOCKET_ERROR)
	{
		printf("Can't Connect Socket!\n");
		return NETWORK_ERROR;
	} 
	else 
	{
		printf("Send Successful!\n");		
	}
	return NETWORK_OK;
}

char receiveCoordinates(SOCKET theSocket)
{
	int n=0;
	//char *buffer = new char[1024];	// or on the heap
	//ZeroMemory(buffer, 1024);

	//n = sprintf(buffer, "TempPoint:=tool_pose()\n socket_send_string(TempPoint)\n");
	//n = send(theSocket,
	//    buffer,
	//    strlen(buffer),	// Note that this specifies the length of the string; not the size of the entire buffer
	//    0);			// Most often is zero, but see MSDN for other options
	////n = recv(theSocket, buffer, 256, 0); 
	//cout<<buffer<<endl;

	printf("Receiving... \n");

	char *buffer2 = new char[1024];	// or on the heap
	ZeroMemory(buffer2, 1024);
	n = recv(theSocket, buffer2, 1024, 0); 
	return *buffer2;
}



