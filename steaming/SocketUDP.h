#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

// Socket interface object for sending and receiving
typedef struct SockInterfaceStruct {
	SOCKET      hSock;
	SOCKADDR_IN hServAddr;

	sockaddr hClientAddr;
	int      iLenClientAddr;

	int    iPortNum;
	char   cIPAddr[16];
}SockObject;

