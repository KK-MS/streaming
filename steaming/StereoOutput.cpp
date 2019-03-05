// TODO fix
// Severity Code  Description Project File  Line  Suppression State
// Error  C4996 'inet_addr': Use inet_pton() or InetPton() instead or define _WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings stereo  C : \prj\ocv\vs\gtruth\stereo\stereo\stereo_output.cpp  220
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <vector>
#include <iostream>
#include <iomanip>

// File operation
#include <fstream> 

// Threads: to run streaming server
#include <process.h>

// Network related includes
//#undef UNICODE

// Need older API support e.g. IPV4. Else result in "Error E0040"
// E0040 expected an identifier ws2def.h
#define WIN32_LEAN_AND_MEAN 

#include <windows.h> // DEFAULT_PORT
#include <ws2tcpip.h> // getaddrinfo, includes #include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include "DataFormat.h"
#include "StereoObject.h"

#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_BUFLEN 100
#define DEFAULT_PORT "27015"

using namespace std;

// MACROS
#define TAG_SOUT "SOut: " // Stereo module OUTPUT

// GLOBAL VARIABLES

static ofstream myFile;

static SOCKET ListenSocket = INVALID_SOCKET;
static SOCKET ClientSocket = INVALID_SOCKET;


//// OUTPUT TO FILE /////
int file_open() {

  // File: out, binary, not appended, thus will reset size to zero
  myFile.open("..\\stereo_out_right.mjpeg",
    ios::out | ios::binary); // | ios::app to append
  if (!myFile.is_open()) {
    std::cout << "Output operation not successful\n";
    return -1;
  }
  return 0;
}

int file_write(std::vector<unsigned char> &buf) {
  char* data = reinterpret_cast<char*>(buf.data());

  // Program assume the file is already opened.
  myFile.write(data, buf.size());
  return 0;
}

//// NETWORK /////
// 
// Server with UDP connection
//
#define INPUT_SERVER_PORT     27015 
#define INPUT_SERVER_IP     "127.0.0.1"
static SOCKET s;
static SOCKADDR_IN servaddr;

#if 0
int stream_to_network(std::vector<unsigned char> &buf)
{
  char* data = reinterpret_cast<char*>(buf.data());

  // Program assume the file is already opened.
  myFile.write(data, buf.size());
  return 0;
}

//int stereo_output_camera(std::vector<unsigned char> &buf)
int stereo_output_camera(unsigned char *frame_buffer)
{
  std::vector<unsigned char> vec_frame_buffer;

  vec_frame_buffer.insert(vec_frame_buffer.end(), frame_buffer, frame_buffer + FRAME_SIZE);

  //file_write(buf);
  //stream_to_network(buf);

  return 0;
}



int StereoOutput_Packet1(StereoObject *pStereoObject)
{
  int ret_val;

  // Network related variables
  sockaddr cliaddr;
  int len = sizeof(sockaddr_in);

  // Metadata variables
  char req_msg[MAX_REQ_SIZE + 1]; // +1 to add null at last
  const char *res_meta = (const char *)& (pStereoObject->meta_pkt);
  int len_meta = sizeof(pStereoObject->meta_pkt);

  // stereo data variables
  char req_imgs[] = REQ_IMAGES;
  char *res_imgs = (char *)pStereoObject->ptr_jpeg_stream;

  int len_imgs = pStereoObject->meta_pkt.jpeg_left_size
    + pStereoObject->meta_pkt.jpeg_right_size;

  // Process command metadata
  cout << TAG_SOUT "recvfrom for REQ_METDATA" << endl;
  if ((ret_val = recvfrom(s, req_msg, MAX_REQ_SIZE, 0, &cliaddr, &len)) < 0) { goto ret_err; }
  req_msg[ret_val] = '\0';


  if (strcmp(req_msg, REQ_METADATA) != 0) {
    cout << TAG_SOUT "Error: expected REQ_METDATA, received:" << req_msg << endl;
    goto ret_err;
  }

  cout << TAG_SOUT "sendto for REQ_METDATA" << endl;
  if (ret_val = sendto(s, res_meta, sizeof(metadata_packet), 0, &cliaddr, len) < 0) { goto ret_err; }

  // Process command to send stereo images
  cout << TAG_SOUT "recvfrom for REQ_IMAGES" << endl;
  if ((ret_val = recvfrom(s, req_msg, MAX_REQ_SIZE, 0, &cliaddr, &len)) < 0) { goto ret_err; }

  cout << TAG_SOUT "sendto for REQ_IMAGES" << endl;
  if (strcmp(req_msg, REQ_IMAGES) != 0) {
    cout << TAG_SOUT "Error: expected REQ_IMAGES, received:" << req_msg << endl;
  }
  cout << TAG_SOUT "REQ_IMAGES, size:" << len_imgs << endl;

  if (len_imgs > 65530) len_imgs = 6553; ////////////////////// only for test! of UDP,  TODO: in UDP max packet looping

  if ((ret_val = sendto(s, res_imgs, len_imgs, 0, &cliaddr, len)) < 0) { goto ret_err; }

  return 0;

ret_err:
  printf(TAG_SOUT "Error: sending %s, ret:%d, NetErr:%d\n", req_msg, ret_val, WSAGetLastError());
  return -1;

}
#endif

