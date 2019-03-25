#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "DataFormat.h"
#include "SocketUDP.h"

#pragma comment (lib, "Ws2_32.lib")

// MACROS
#define STEREO_QUALITY_VALUE (95u) // Quality percentage

// STRUCTURE

//
// Stereo module needs:
// REQ_STEREO_DATA
// 1. INPUT:   Two camera data
// 2. PROCESS: Create the Metadata, process the camera data to MJPEG format,
// 3. OUTPUT:  Send the MJPEG stream
//
typedef struct StereoObjectStruct {
  // INOUT: Metadata
  StereoPacket *pStereoPacket;

  // IN: RAW (obtained from camera are stored into)
  unsigned char *pFrameLeft;  // Stereo Left camera data 
  unsigned char *pFrameRight; // Stereo Right camera data 

  // JPEG buffer handler
  unsigned char *pJpegBufferWrite; // Stereo Right camera data 

  // SERVER: Network interfacing variables
  SockObject hSockObj;

} StereoObject;

// Stereo main execution function declaration

int StereoExecute_Start(StereoObject *pStereoObject);
int StereoExecute_Termination(StereoObject *pStereoObject);

// Stereo input processing function declarations

int StereoInput_Init(StereoObject *pStereoObject);
int StereoInput_Deinit(StereoObject *pStereoObject);
int StereoInput_FromCamera(StereoObject *pStereoObject);

// Stereo output processing function declarations

int StereoOutput_Init(StereoObject *pStereoObject);
int StereoOutput_Deinit(StereoObject *pStereoObject);
int StereoOutput_Packet(StereoObject *pStereoObject);

// Stereo data processing function declarations

int StereoProcess_ToJpeg(StereoObject *pStereoObject);
int StereoProcess_Request(StereoObject *pStereoObject);

// Network related API declarations
int SocketUDP_PrintIpPort(SOCKET *phSock, const char *pTagName);
int SocketUDP_RecvFrom(SOCKET *phSock, char *pDataBuf, int iDataSize,
    sockaddr *pSockCliAddr, int *pSockSize);

int SocketUDP_SendTo(SOCKET *phSock, char *pDataBuf, int iDataSize, 
    sockaddr *pSockClientAddr, int iSockSize);

int SocketUDP_InitServer(SOCKET *phSock, SOCKADDR_IN *phServAddr,
    int  iPortNum,   char *pServerIP);

int SocketUDP_Deinit(SOCKET*);
int SocketUDP_ServerInit();

