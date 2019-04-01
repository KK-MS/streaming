#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Network related includes
//#undef UNICODE

// Need older API support e.g. IPV4. Else result in "Error E0040"
// E0040 expected an identifier ws2def.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h> // getaddrinfo, includes #include <winsock2.h>
#include <stdio.h>

#include "SocketUDP.h"

#pragma comment (lib, "Ws2_32.lib")

// Network macros
//
#ifndef MAX_UDP_DATA_SIZE
#define MAX_UDP_DATA_SIZE (65000u)
#endif

#define TAG_SOCK "StereoSock: "

// Socket static variable
static SOCKET s;
static SOCKADDR_IN servaddr;

int SocketUDP_PrintIpPort(SOCKET *phSock, const char *pTagName)
{
  struct sockaddr_in sin;
  int len = sizeof(sin);

  if (getsockname(*phSock, (struct sockaddr *)&sin, &len) == -1) {
    perror("getsockname");
    return -1;
  }

  printf(TAG_SOCK"[%s] IP:Port %s:%d\n", pTagName, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
  return 0;
}

int SocketUDP_RecvFrom(SOCKET *phSock, char *pDataBuf, int iDataSize,
    sockaddr *pSockCliAddr, int *pSockSize)
{
  int iRetVal;

  // Packet details
  char *pRecvBuf;
  int iRxLen;
  int iAccRxLen;

  pRecvBuf = pDataBuf;
  iAccRxLen = 0;

  while(iAccRxLen < iDataSize) {

    iRxLen = iDataSize - iAccRxLen;

    if (iRxLen > MAX_UDP_DATA_SIZE) { iRxLen = MAX_UDP_DATA_SIZE; }

    printf(TAG_SOCK "B4 RecvFrom: ReqLen:%d len:%d AccLen:%d\n", iDataSize, iRxLen, iAccRxLen);
    iRetVal = recvfrom(*phSock, pRecvBuf, iRxLen, 0, pSockCliAddr, pSockSize);
    SocketUDP_PrintIpPort(phSock, "RecvTo");
    printf(TAG_SOCK"[%s] IP:Port %s:%d\n", "Remote details:", inet_ntoa(((struct sockaddr_in *)pSockCliAddr)->sin_addr), ntohs(((struct sockaddr_in *)pSockCliAddr)->sin_port));
    if (iRetVal < 0) { return -1; }

    pRecvBuf  += iRetVal;
    iAccRxLen += iRetVal;
    printf(TAG_SOCK "A4 RecvFrom: len:%d AccLen:%d, iRetVal:%d\n", iRxLen, iAccRxLen, iRetVal);

    if (iRetVal < iRxLen) { break; }
  }

  return iAccRxLen;
}

int SocketUDP_SendTo(SOCKET *phSock, char *pDataBuf, int iDataSize,
    sockaddr *pSockDestAddr, int iSockSize)
{

  int iRetVal;

  // Packet details
  const char *pSendBuf;
  int iTxLen;
  int iAccTxLen;

  pSendBuf    = pDataBuf;
  iAccTxLen = 0;

  while(iAccTxLen < iDataSize) {

    iTxLen = iDataSize - iAccTxLen;

    if (iTxLen > MAX_UDP_DATA_SIZE) { iTxLen = MAX_UDP_DATA_SIZE; }

    printf(TAG_SOCK "B4 sendto: iDataSize:%d len:%d AccLen:%d\n", iDataSize, iTxLen, iAccTxLen);
    SocketUDP_PrintIpPort(phSock, "SendTo");
    iRetVal = sendto(*phSock, pSendBuf, iTxLen, 0, pSockDestAddr, iSockSize);
    if (iRetVal < 0) { return -1; }

    pSendBuf  += iRetVal;
    iAccTxLen += iRetVal;
    printf(TAG_SOCK "A4 sendto: iRetVal:%d len:%d AccLen:%d\n", iRetVal, iTxLen, iAccTxLen);

    if (iRetVal < iTxLen) {
      // Should not happend, Track it!!.
      printf(TAG_SOCK "WARNING: Sending %d / %d\n", iAccTxLen, iDataSize);
      break;
    }
  }

  return iAccTxLen;
}

int SocketUDP_ClientRecv(SockObject *pSockObj, char *pDataBuf, int iDataSize)
{
  SOCKET *phSock      = &pSockObj->hSock;
  sockaddr *phCliAddr = &pSockObj->hClientAddr;
  int *pSockSize      = &pSockObj->iLenClientAddr;

  return SocketUDP_RecvFrom(phSock, pDataBuf, iDataSize, phCliAddr, pSockSize);

}


int SocketUDP_ClientSend(SockObject *pSockObj, char *pDataBuf, int iDataSize)
{
  SOCKET *phSock = &pSockObj->hSock;
  sockaddr *phCliAddr = &pSockObj->hClientAddr;
  int iSockSize = pSockObj->iLenClientAddr;

  return SocketUDP_SendTo(phSock, pDataBuf, iDataSize, (sockaddr *)phCliAddr, iSockSize);

}

int SocketUDP_Deinit(SOCKET *phSock)
{
  // deinit the network connection
  closesocket(*phSock);
  WSACleanup();

  printf("localize SocketUDP_Deinit ends\n");

  return 0;
}

SOCKET* SocketUPD_GetSocket(SockObject *pSockObj)
{
  int iRetVal;

  WSADATA wsaData;
  SOCKET *phSock;

  // Initialize Winsock
  iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iRetVal != 0) { goto ret_err; }

  // Create socket
  phSock  = &(pSockObj->hSock);
  *phSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (pSockObj->hSock == INVALID_SOCKET) { goto ret_err; }

  return phSock;

ret_err:
  printf(TAG_SOCK "Error in SocketUPD_GetSocket: %d \n", WSAGetLastError());
  return NULL;
}

