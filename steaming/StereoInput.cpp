
#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>      //for imshow

#include <stdio.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include "DataFormat.h"
#include "StereoObject.h"

using namespace std;
using namespace cv;

// Macros

#define TAG_SINP "SInp: "
//#define WINDOW_STEREO_INPUT_DB "StereoInputDebug"

// Debug enable macros
//#define DEBUG_STEREO_INIT (1u)

// Global variables
static VideoCapture   hVLeft; // pointer to the handler of video input
static VideoCapture   hVRight;

// Function definitions
void debug_mat(Mat &mat_obj, const char *mat_name)
{
  printf("Mat %s: H %d\n", mat_name, mat_obj.rows);
  printf("Mat %s: W %d\n", mat_name, mat_obj.cols);
  printf("Mat %s: T %d\n", mat_name, mat_obj.type());
  printf("Mat %s: D %p\n", mat_name, mat_obj.data);
  printf("Mat %s: S %d\n", mat_name, mat_obj.step);
  printf("Mat %s: C %d\n", mat_name, mat_obj.isContinuous());
}

int StereoInput_FromCamera(StereoObject *pStereoObject)
{
  StereoPacket   *pPkt;
  StereoMetadata *pMeta;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;
  int iDatabytes;
  int iFrameType;

#if (FRAME_CHANNELS == 1u)
  iFrameType = CV_8UC1;
#elif  (FRAME_CHANNELS == 3u)
  iFrameType = CV_8UC3;
#else
Error: in FRAME_CHANNELS
#endif 

  pPkt    = pStereoObject->pStereoPacket;
  pMeta = &(pPkt->stMetadata.stStereoMetadata);

  // get the frame pointers
  pFrameL = pStereoObject->pFrameLeft;
  pFrameR = pStereoObject->pFrameRight;
 
  Mat mLeft(Size(pMeta->uiFrameWidth, pMeta->uiFrameHeight), iFrameType, pFrameL);

  // Capture video frames
  hVLeft >> mLeft;

#if (FRAME_CHANNELS == 1u)
  // Covert to gray scale
  cv::cvtColor(mLeft, mLeft, CV_BGR2GRAY);
#endif

  // Note: Assignment of allocated buffer is not reliable.
  // OpenCV will allocate internal buffer. Not worked here !!, so memcpy
  // Copy the data to our object buffer.
  iDatabytes = mLeft.total() * mLeft.elemSize(); // pMeta->uiFrameBytes

  if (iDatabytes != pMeta->uiFrameBytes) {
	  printf("Error: iDatabytes:%d, uiFrameBytes:%d\n", iDatabytes, pMeta->uiFrameBytes); getchar(); return -1;
  }


  // Observed that given memory is overridding by MAT with dynamic memory
  // Thus copy manually.
  
  ////////// LEFT /////////////
  memcpy(pFrameL, mLeft.data, iDatabytes);

  ////////// RIGHT /////////////
  memcpy(pFrameR, mLeft.data, iDatabytes);

#ifdef WINDOW_STEREO_INPUT_DB
  // Debug: Show gray scale
  imshow(WINDOW_STEREO_INPUT_DB, mLeft);

  // Debug: Print the gray scale matrix details
  debug_mat(mLeft, "Input");
#endif // DEBUG_STEREO_INPUT

  return 0;

}


int StereoInput_Deinit(StereoObject *pStereoObject)
{
  printf("In stereo_input_deinit\n");

  // Release video handler
  hVLeft.release();

  // Free the memory
  free(pStereoObject->pFrameLeft);
  free(pStereoObject->pFrameRight);
  free(pStereoObject->pStereoPacket);

  return 0;
}

int StereoInput_Init(StereoObject *pStereoObject)
{
  Mat frame;
  int iVideoIDLeft  = 0; // Camera ID 0
  int iVideoIDRight = 1; // Camera ID 1

  StereoMetadata *pMeta;
  StereoPacket   *pPkt;

  unsigned char  *pFrameL;
  unsigned char  *pFrameR;
  int iFrameType;

#if (FRAME_CHANNELS == 1u)
  iFrameType = CV_8UC1;
#elif  (FRAME_CHANNELS == 3u)
  iFrameType = CV_8UC3;
#else
Error: in FRAME_CHANNELS
#endif  
  printf("In StereoInput_Init\n");

  // TODO: Ring buffer

  // Allocate memory stereo packet, i.e. metadata + jpeg frames bytes
  pPkt     = (StereoPacket *) malloc(sizeof(StereoPacket));
  pMeta    = &(pPkt->stMetadata.stStereoMetadata);

  // DEBUG: Init the unique seq with unique Timestamp value
  pPkt->stMetadata.stImuMetadata.ulTimestamp = 1;

  // Allocate memory for raw frame data
  pFrameL  = (uchar *) malloc(MAX_FRAME_SIZE);
  pFrameR  = (uchar *) malloc(MAX_FRAME_SIZE);

  if ((pPkt == NULL)
	  || (pFrameL == NULL)
	  || (pFrameR == NULL) ) {
	printf("Error: malloc\n"); return -1; }

  // Open the left side video input
  hVLeft.open(iVideoIDLeft);
  if (!hVLeft.isOpened()) {
    printf("Error: Couldn't open video:%d\n", iVideoIDLeft); return -1; }

  hVLeft.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
  hVLeft.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

  // get the initial frame to know the camera frame values
  hVLeft.read(frame);

#if (FRAME_CHANNELS == 1u)
  // Covert to gray scale
  cv::cvtColor(frame, frame, CV_BGR2GRAY);
#endif  

  if (frame.cols != FRAME_WIDTH) {
	  printf("Error: Frame cols is %d\n", frame.cols); return -1;
  }

  if (frame.rows != FRAME_HEIGHT) {
	  printf("Error: Frame rows is %d\n", frame.rows); return -1;
  }
  
  if (frame.channels() != FRAME_CHANNELS) {
	  printf("Error: Frame channels is %d\n", frame.channels()); return -1; }

  if (frame.type() != iFrameType) {
	  printf("Error: Frame type is %d\n", frame.type()); return -1;
  }

  // fill the object
  pStereoObject->pStereoPacket = pPkt;
  pStereoObject->pFrameLeft     = pFrameL;
  pStereoObject->pFrameRight    = pFrameR;

  // fill the stereo metadata
  pMeta->uiFrameWidth = FRAME_WIDTH; //frame.cols;
  pMeta->uiFrameHeight = FRAME_HEIGHT; //frame.rows;
  pMeta->uiNumOfChannels = FRAME_CHANNELS; //frame.channels(); //FRAME_CHANNELS;
  pMeta->uiFrameBytes = FRAME_SIZE; //frame.cols * frame.rows * frame.channels();

#ifdef DEBUG_STEREO_INIT
  namedWindow("Stereo_Init", cv::WINDOW_NORMAL);
  while(1) {
	hVLeft.read(frame);
	imshow("Stereo_Init", frame);
	//  wait until ESC key
	if (cv::waitKey(10) == 27)
		break;
  }
#endif // DEBUG_STEREO_INIT
#ifdef WINDOW_STEREO_INPUT_DB
  namedWindow(WINDOW_STEREO_INPUT_DB, cv::WINDOW_AUTOSIZE);
#endif
  return 0;
}