int StereoOutput_Packet(StereoObject *pStereoObject)
{
  int iRetVal;
 
  // REQUEST holder
  char req_msg[MAX_REQ_SIZE + 1]; // +1 to add null at last

  // local variable point to the allocated buffers
  StereoPacket   *pPkt;
  StereoMetadata *pMeta;
  
  // Network related variables
  sockaddr sockClientAddr;
  int iLenSockClient = sizeof(sockaddr_in);
  
  // Packet details
  const char *pPktBuf;
  const char *pSendBuf;
  int iPktLen;
  int iSendLen;
  int iRemainLen;

  // initialize local variables
  pPkt        = pStereoObject->stStereoPacket;
  pPktBuf = (const char *)pPkt;
  pMeta        = &(pPkt->stMetadata.stStereoMetadata);

  iPktLen = sizeof(Metadata) + pMeta->uiRightJpegSize + pMeta->uiLeftJpegSize;

  // Receive the request
  printf(TAG_SOUT "Waiting for request\n");
  iRetVal = recvfrom(s, req_msg, MAX_REQ_SIZE, 0, &sockClientAddr, &iLenSockClient);
  if (iRetVal < 0 ) { goto ret_err; }

  if (strcmp(req_msg, REQ_STREAM) != 0) {
    cout << TAG_SOUT "Error: expected REQ_STREAM, received:" << req_msg << endl;
    goto ret_err;
  }

  // Transmit the data in junk of MAX UDP pakcet.
  pSendBuf = pPktBuf;
  iRemainLen = iPktLen;

  while(iRemainLen != 0) {

    iSendLen = iRemainLen;
    if (iRemainLen > MAX_UDP_DATA_SIZE) {
      iSendLen = MAX_UDP_DATA_SIZE;
    }

	printf(TAG_SOUT "sendto: len:%d. M:%d, rj:%d, lj:%d\n", iSendLen, sizeof(Metadata), pMeta->uiRightJpegSize, pMeta->uiLeftJpegSize);
    iRetVal = sendto(s, pSendBuf, iSendLen, 0, &sockClientAddr, iLenSockClient);

    if (iRetVal < 0) { goto ret_err; }
    else { 
      iRemainLen -= iRetVal;
      pSendBuf += iRetVal;
    }

  }
  return 0;
ret_err:
  printf(TAG_SOUT "Error: sending %s, ret:%d, NetErr:%d\n", req_msg, iRetVal, WSAGetLastError());
  return -1;

}



int network_deinit() {

  // deinit the network connection
  closesocket(s);
  WSACleanup();

  cout << "stereo network_deinit ends" << endl;

  return 0;
}


int StereoOutput_Deinit(StereoObject *ptr_stereo_object)
{

#ifdef DEBUG_OUTPUT_FILE
  myFile.close();
#endif // DEBUG_OUTPUT_FILE

  network_deinit();

  return 0;
}



int NetworkServerInit()
{
  WSADATA wsaData;
  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return 1;
  }

  //Create socket
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s == INVALID_SOCKET) {
    printf("Error localize_network_input_init socket %d\n", WSAGetLastError());
    return false; //Couldn't create the socket
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(INPUT_SERVER_PORT); //Port to connect on
  servaddr.sin_addr.s_addr = inet_addr(INPUT_SERVER_IP);
  //servaddr.sin_port = IPPROTO_UDP;

  // Setup the UDP listening socket
  iResult = bind(s, (struct sockaddr*) &servaddr, (int)sizeof(servaddr));
  if (iResult < 0) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    closesocket(s);
    WSACleanup();
    return 1;
  }

  // cout << "Starting thread" << endl;
  // gTerminateServer == FALSE;
  // hServerThread = (HANDLE)_beginthread(network_udp_server, 0, NULL);

  return 0;
}
int StereoOutput_Init(StereoObject *pStereoObject)
{
  printf("In stereo_output_init\n");

#ifdef DEBUG_OUTPUT_FILE
  file_open();
#endif // DEBUG_OUTPUT_FILE

  if (NetworkServerInit() != 0) {
    printf("Error: In NetworkServerInit\n");
    return -1;
  }
  printf("NetworkServerInit ... OK\n");
  return 0;
}

// Error code links:
// https://docs.microsoft.com/en-us/windows/desktop/debug/system-error-codes--0-499-
//
// https://docs.microsoft.com/de-de/windows/desktop/WinSock/windows-sockets-error-codes-2
// WSAEINTR 10004: Interrupted function call. 
//    A blocking operation was interrupted by a call to WSACancelBlockingCall.
// WSAEFAULT 10014: Bad address. The system detected an invalid pointer address 
//    in attempting to use a pointer argument of a call. 
//    This error occurs if an application passes an invalid pointer value, 
//    or if the length of the buffer is too small. For instance, if the length 
//    of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).
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
//    Any other type of operation might also return this error—for example, 
//    setsockopt setting SO_KEEPALIVE if the connection has been reset.
