

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
#define DEBUG_STEREO_JPEG   (1u)

#define TAG_LOC "STE: "

// Namespace
using namespace std;
using namespace cv;

// Thread, scheduler
static HANDLE hStreamingScheduler;

//
// StereoExecute_Scheduler
//
// This is the localize process scheduler.
// It schedules the capture of Metadata, and stereo cameras frames.
// Convert the Camera Raw Frame to compressed JPEG format,
// and transmit the metadata and frames when requested.
//
void StereoExecute_Scheduler(void *param)
{
  int iRetVal = 0;

  StereoObject *pStereoObject = (StereoObject *)param;
  unsigned long ulTimeStamp;

  int iLoopCount = 0;

  // Enter into scheduling
  while (1) {

    // IMU Data + camera images
    //iRetVal = StereoInput_Metadata(pStereoObject);
    //if (iRetVal) { goto err_ret; }

    // DEBUG print. Time stamp
    ulTimeStamp = pStereoObject->pStereoPacket->stMetadata.stImuMetadata.ulTimestamp++;
    printf("\n\n ----------------------------------------------- [%d]\n", ulTimeStamp);

    // INPUT: CAMERA Images (two camera data)
    iRetVal = StereoInput_FromCamera(pStereoObject); if (iRetVal) { goto err_ret; }

    // PROCESS: To JPEG
    StereoProcess_ToJpeg(pStereoObject); if (iRetVal) { goto err_ret; }

    // OUTPUT:e.g. Images
    iRetVal = StereoOutput_Packet(pStereoObject); if (iRetVal) { goto err_ret; }

    //  wait until ESC key
    if (cv::waitKey(10) == 27)
      break;

err_ret:
  destroyAllWindows();
  printf("%s: thread closing: %d\n", __func__, iRetVal);
}

//
// StereoExecute_Termination
//
// Termination will wait for the schduler thread to be terminated,
// free the allocated resources in the thread and then
// terminate main process.
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

//
// main
//
// main function of the stereo module.
//
// Create stereo object, init the input and output resources
// Start the stereo module process
//
int main()
{

  StereoObject *pStereoObject;

  pStereoObject = (StereoObject *)malloc(sizeof(StereoObject));
  if (!(pStereoObject)) { printf("Error: malloc\n"); return -1; }

  // Initialize the input interfaces
  // Input: IMU data + Stereo Camera data
  StereoInput_Init(pStereoObject);

  // Initialize the output interface.
  // Output is a stream of IMU data + Stereo camera frames
  StereoOutput_Init(pStereoObject);

  // Start the stereo module process
  StereoExecute_Start(pStereoObject);

  cv::waitKey(0);

  return 0;
}
