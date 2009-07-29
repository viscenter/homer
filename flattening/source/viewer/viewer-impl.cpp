#include <iostream>
#include <map>
// TUIO header files
#include "TuioClient.h"
#include "MyTuioListener.h"
#include "viewer.h"
#include <glui.h>
#include <pthread.h>
//
//**********TUIO Implementation**********
using namespace TUIO;
using namespace std;



struct cursorInfo
{
  float x_value; 
  float y_value;
  float speed;
  float acceleration;
  //pthread_mutex_t mutex; 
};
bool TranState= false; // "True" if the are 3 or more active cursors, "False" oherwise
bool ZoomState= false;  // "True" if there are 2 active cursors, "False" oherwise
bool  RotState= false;   // "True" if there is 1 active cursor, "False" oherwise
std::list<TuioCursor*>::iterator it; 
std::map<int,cursorInfo>::iterator itmap; // maybe this will be erased
std::map<int,cursorInfo> previousCursorValues; // the first slot contains the Cursor ID and the second contains an instance of cursorInfo.
std::queue<XY> mouseEvents;


MyTuioListener::MyTuioListener() : TuioListener()
{


}


MyTuioListener::~MyTuioListener()
{

}
		
		
void MyTuioListener::addTuioObject(TuioObject *tobj)
{
 //cout << "add obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << ") "<< tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() << endl;
}

		
void MyTuioListener::updateTuioObject(TuioObject *tobj)
{

}
		
		
void MyTuioListener::removeTuioObject(TuioObject *tobj)
{
}
		
		
void MyTuioListener::addTuioCursor(TuioCursor *tcur)
{   
 // the information of the cursor is collected
  cursorInfo a;
  a.x_value = tcur->getX(); 
  a.y_value = 1-tcur->getY();
  a.speed= tcur->getMotionSpeed();
  a.acceleration= tcur->getMotionAccel();
 // pthread_mutex_init(&a.mutex , NULL); 
  previousCursorValues.insert ( pair<int,cursorInfo>(tcur->getCursorID(),a) ); // the cursor is inserted in the list
  
  
/*
// showing contents:
  //cout << " previousCursorValues contains:\n";
  for ( itmap=previousCursorValues.begin() ; itmap != previousCursorValues.end(); itmap++ )
   {
  //  pthread_mutex_lock( &((*itmap).second).mutex );
      cerr << "Id="<<(*itmap).first <<" x_value="<< ((*itmap).second).x_value <<" y_value="<< ((*itmap).second).y_value<<endl;
      //cerr <<" speed="<< ((*itmap).second).speed <<" acceleration="<< ((*itmap).second).acceleration<<endl;
    //pthread_mutex_unlock( &((*itmap).second).mutex );
   }
cout<<endl;
*/

   cout << "add " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << endl; 
   x= tcur->getX();      //gets the current x cordinate
   y= 1 - tcur->getY();  //gets the current y cordinate. On the viewer.cpp the higher the value the higher the objects goes 
   previousX=x;    // stores the last value of x
   previousY=y;    // stores the last value of y 
   pre_rotX=71;    // stores the last value of x for the rotation part
   pre_rotY=153;   // stores the last value of y for the rotation part
   
   
   
   std::list<TuioCursor*> cursors = client.getTuioCursors();
   it=cursors.begin();
   
  if(cursors.size() == 1)
   {
    RotState= true;
    XY Coordinates = { 71, 153, mouseDown};
    mouseEvents.push (Coordinates);
   }
   
  if(cursors.size() == 2)
  { 
      ZoomState=true;
      z = tcur->getDistance((*it));           
      previousZ=z;             
  }
  if(cursors.size() >= 3)
      TranState= true;
          
    
  
}

		
void MyTuioListener::updateTuioCursor(TuioCursor *tcur)
{ 
   static float o=0; //erase this
   float SimpleSimX,SimpleSimY;  // this are the x and y cordinates of the SimpleSimulator
   bool sameDirection=false;      // this variable decides if the cursors are moving in the same direction or in oposite directions.
   cout << "set "<< tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() 
				<< " " << tcur->getMotionSpeed() << " " << tcur->getMotionAccel() << " " << endl;
  //float view_rotate[16] = { 0.776922,-0.418051,0.470771,0, -0.038177,0.715077,0.698002,0, -0.628438,-0.560266,0.539599,0, 0,0,0,1 }; // **erase this**
   //o+=0.1;  //erase this
   //view_rotate[0]=o; //erase this
   std::list<TuioCursor*> cursors = client.getTuioCursors();
   it=cursors.begin();
   
   x= tcur->getX();               //gets the current x cordinate
   y= 1 - tcur->getY();           //gets the current y cordinate. On the viewer.cpp the higher the value the higher the objects goes

  
    if(cursors.size() < 3)
      TranState= false;
    if(cursors.size() != 2)
      ZoomState= false;
   // if(RotState == true)            // maybe it will be erased
     // ZoomState= false;            // if the user wants to rotate and puts another finger by mistake this line will keep the rotation state
   //***********************Rotation Part**************************************
   //**************************************************************************
    if(/*cursors.size() == 1 &&*/ RotState== true && TranState == false && ZoomState == false)
     { 
       if(x==0) //Sometimes x goes to '0' for unknown reasons
        return;
       if(y==0) //Sometimes y goes to '0' for unknown reasons
        return;
       
       float x_rot = (pre_rotX + 150 *(x-previousX))
            ,y_rot = (pre_rotY - 150 *(y-previousY));
        
       XY Coordinates = {  x_rot,   y_rot , mouseHeldDown};
       mouseEvents.push (Coordinates); 
       pre_rotX = x_rot;
       pre_rotY = y_rot;
       previousX= x;
       previousY= y;
       return;
     }
   //**************************************************************************
   //**************************************************************************







  
 


  if(cursors.size() < 3)
      TranState= false;
  if(cursors.size() != 1)
      RotState= false;
  
 //***********************Zooming Part***************************************
 //**************************************************************************
   
   it=cursors.begin();
   if(/*cursors.size() == 2 &&*/ ZoomState==true && TranState == false && RotState == false)
  {          
     
      z = tcur->getDistance((*it));
      
      if(z==0)    
      {
         it++;
        z = tcur->getDistance((*it));
      }  
      
      obj_pos[2] += ( (previousZ) - z)*10;    
      previousZ=z;     
      
    return;
  }
 //**************************************************************************** 
 //****************************************************************************
  

 if(cursors.size() != 1)
      RotState= false;
 if(cursors.size() != 2)
      ZoomState= false;
   
 //************************Translation Part************************************
 //**************************************************************************** 
   if(/*cursors.size() >= 3 && */TranState == true && RotState == false && ZoomState == false)
   {
     if(x==0) //Sometimes x goes to '0' for unknown reasons, maybe this line will be deleted
      return;
     if(y==0) //Sometimes y goes to '0' for unknown reasons, maybe this line will be deleted
      return;
     itmap=previousCursorValues.find (tcur->getCursorID());
     float prevX =  (((*itmap).second).x_value);
     float prevY =  (((*itmap).second).y_value);
    
       if((y-prevY) > 0.4)        // maybe this will be errased 
          {
           cerr<<"jumpY!!!!!!!!!!!!!!!!!"<<endl;
           return;
          }
       if((x-prevX) > 0.4)        // maybe this will be errased 
          {
           cerr<<"jumpX!!!!!!!!!!!!!!!!!"<<endl;
           return;
          }

     cerr<<"prevX="<<prevX<<endl;//erase this
   //obj_pos[0] += (x-previousX)*3;
     obj_pos[0] += (x-prevX)*3;
   //  previousX=x;
     cerr<<"prevY="<<prevY<<endl;//erase this
   //obj_pos[1] += (y-previousY)*3;
     obj_pos[1] += (y-prevY)*3;
     //previousY=y;
       
    //itmap=previousCursorValues.find (tcur->getCursorID());    
   
   (((*itmap).second).x_value)      = x; 
   (((*itmap).second).y_value)      = y;
   (((*itmap).second).speed)        = tcur->getMotionSpeed();
   (((*itmap).second).acceleration) = tcur->getMotionAccel();
  }
 //****************************************************************************
 //****************************************************************************
cerr<<endl; // errase this
}

		
void MyTuioListener::removeTuioCursor(TuioCursor *tcur)
{         
  cout << "del " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ")" << endl;
  previousCursorValues.erase (tcur->getCursorID()); 
  std::list<TuioCursor*> cursors = client.getTuioCursors();
  if(cursors.size() < 3)
      TranState= false;
  if(cursors.size() != 2)
      ZoomState= false;
  if(cursors.size() != 1)
      RotState= false;
  
  cerr<<"TranState="<<( TranState ? "true"  : "false")<<" cursorsSize="<<cursors.size()<<endl;
  cerr<<"ZoomState="<<( ZoomState ? "true"  : "false")<<" cursorsSize="<<cursors.size()<<endl;
  cerr<<"RotState=" <<( RotState  ? "true"  : "false")<<" cursorsSize="<<cursors.size()<<endl;
  //XY Coordinates = { pre_rotX,  pre_rotY, mouseUp};
  //mouseEvents.push (Coordinates);      
}
		
		
void MyTuioListener::refresh(TuioTime ftime)
{
//cerr<<view_rotate[0]<<" "<<view_rotate[1]<<" "<<view_rotate[2]<<" "<<view_rotate[3]
//<<" "<<view_rotate[4]<<" "<<view_rotate[5]<<" "<<view_rotate[6]<<" "<<view_rotate[7]
//<<" "<<view_rotate[8]<<" "<<view_rotate[9]<<" "<<view_rotate[10]<<" "<<view_rotate[11]
//<<" "<<view_rotate[12]<<" "<<view_rotate[13]<<" "<<view_rotate[14]<<" "<<view_rotate[15]<<endl;// erase this
//cerr<<endl;
}
//**************************************
