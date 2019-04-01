#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>

#define SOCKET_READ_TIMEOUT_SEC (2u)

// Socket interface object for sending and receiving
typedef struct SockInterfaceStruct {
  SOCKET      hSock;
  SOCKADDR_IN hServAddr;

  sockaddr hClientAddr;
  int      iLenClientAddr;

  int    iPortNum;
  char   cIPAddr[16]; // IP4 xxx.xxx.xxx.xxx
} SockObject;

// Function declarations 

int SocketUDP_SendServer2Client(SockObject *pSockObj, char *pDataBuf, int iDataSize);