int SocketUDP_ConnectServer(SockObject *pSockObj)
{
  int iRetVal;

  SOCKET *phSock;
  SOCKADDR_IN *phServAddr;

#if 0
  WSADATA wsaData;

  // Initialize Winsock
  iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iRetVal != 0) { goto ret_err; }

  // Create socket
  phSock  = &(pSockObj->hSock);
  *phSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (pSockObj->hSock == INVALID_SOCKET) { goto ret_err; }
#else
  if ((phSock = SocketUPD_GetSocket(pSockObj)) == NULL) { return -1;}
#endif

  // Connect to server
  phServAddr                  = &(pSockObj->hServAddr);
  memset((char *)phServAddr, 0, sizeof(SOCKADDR_IN));

  phServAddr->sin_family      = AF_INET;
  phServAddr->sin_port        = htons(pSockObj->iPortNum);
  phServAddr->sin_addr.s_addr = inet_addr(pSockObj->cIPAddr);

  iRetVal = connect(*phSock, (struct sockaddr *)phServAddr, sizeof(SOCKADDR_IN));
  if (iRetVal < 0) { goto ret_err; }

  printf(TAG_SOCK "Connected to Server: %s:%d\n", pSockObj->cIPAddr, pSockObj->iPortNum);
  SocketUDP_PrintIpPort(phSock, "Init");

  return 0;

ret_err:
  printf(TAG_SOCK "Error in SocketUDP_ClientConnectServer: %d \n", WSAGetLastError());
  return -1;
}

int SocketUDP_InitServer(SOCKET *phSock, SOCKADDR_IN *phServAddr,
    int  iPortNum,   char *pServerIP)
{
  WSADATA wsaData;
  int iRetVal;

  // Initialize Winsock
  iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iRetVal != 0) { goto ret_err; }

  // IMP: to clear server addr
  memset(phServAddr, 0, sizeof(SOCKADDR_IN));

  phServAddr->sin_family      = AF_INET;
  phServAddr->sin_addr.s_addr = inet_addr(pServerIP);
  phServAddr->sin_port        = htons(iPortNum);

  *phSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (*phSock == INVALID_SOCKET) { goto ret_err; }

  // Setup the UDP listening socket
  iRetVal = bind(*phSock, (struct sockaddr*) phServAddr, (int)sizeof(SOCKADDR_IN));
  if (iRetVal < 0) { goto ret_err; }

  return 0;

