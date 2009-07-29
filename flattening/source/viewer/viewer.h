
#ifndef __VIEWER_H__
#define __VIEWER_H__
#include "TuioClient.h"
#include "glui.h"
#include <queue>
#include <map>
#include <pthread.h>
 
 enum state  {mouseDown, mouseHeldDown, mouseUp};  
 struct XY // contains the x and y cordinates for the rotation part 
 {  
   int X;
   int Y;
   state mouseState;
 };

  

 

extern TUIO::TuioClient client;
extern float x;
extern float y;
extern float z; 
extern float obj_pos[3];
extern float previousZ;  // stores the last value of z
extern float view_rotate[16]; 
extern bool  TranState;  // "True" if there are 3 or more active cursors, "False" oherwise
extern bool  ZoomState;  // "True" if there are 2 active cursors, "False" oherwise
extern bool  RotState;   // "True" if there is 1 active cursor, "False" oherwise
extern GLUI_Rotation *view_rot; 
extern std::queue<XY> mouseEvents;
extern pthread_mutex_t queue_mutex;
         

#endif

