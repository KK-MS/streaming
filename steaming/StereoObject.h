#pragma once

#include "DataFormat.h"

// MACROS
#define STEREO_QUALITY_VALUE (95u) // Quality percentage

typedef struct StereoObjectStruct {
	// INOUT: Metadata
	StereoPacket *stStereoPacket;

	// IN: RAW (obtained from camera are stored into)
	unsigned char *pFrameLeft;  // Stereo Left camera data 
	unsigned char *pFrameRight; // Stereo Right camera data 

	// JPEG buffer handler
	unsigned char *pJpegBufferWrite; // Stereo Right camera data 

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