ret_err:

  printf(TAG_SOCK "Error in SocketUDP_InitServer: %d \n", WSAGetLastError());
  closesocket(*phSock);
  WSACleanup();

  return -1;

}

// Error code links:
// https://docs.microsoft.com/en-us/windows/desktop/debug/system-error-codes--0-499-
//
// https://docs.microsoft.com/de-de/windows/desktop/WinSock/windows-sockets-error-codes-2
// WSAEINTR 10004: Interrupted function call.
//    A blocking operation was interrupted by a call to WSACancelBlockingCall.
// WSAEACCES 10013: Permission denied.
//    An attempt was made to access a socket in a way forbidden by its access permissions.
//    An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST).
//    Another possible reason for the WSAEACCES error is that when the bind
//    function is called(on Windows NT 4.0 with SP4 and later), another
//    application, service, or kernel mode driver is bound to the same address
//    with exclusive access. Such exclusive access is a new feature of Windows
//    NT 4.0 with SP4 and later, and is implemented by using the
//    SO_EXCLUSIVEADDRUSE option.
// WSAEFAULT 10014: Bad address. The system detected an invalid pointer address
//    in attempting to use a pointer argument of a call.
//    This error occurs if an application passes an invalid pointer value,
//    or if the length of the buffer is too small. For instance, if the length
//    of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).
// WSAEINVAL 10022: Invalid argument. Some invalid argument was supplied
//   (for example, specifying an invalid level to the setsockopt function). In some instances,
//   it also refers to the current state of the socket, for instance, calling accept on a socket
//   that is not listening.
// WSAENOTSOCK 10038: Socket operation on nonsocket.
// WSAEMSGSIZE 10040: Message too long.
//   A message sent on a datagram socket was larger than the internal message
//   buffer or some other network limit, or the buffer used to receive a
//   datagram was smaller than the datagram itself.
// WSAEPROTONOSUPPORT 10043: Protocol not supported.
//   The requested protocol has not been configured into the system, or no
//   implementation for it exists.
//   e.g. A socket call requests a SOCK_DGRAM socket, but specifies a stream protocol.
// WSAEOPNOTSUPP 10045: Operation not supported.
//   The attempted operation is not supported for the type of object referenced.
//   Usually this occurs when a socket descriptor to a socket that cannot
//   support this operation is trying to accept a connection on a datagram socket.
// WSAEAFNOSUPPORT 10047: Address family not supported by protocol family.
//    An address incompatible with the requested protocol was used.
//    All sockets are created with an associated address family
//    (i.e. AF_INET for Internet Protocols) and a generic protocol type
//    (i.e. SOCK_STREAM). This error is returned if an incorrect protocol is
//    explicitly requested in the socket call, or if an address of the wrong
//    family is used for a socket, for example, in sendto.
// WSAEADDRNOTAVAIL 10049: Cannot assign requested address.
//    The requested address is not valid in its context. This normally results
//    from an attempt to bind to an address that is not valid for the local
//    computer. This can also result from connect, sendto, WSAConnect,
//    WSAJoinLeaf, or WSASendTo when the remote address or port is not valid
//    for a remote computer(for example, address or port 0).
// WSAECONNRESET 10054: Connection reset by peer.
//    An existing connection was forcibly closed by the remote host. This
//    normally results if the peer application on the remote host is suddenly
//    stopped, the host is rebooted, the host or remote network interface is
//    disabled, or the remote host uses a hard close (see setsockopt for more
//    information on the SO_LINGER option on the remote socket).This error may
//    also result if a connection was broken due to keep - alive activity detecting a failure while one or more operations are in progress.Operations that were in progress fail with WSAENETRESET.Subsequent operations fail with WSAECONNRESET.
// WSAENOTCONN 10057: Socket is not connected. A request to send or receive
//    data was disallowed because the socket is not connected and (when
//    sending on a datagram socket using sendto) no address was supplied.
//    Any other type of operation might also return this error. For example,
//    setsockopt setting SO_KEEPALIVE if the connection has been reset.
