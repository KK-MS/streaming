

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

// Threads: to run streaming server
#include <process.h>

#include <opencv2/highgui.hpp> // for waitkey()

#include "DataFormat.h"
#include "StereoObject.h"

// MACROS
#define WINDOW_STEREO_INPUT "Stereo Input"
#define WINDOW_STEREO_OUTPUT "Stereo Output"
#define WINDOW_STEREO_JPEG   "Stereo JPEG"
#define WINDOW_STEREO_DEBUG   "Stereo Debug"

// uncomment the require debug
#define DEBUG_STEREO_INPUT  (1u)
#define DEBUG_STEREO_OUTPUT (0u)
#define DEBUG_STEREO_DEBUG  (0u)
#define DEBUG_STEREO_JPEG   (0u)

#define TAG_LOC "STE: "

// Namespace
using namespace std;
using namespace cv;

// Thread, scheduler
static HANDLE hStreamingScheduler;

#if (DEBUG_STEREO_INPUT | DEBUG_STEREO_OUTPUT |DEBUG_STEREO_JPEG | DEBUG_STEREO_DEBUG )

void OpenDisplayWindows()
{
#ifdef DEBUG_STEREO_INPUT
	namedWindow(WINDOW_STEREO_INPUT, WINDOW_NORMAL);
#endif // DEBUG_STEREO_INPUT

#if DEBUG_STEREO_OUTPUT
	namedWindow(WINDOW_STEREO_OUTPUT, WINDOW_NORMAL);
#endif // DEBUG_STEREO_OUTPUT

#if DEBUG_STEREO_JPEG
	namedWindow(WINDOW_STEREO_JPEG, WINDOW_NORMAL);
#endif // DEBUG_STEREO_JPEG

#if DEBUG_STEREO_DEBUG
	namedWindow(WINDOW_STEREO_DEBUG, WINDOW_NORMAL);
#endif // DEBUG_STEREO_DEBUG
}
#else
void OpenDisplayWindows() {}
#endif

void debug_mat(Mat &mat_obj, const char *mat_name);
#if (DEBUG_STEREO_INPUT | DEBUG_STEREO_OUTPUT |DEBUG_STEREO_JPEG | DEBUG_STEREO_DEBUG )
void ImageShowDebug(StereoObject *pStereoObject)
{
	Mat cam_frame;

	StereoPacket   *pPkt;
	StereoMetadata *pMeta;
	unsigned char  *pFrameL;
	unsigned char  *pFrameR;

	pPkt = pStereoObject->stStereoPacket;
	pMeta = &(pPkt->stMetadata.stStereoMetadata);

	pFrameL = pStereoObject->pFrameLeft;
	pFrameR = pStereoObject->pFrameRight;

	printf("Ex1 %x %x %x %x i:%d\n", pFrameL[0], pFrameL[1], pFrameL[2], pFrameL[3], pMeta->uiFrameBytes);
	
	Mat mGrayScaleLeft(Size(pMeta->uiFrameWidth, pMeta->uiFrameHeight), CV_8UC1, pFrameL);

#if DEBUG_STEREO_INPUT
	imshow(WINDOW_STEREO_INPUT, mGrayScaleLeft);
	debug_mat(mGrayScaleLeft, "Exe_input");
#endif // DEBUG_STEREO_INPUT
	
	
	//cv::waitKey(0);
}
#else
void ImageShowDebug(StereoObject *pStereoObject) {}
#endif

/**
* StereoExecute_Scheduler
*
* This is the localize process scheduler.
* It schedules the video capture from camera,
* convert the Camera Raw Frame to compressed JPEG format,
* and transmit the metadata and frames when requested.
*/
void StereoExecute_Scheduler(void *param)
{
	int ret_val = 0;

	StereoObject *pStereoObject = (StereoObject *)param;

	// for debug: open the required windows
	OpenDisplayWindows();

	// enter into scheduling
	while (1) {

		// IMU DATA + CAMERA IMAGEs
		//ret_val = StereoInput_Metadata(pStereoObject);
		//if (ret_val) { goto err_ret; }

		// OUTPUT => e.g. METADATA
		//ret_val = stereo_output_request(stereo_packet);
		//if (ret_val) { goto err_ret; }

		// INPUT: CAMERA IMAGEs (TWO Camera data)
		ret_val = StereoInput_FromCamera(pStereoObject);
		if (ret_val) { goto err_ret; }
		//cv::waitKey(10);

		// PROCESS: TO JPEG
		StereoProcess_ToJpeg(pStereoObject);
		if (ret_val) { goto err_ret; }

		//ret_val = StereoInput_Init(pStereoObject);
		//if (ret_val) { goto err_ret; }

		// OUTPUT:e.g. IMAGES
		ret_val = StereoOutput_Packet(pStereoObject);
		//ret_val = stereo_output_request(ptr_stereo_object);
		if (ret_val) { goto err_ret; }

		ImageShowDebug(pStereoObject);

		//  wait until ESC key
		if (cv::waitKey(10) == 27) { // delay: Tune it.
			break;
		}
		
	}

err_ret:
	destroyAllWindows();
	printf("%s: thread closing: %d\n", __func__, ret_val);
}

//
// StereoExecute_Termination
// 
// Termination will wait for the schduler thread to be terminated,
// free the allocated resources and then terminate main process.
// 
int StereoExecute_Termination(StereoObject *pStereoObject)
{
	// Terminate will only wait for the thread to exit and
	// exit the module gracefully.

	// WAIT for Server loop to end
	printf("Stereo: Starting stereo_terminate\n");
	WaitForSingleObject(hStreamingScheduler, INFINITE);
	printf("Stereo: ending stereo_terminate\n");

	return 0;
}

// 
// StereoExecute_Start
// 
// Starts the execution processing by starting the scheduler.
//
int StereoExecute_Start(StereoObject *pStereoObject)
{
	// start stereo module scheduler
	hStreamingScheduler =
		(HANDLE)_beginthread(StereoExecute_Scheduler, 0, (void *)pStereoObject);


	return 0;
}


int main()
{

	StereoObject *pStereoObject;

	pStereoObject = (StereoObject *)malloc(sizeof(StereoObject));
	if (!(pStereoObject)) { printf("Error: malloc\n"); return -1; }

	// Initialize the input interfaces
	StereoInput_Init(pStereoObject);

	// Initialize the output interface.
	// Output is a stream of IMU data + Stereo camera frames
	StereoOutput_Init(pStereoObject);

	StereoExecute_Start(pStereoObject);

	cv::waitKey(0);

	return 0;
}
