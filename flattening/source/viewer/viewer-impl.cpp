#include <iostream>
// TUIO header files
#include "TuioClient.h"
#include "MyTuioListener.h"
#include "viewer.h"

 //
//**********TUIO Implementation**********
using namespace TUIO;
using namespace std;

std::list<TuioCursor*>::iterator it; 


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
   cout << "add " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << endl; 
   x= tcur->getX();      //gets the current x cordinate
   y= 1 - tcur->getY();  //gets the current y cordinate. On the viewer.cpp the higher the value the higher the objects goes 
   
   previousX=x;          
   previousY=y;
    
   std::list<TuioCursor*> cursors = client.getTuioCursors();
   it=cursors.begin();

  if(cursors.size() == 2)
  { 
      z = tcur->getDistance((*it));           
      previousZ=z;             
  }
     
}

		
void MyTuioListener::updateTuioCursor(TuioCursor *tcur)
{ 
 
  cout << "set " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() 
				<< " " << tcur->getMotionSpeed() << " " << tcur->getMotionAccel() << " " << endl;
     
   
   x= tcur->getX();               //gets the current x cordinate
   y= 1 - tcur->getY();           //gets the current y cordinate. On the viewer.cpp the higher the value the higher the objects goes
   

   std::list<TuioCursor*> cursors = client.getTuioCursors();
   it=cursors.begin();
/*
if(cursors.size() >= 2  && (  (it*)->getMotionSpeed() == ((++it)*)->getMotionSpeed()  ) )
  {          
    
      z = tcur->getDistance((*it));
      if(z==0)    
      {
         it++;
        z = tcur->getDistance((*it));
      }  
      obj_pos[2] += (z-previousZ)*10;    
      previousZ=z;     
      
    return;
  }
*/




   if(cursors.size() == 2)
  {          
    
      z = tcur->getDistance((*it));
      if(z==0)    
      {
         it++;
        z = tcur->getDistance((*it));
      }  
      obj_pos[2] += (z-previousZ)*10;    
      previousZ=z;     
      
    return;
  }
 
  
    obj_pos[0] += (x-previousX)*3;
    previousX=x;
        
    obj_pos[1] += (y-previousY)*3;
    previousY=y;
        
   
  
   for ( it=cursors.begin() ; it != cursors.end(); it++ )// this line was added by jose
        {
         cout << " " << (*it)->getX();      
      
        }
   
}

		
void MyTuioListener::removeTuioCursor(TuioCursor *tcur)
{
 cout << "del " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ")" << endl;
}
		
		
void MyTuioListener::refresh(TuioTime ftime)
{
}
//**************************************
