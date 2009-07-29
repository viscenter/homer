
#ifndef __VIEWER_H__
#define __VIEWER_H__
#include "TuioClient.h"
extern TUIO::TuioClient client;
extern float x;
extern float y;
extern float z; 
extern float obj_pos[3];
extern float previousZ;  // stores the last value of z

#endif

