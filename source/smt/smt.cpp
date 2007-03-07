
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "smc.h"

bool countdisplay = true, screenshot = false;
int framecount = 10000;

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	
	glPolygonMode(GL_FRONT,GL_FILL);
	glLineWidth(2.0f);
	glPointSize(8.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	
	glEnable(GL_FRONT_FACE); // both sides of polygon
	
	performAction( PERFORM_ACTION_DEBUG, PERFORM_ACTION_TRUE );
	performAction( PERFORM_ACTION_DEFINE_MESH_TYPE, PERFORM_ACTION_FALSE );
	
	LoadFilename( "egypt50-100.surf", 0 ); // meshFilename
	LoadFilename( "trig_test_50-100.ppm", 1 ); // textureFilename
	LoadFilename( "test.ssf", 3 ); // scriptFilename
	
	Init();

	performAction( PERFORM_ACTION_ADJUST_COEF_REST, 100 );
	performAction( PERFORM_ACTION_ADJUST_DAMPING, 40 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_X, 0 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_Y, -200 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_Z, 0 );
	performAction( PERFORM_ACTION_ADJUST_SPRING_CONSTANT, 2500 );
	performAction( PERFORM_ACTION_ADJUST_SPRING_DAMPING, 100 );
	performAction( PERFORM_ACTION_ADJUST_USER_FORCE_MAG, 100000 );
	performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, PERFORM_ACTION_TRUE );
}

void Display()
{
	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
	
	changeAngle();
	
	RenderScene();

	/*if( countdisplay)
	{
   int width = 500, height = 500;
	glViewport(0, 0, width, height); 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height );
	glMatrixModatade(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	char framecount[10];
	sprintf( framecount, "%d", GetCurrentFrameCount() );
	MBglDrawString( 10, 10, framecount );
	// Pop the matrices back on
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}*/

	glutSwapBuffers();
	glutPostRedisplay();
}

void Keyboard( unsigned char value, int x, int y )
{
	switch( value )
	{
		case 'r': case 'R': performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE ); break;
		case 'q': exit(0);
		case 'p': performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_TRUE ); break;
		
		case 'g': performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_TRUE ); break;
		case 'G': performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_FALSE ); break;
		
		case 'b': performAction( PERFORM_ACTION_ADJUST_GRAVITY_Y, 1000 ); break;
		case 'n': performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, PERFORM_ACTION_TRUE ); break;
		
		case 's': performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE ); screenshot = true; break;
	}
	glutPostRedisplay();
}

int main( int argc, char** argv )
{
   glutInit( &argc, argv );
   glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   int width = 500, height = 500;
   glutInitWindowSize( width, height );
   glutInitWindowPosition( 0, 0 );
   glutCreateWindow( "Scroll Manipulation Toolkit" );
   
   init();

   glutDisplayFunc( Display );
   glutReshapeFunc( ReshapeCanvas );
   glutKeyboardFunc( Keyboard );
  // glutIdleFunc( Idle );
   glutMainLoop();
   return 0;
}
