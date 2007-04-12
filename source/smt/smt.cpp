#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "smc.h"
#include "smtss.h"

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

bool countdisplay = true, screenshot = false, springs = false, vertices = false;
int framecount = 10000;

void init(char *meshfile, char *texturefile, char *scriptfile)
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
	
	LoadFilename( meshfile, 0 ); // meshFilename
	LoadFilename( texturefile, 1 ); // textureFilename
	LoadFilename( scriptfile, 3 ); // scriptFilename
	
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
	performAction( PERFORM_ACTION_DISPLAY_SPRINGS, PERFORM_ACTION_FALSE );
	performAction( PERFORM_ACTION_DISPLAY_VERTICES, PERFORM_ACTION_FALSE );
	performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE );
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
	
	if(screenshot) {
	  printf("Taking screenshot\n");
	  // Screenshot_JPEG("output.jpg",WINDOW_WIDTH,WINDOW_HEIGHT,90);
	  Screenshot_TR("output.ppm",2048,2048);
	  screenshot = false;
	  exit(0);
	}
	
	glutPostRedisplay();
}

void Keyboard( unsigned char value, int x, int y )
{
	switch( value )
	{
		case 'r': case 'R': performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE ); break;
		case 'q': exit(0);
		case 'p': printf("Running simulation\n");
				  performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_TRUE ); break;
		
		case 'g': printf("Using gravity\n");
				  performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_TRUE ); break;
		case 'G': performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_FALSE ); break;
		
		case 'b': printf("Adjusting gravity\n");
				  performAction( PERFORM_ACTION_ADJUST_GRAVITY_Y, -1000 ); break;
		case 'n': printf("Committed properties\n");
				  performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, PERFORM_ACTION_TRUE ); break;
		
		case 's': performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE ); screenshot = true; break;
		case '.':
				  printf("Toggling spring display\n");
				  performAction( PERFORM_ACTION_DISPLAY_SPRINGS, springs ? PERFORM_ACTION_FALSE : PERFORM_ACTION_TRUE );
				  springs = !springs;
				  break;
		case ',':
				  printf("Toggling vertex display\n");
				  performAction( PERFORM_ACTION_DISPLAY_VERTICES, vertices ? PERFORM_ACTION_FALSE : PERFORM_ACTION_TRUE );
				  vertices = !vertices;
				  break;
		case 'j':
				  printf("Taking screenshot\n");
				  Screenshot_JPEG("test.jpg",WINDOW_WIDTH,WINDOW_HEIGHT,90);
				  break;
	}
	glutPostRedisplay();
}

int main( int argc, char** argv )
{
   glutInit( &argc, argv );
   glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
   glutInitWindowSize( width, height );
   glutInitWindowPosition( 0, 0 );
   glutCreateWindow( "Scroll Manipulation Toolkit" );
  
   if(argc == 4) {
	   init(argv[1],argv[2],argv[3]);
   }
   else { 
	init("data/ski.surf", "data/ski-1.ppm", "data/test.ssf");
   }

   glutDisplayFunc( Display );
   glutReshapeFunc( ReshapeCanvas );
   glutKeyboardFunc( Keyboard );
  // glutIdleFunc( Idle );
   glutMainLoop();
   return 0;
}
