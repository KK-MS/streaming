
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
  Mat cam_frame;

  StereoPacket   *pPkt;
  StereoMetadata *pMeta;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;
  int iDatabytes;

  pPkt    = pStereoObject->pStereoPacket;
  pMeta = &(pPkt->stMetadata.stStereoMetadata);

  // get the frame pointers
  pFrameL = pStereoObject->pFrameLeft;
  pFrameR = pStereoObject->pFrameRight;

  Mat mGrayScaleLeft(Size(pMeta->uiFrameWidth, pMeta->uiFrameHeight), CV_8UC1);

  // Capture video frames
  hVLeft >> cam_frame;

  // Covert to gray scale
  cv::cvtColor(cam_frame, mGrayScaleLeft, CV_BGR2GRAY);

  // Note: Assignment of allocated buffer is not reliable.
  // OpenCV will allocate internal buffer. Not worked here !!, so memcpy
  // Copy the data to our object buffer.
  iDatabytes = mGrayScaleLeft.total() * mGrayScaleLeft.elemSize(); // pMeta->uiFrameBytes
  memcpy(pFrameL, mGrayScaleLeft.data, iDatabytes);

  ////////// RIGHT /////////////
  memcpy(pFrameR, mGrayScaleLeft.data, iDatabytes);

#ifdef WINDOW_STEREO_INPUT_DB
  // Debug: Show gray scale
  imshow(WINDOW_STEREO_INPUT_DB, mGrayScaleLeft);

  // Debug: Print the gray scale matrix details
  debug_mat(mGrayScaleLeft, "Gray_input");
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

  // get the initial frame to know the camera frame values
  hVLeft.read(frame);

  // fill the object
  pStereoObject->pStereoPacket = pPkt;
  pStereoObject->pFrameLeft     = pFrameL;
  pStereoObject->pFrameRight    = pFrameR;

  // fill the stereo metadata
  pMeta->uiFrameWidth  = frame.cols;
  pMeta->uiFrameHeight = frame.rows;
  pMeta->uiNumOfChannels = 1;
  pMeta->uiFrameBytes  = frame.cols * frame.rows * pMeta->uiNumOfChannels;

#ifdef DEBUG_STEREO_INIT
  namedWindow("Stereo_Init", cv::WINDOW_NORMAL);
  while(1) {
	hVLeft.read(frame);
	imshow("Stereo_Init", frame);
    cv::waitKey(10);
  }
#endif // DEBUG_STEREO_INIT
#ifdef WINDOW_STEREO_INPUT_DB
  namedWindow(WINDOW_STEREO_INPUT_DB, cv::WINDOW_AUTOSIZE);
#endif
  return 0;
}
