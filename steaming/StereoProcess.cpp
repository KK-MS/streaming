

#include <opencv2/highgui.hpp>      //for imshow

#include "DataFormat.h"
#include "StereoObject.h"

using namespace std;
using namespace cv;

// Macros
#define TAG_SPRS "SPrs: "
#define DEBUG_STEREO_JPEG_DB (1u)
#define WINDOW_JPEG_DB_RIGHT "Debug JPEG Decode Right"
#define WINDOW_JPEG_DB_LEFT "Debug JPEG Decode Left"


// FUNCTIONS
int StereoProcess_ToJpeg(StereoObject *pStereoObject)
{


	StereoPacket   *pPkt;
	StereoMetadata *pMeta;
	unsigned char  *pFrameL;
	unsigned char  *pFrameR;
	unsigned char  *pJpegWrite; // Write pointer
	std::vector<uchar> jpeg_buffer;

	static std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, STEREO_QUALITY_VALUE };

	// Get the required data locally
	pPkt = pStereoObject->stStereoPacket;
	pMeta = &(pPkt->stMetadata.stStereoMetadata);
	pJpegWrite = pPkt->ucJpegFrames;

	pFrameL = pStereoObject->pFrameLeft;
	pFrameR = pStereoObject->pFrameRight;

	////////////// RIGHT FRAME ///////////////////////////////
	// RIGHT: Create a MAT from our buffer
	Mat mGrayScaleRight(Size(pMeta->uiFrameWidth, pMeta->uiFrameHeight), CV_8UC1, pFrameR);

	// RIGHT: Covert each frame to .jpeg format
	imencode(".jpeg", mGrayScaleRight, jpeg_buffer, params);

	// RIGHT: Store the info and data in the packet
	pMeta->uiRightJpegSize = static_cast<unsigned int> (jpeg_buffer.size());

	// RIGHT: Copy the JPEG data
	memcpy(pJpegWrite,
		reinterpret_cast<char*>(jpeg_buffer.data()), jpeg_buffer.size());


	////////////// LEFT FRAME ///////////////////////////////
	// Increament the address to store other camera frame !!
	printf("b4 rs:%d\n", pMeta->uiRightJpegSize);
	pMeta->uiRightJpegSize = ALIGN(pMeta->uiRightJpegSize, ALIGN_ADDRESS_BYTE);
	printf("A4 rs:%d\n", pMeta->uiRightJpegSize);
	
	pJpegWrite += pMeta->uiRightJpegSize;

	// LEFT: Create a MAT from our buffer
	Mat mGrayScaleLeft(Size(pMeta->uiFrameWidth, pMeta->uiFrameHeight), CV_8UC1, pFrameL);

	// LEFT: Covert each frame to .jpeg format
	imencode(".jpeg", mGrayScaleLeft, jpeg_buffer, params);

	// LEFT: Store the info and data in the packet
	pMeta->uiLeftJpegSize = static_cast<unsigned int> (jpeg_buffer.size());
	
	// LEFT: Copy the JPEG data
	memcpy(pJpegWrite,
		reinterpret_cast<char*>(jpeg_buffer.data()), jpeg_buffer.size());
	

	printf(TAG_SPRS "JPEG Size R:%d, L:%d, T:%d\n", pMeta->uiRightJpegSize, 
		pMeta->uiLeftJpegSize, pMeta->uiRightJpegSize + pMeta->uiLeftJpegSize);


#ifdef DEBUG_STEREO_JPEG_DB

	//Create a Size(1, nSize) Mat object of 8 - bit, single - byte elements
	Mat decodedImage;
	uchar *pJpegRead;
	unsigned long iJpeg_size;

	// Get the JPEG base address to read
	pJpegRead = pPkt->ucJpegFrames;

	//////////// RIGHT ////////////////////////
	iJpeg_size = static_cast<unsigned int> (pMeta->uiRightJpegSize);
	
	// RIGHT: Create a MAT of JPEG data
	Mat rawDataRight(1, iJpeg_size, CV_8UC1, (void*)pJpegRead);
	
	// RIGHT: Decode JPEG to RAW
	decodedImage = imdecode(rawDataRight, false);

	if (decodedImage.data == NULL) {
		// Error reading raw image data
		printf("Stereo: Error: Could not decode right frame\n");
		while (cv::waitKey(1) < 1);
	}
	else {

		imshow(WINDOW_JPEG_DB_RIGHT, decodedImage);
		int bytes1 = static_cast<unsigned int> (rawDataRight.total() * rawDataRight.elemSize());
		int bytes2 = static_cast<unsigned int> (decodedImage.total() *
			decodedImage.elemSize());

		if (pMeta->uiFrameBytes != bytes2) {
			printf("Error: Decode: RIGHT frame size: 0x%x, jpeg frame size: 0x%x\n", bytes1, bytes2);
		}
		
	}

	printf("StereProcess: R Size:%d, Data: %x, %x, %x, %x, End: %x, %x, %x, %x\n",
		iJpeg_size,
		pJpegRead[0], pJpegRead[1], pJpegRead[2], pJpegRead[3],
		pJpegRead[iJpeg_size - 4], pJpegRead[iJpeg_size - 3], pJpegRead[iJpeg_size - 2], pJpegRead[iJpeg_size - 1]);

	//////////// LEFT ////////////////////////

	iJpeg_size = static_cast<unsigned int> (pMeta->uiLeftJpegSize);
	
	pJpegRead += pMeta->uiRightJpegSize;

	// LEFT: Create a MAT of JPEG data
	Mat rawDataLeft(1, iJpeg_size, CV_8UC1, (void*)pJpegRead);

	// RIGHT: Decode JPEG to RAW
	decodedImage = imdecode(rawDataLeft, false);

	if (decodedImage.data == NULL) {
		// Error reading raw image data
		printf("Stereo: Error: Could not decode left frame\n");
		cv::waitKey(0);
	}
	else {

		imshow(WINDOW_JPEG_DB_LEFT, decodedImage);
		int bytes1 = static_cast<unsigned int> (rawDataLeft.total() * rawDataLeft.elemSize());
		int bytes2 = static_cast<unsigned int> (decodedImage.total() *
			decodedImage.elemSize());

		if (pMeta->uiFrameBytes != bytes2) {
			printf("Error: Decode: LEFT frame size: 0x%x, jpeg frame size: 0x%x\n", bytes1, bytes2);
			cv::waitKey(0);
		}

	}
	printf("StereProcess: L Size:%d, Data: %x, %x, %x, %x, End: %x, %x, %x, %x\n",
		iJpeg_size,
		pJpegRead[0], pJpegRead[1], pJpegRead[2], pJpegRead[3],
		pJpegRead[iJpeg_size - 4], pJpegRead[iJpeg_size - 3], pJpegRead[iJpeg_size - 2], pJpegRead[iJpeg_size - 1]);

	// https://stackoverflow.com/questions/14727267/opencv-read-jpeg-image-from-buffer
#endif //DEBUG_STEREO_JPEG	
	return 0;
}
