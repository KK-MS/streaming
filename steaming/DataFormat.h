#pragma once

#define FRAME_TYPE     (16) // CV_8UC3
#define FRAME_WIDTH    (1280u)
#define FRAME_HEIGHT   (720u)
#define FRAME_CHANNELS (3u) // 1 => Grayscale, 3=> Color RGB/HSI
#define FRAME_SIZE     (FRAME_WIDTH * FRAME_HEIGHT * FRAME_CHANNELS)
#define MAX_FRAME_SIZE FRAME_SIZE

#define ALIGN_ADDRESS_BYTE      (4u)
#define ALIGN(x,a)              __ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

// Network macros
#define MAX_UDP_DATA_SIZE (65000u)

// Network Socket details
#define SOCK_PORT_IMU    (27014)
#define SOCK_PORT_STEREO (27015)
#define SOCK_PORT_GTMAP  (27016)
#define SOCK_IP_STEREO   "127.0.0.1" 
#define SOCK_IP_GTMAP    "127.0.0.1" 
#define SOCK_IP_IMU      "127.0.0.1" 

// Request tags
#define REQ_STREAM       "meta+jpegs"
#define REQ_METADATA     "metadata"
#define REQ_IMAGES       "stereo_images"
#define REQ_FRAME_LEFT   "frame_left"
#define REQ_FRAME_RIGHT  "frame_right"
#define REQ_TRAFFISIGNS  "trafficSigns"
#define REQ_TS_INFO      "trafficSignsInfo"

#define REQ_GTMAP_BASE    (400u)
#define REQ_GTMAP_MARKS   (REQ_GTMAP_BASE + 1u)
#define REQ_GTMAP_CALC    (REQ_GTMAP_BASE + 2u)

#define MAX_REQ_CMD_SIZE (64u)

typedef unsigned char uchar;

typedef struct ImuMetadata {
  // IMU values. Written by IMU
  unsigned long  ulTimestamp;
  unsigned int   uiLatitude;
  unsigned int   uiLongitude;
  // TODO: other IMU variables should be appended
} ImuMetadata;

typedef struct StereoMetadata {
  // Image packet. Written by stereo
  unsigned int  uiLeftJpegSize;
  unsigned int  uiRightJpegSize;
  //unsigned int  uiJpegsSize;

  // Image features. Written by stereo
  unsigned int  uiFrameWidth;
  unsigned int  uiFrameHeight;
  unsigned int  uiNumOfChannels;
  unsigned int  uiFrameBytes;

} StereoMetadata;

typedef struct GTMapMetadata {
  // Object count
  int iObjectCount;
  int aObjectType[8];
} GTMapMetadata;

typedef struct LocalizeMetadata {
  // Localized values.
  unsigned int   uiLocLat;
  unsigned int   uiLocLong;
  unsigned int   uiAccLat;
  unsigned int   uiAccLong;
} LocalizeMetadata;

typedef struct Metadata {
  ImuMetadata      stImuMetadata;
  StereoMetadata   stStereoMetadata;
  GTMapMetadata    stGTMapMetadata;
  LocalizeMetadata stLocalizeMetadata;
} Metadata;


typedef struct StereoPacket
{
  // Metadata
  Metadata stMetadata;

  // stereo camera 2 x JPEG frames, each frame max size MAX_FRAME_SIZE
  unsigned char ucJpegFrames[MAX_FRAME_SIZE * 2];

}StereoPacket;

// Request for the object list
typedef struct GTMapItemStruct
{
  unsigned int iType;
  unsigned int iGTMapLat;
  unsigned int iGTMapLong;
  unsigned int iRelativeDistance;
  unsigned int iDescriptorLength;
  unsigned char ucDescriptor[2000];

} GTMapItem;

// REQ_TRAFFIC_SIGNS: Request for the Item list
//
// REQ_CAL_COORDINATES: Request for calculating relative coordinates
// This will make sure that co-ordinates (lat-long) to 
// real world (x,y,z) function used by GTMap is used.
typedef struct GTMapPacket
{
  // Request type
  int         iRequestType;

  // Request data
  ImuMetadata stImuMetadata;

  // Response for REQ_CAL_COORDINATES
  LocalizeMetadata stCalCoordinate;

  // Response data for REQ_TRAFFIC_SIGNS
  int          iItemCount;
  GTMapItem    aItemInfo[5];

} GTMapPacket;


#if 0
typedef struct GTMapCoordinates
{
  // Request type
  int iRequestType;

  // Request data
    unsigned int iGTMapLat;
    unsigned int iGTMapLong;

    unsigned int iCalDist;

    // Response data
    unsigned int iCalLat;
    unsigned int iCalLong;
} GTMapRequest;
#endif

typedef struct LocalizePacket
{
  // Metadata with localized values stored
  Metadata stMetaPkt;
}LocalizePacket;
