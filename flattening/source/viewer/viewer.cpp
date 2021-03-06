#ifdef __APPLE__
#include "GLee.h" 
#include <GLUT/glut.h>
#else
#include "GLee.h"
#include <GL/glut.h>
#endif
#include <cstdio>
#include <cstdlib>
#include "smtss.h"

#include <glui.h>

#include <errno.h>
#include <dirent.h>
#include <deque>

#include <algorithm>

#include <iostream>
#include <string>
using namespace std;

#include "smc.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define DETACHED_CONTROLS 0
#define CACHE_BUTTON 1

float xy_aspect;
float view_rotate[16] = { 0.776922,-0.418051,0.470771,0, -0.038177,0.715077,0.698002,0, -0.628438,-0.560266,0.539599,0, 0,0,0,1 };
// float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float initial_obj_pos[] = { 1.48, 0.685, 8.964999 };
float obj_pos[] = { 0.0, 0.0, 0.0 };
int	  show_vertices = 0;
int	  show_springs = 0;
float flattening = 0;

GLUI *glui;
GLUI_Listbox *fileBox;
GLUI_EditText *searchBox;
GLUI_Translation *trans_z;
GLUI_Button *reloadButton, *prevButton, *nextButton, *cacheButton;

// User IDs for callbacks
enum { FLATTENING_ID = 300, FILE_SELECT_ID, SEARCH_ID, PREV_ID, NEXT_ID, ZOOM_ID, WRINKLING_ID, VERTICES_ID, SPRINGS_ID, QUIT_ID, CACHE_ID };

deque<string> fileNames;

int main_window;
bool screenshot = false;

extern GLint width, height;
float TotalTime = 0.0f;
float time_limit = 0;

void ToggleVerticesAndSprings( void )
{
	performAction( PERFORM_ACTION_DISPLAY_VERTICES, show_vertices ? PERFORM_ACTION_TRUE : PERFORM_ACTION_FALSE );
	performAction( PERFORM_ACTION_DISPLAY_SPRINGS, show_springs ? PERFORM_ACTION_TRUE : PERFORM_ACTION_FALSE );
}

void init(char *meshfile, char *texturefile)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	//glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);

	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	
	glPolygonMode(GL_FRONT,GL_FILL);
	glLineWidth(2.0f);
	glPointSize(8.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	// performAction( PERFORM_ACTION_DEBUG, PERFORM_ACTION_TRUE );
	performAction( PERFORM_ACTION_DEFINE_MESH_TYPE, PERFORM_ACTION_FALSE );
	
	LoadFilename( meshfile, 0 ); // meshFilename
	LoadFilename( texturefile, 1 ); // textureFilename
	// LoadFilename( scriptfile, 3 ); // scriptFilename
	
	Init();

	performAction( PERFORM_ACTION_ADJUST_COEF_REST, 100 );
	performAction( PERFORM_ACTION_ADJUST_DAMPING, 40 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_X, 0 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_Y, -200 );
	performAction( PERFORM_ACTION_ADJUST_GRAVITY_Z, 0 );
	performAction( PERFORM_ACTION_ADJUST_SPRING_CONSTANT, 2500 );
	performAction( PERFORM_ACTION_ADJUST_SPRING_DAMPING, 100 );
	performAction( PERFORM_ACTION_ADJUST_USER_FORCE_MAG, 100000 );

	performAction( PERFORM_ACTION_SET_INTEGRATOR_TYPE, EULER_INTEGRATOR); 

	performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, PERFORM_ACTION_TRUE );
	ToggleVerticesAndSprings();
}

void update_once(void)
{
	view_rotate[0] = 0.776922;
	view_rotate[1] = -0.418051;
	view_rotate[2] = 0.470771;
	view_rotate[3] = 0;
	view_rotate[4] = -0.038177;
	view_rotate[5] = 0.715077;
	view_rotate[6] = 0.698002;
	view_rotate[7] = 0;
	view_rotate[8] =  -0.628438;
	view_rotate[9] = -0.560266;
	view_rotate[10] = 0.539599;
	view_rotate[11] = 0;
	view_rotate[12] = 0;
	view_rotate[13] = 0;
	view_rotate[14] = 0;
	view_rotate[15] = 1;

	/*
	obj_pos[0] = 1.48;
	obj_pos[1] = 0.685;
 	obj_pos[2] = 8.964999;
	*/
	
	glutPostRedisplay();
}

int frame=0;
int mytime=0,mytimebase=0;

void MyDisplay()
{
	/*
	frame++;
	mytime=glutGet(GLUT_ELAPSED_TIME);
	if (mytime - mytimebase > 1000) {
		printf("FPS:%4.2f\n",
			frame*1000.0/(mytime-mytimebase));
		mytimebase = mytime;		
		frame = 0;
	}
	*/
	
	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
  /*	
  glClear( GL_COLOR_BUFFER_BIT );
	glEnable (GL_BLEND);
  glEnable (GL_POLYGON_SMOOTH);
  glEnable (GL_LINE_SMOOTH);
  glEnable (GL_POINT_SMOOTH);

  glDisable (GL_DEPTH_TEST);
  */
	glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 1500.0 );
	// gluPerspective( 60.0, xy_aspect, 1.0, 2000.0 );

  glMatrixMode( GL_MODELVIEW );

  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -2.6f );
  glTranslatef( initial_obj_pos[0], initial_obj_pos[1], -initial_obj_pos[2] ); 
  glTranslatef( obj_pos[0], obj_pos[1], -obj_pos[2] ); 
  glMultMatrixf( view_rotate );

	glPushMatrix();

	RenderScene();

	glPopMatrix();

	if(screenshot) {
		screenshot = false;
		// glui->enable();
	}
	
	glutSwapBuffers();
}

void getFileNames()
{
	DIR *pdir;
	struct dirent *pent;

	GLint texsize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texsize);

	pdir = opendir("venetus");
	if(!pdir)	{
		fprintf(stderr,"Failed to open directory. Terminating.\n");
		exit(1);
	}
	errno = 0;
	while(pent=readdir(pdir)) {
		string file = pent->d_name;
		if(file[0] != '.') {
			if((file.compare(file.length()-5,5,".surf") == 0) || (file.compare(file.length()-4,4,".obj") == 0)) {
				if((file.compare(file.length()-8,3,"-lo") == 0) || (file.compare(file.length()-7,3,"-lo") == 0)) {
					fileNames.push_front(file);
				}
				else {
					fileNames.push_back(file);
				}
			}
		}
	}
	if(errno) {
		fprintf(stderr,"opendir() failed.  Terminating.\n");
		exit(1);
	}
	closedir(pdir);
	sort(fileNames.begin(),fileNames.end());
}

void InitFromFileNames(int pos) {
	string selected_file, corresponding_image;
	string directory = "venetus/";

	selected_file = fileNames[pos];
	corresponding_image = selected_file;
	if(selected_file.compare(selected_file.length()-5,5,".surf") == 0) {
		corresponding_image.replace(corresponding_image.end()-5,corresponding_image.end(),".jpg");
	}
	else {
		corresponding_image.replace(corresponding_image.end()-4,corresponding_image.end(),".jpg");
	}
	// Look for jpg first, then try png
	if(access((directory+corresponding_image).c_str(), R_OK) != 0) {
		corresponding_image.replace(corresponding_image.end()-4,corresponding_image.end(),".png");
	}
	// printf( "%s\n", selected_file.c_str() );
	// printf( "%s\n", corresponding_image.c_str() );
	init((char *)(directory + selected_file).c_str(), (char *)(directory + corresponding_image).c_str());
}

void DisableGLUIandInit(void)
{
	glui->disable();
	InitFromFileNames(fileBox->get_int_val());
	glui->enable();
	reloadButton->disable();
	if(fileBox->get_int_val() == 0) {
		prevButton->disable();
	}
	if((fileBox->get_int_val() + 1) ==  fileNames.size()) {
		nextButton->disable();
	}
}

void cacheAllFiles(void)
{
	for(unsigned int i = 0; i < fileNames.size(); i++) {
		string selected_file = fileNames[i];
		string corresponding_image = selected_file;
		if(selected_file.compare(selected_file.length()-5,5,".surf") == 0) {
			corresponding_image.replace(corresponding_image.end()-5,corresponding_image.end(),".jpg-0-0-0.cmp");
		}
		else {
			corresponding_image.replace(corresponding_image.end()-4,corresponding_image.end(),".jpg-0-0-0.cmp");
		}
		corresponding_image = "venetus/cache/" + corresponding_image;
	 	printf("Checking for %s: ", corresponding_image.c_str());	
		if(access(corresponding_image.c_str(),R_OK) != 0) {
			printf("not found, generating\n");
			fileBox->set_int_val(i);
			DeleteSystem();
			DisableGLUIandInit();
			MyDisplay();
		}
		else {
			printf("found, skipping\n");
		}
	}
}

void control_cb(int control)
{
	switch( control )
	{
		case VERTICES_ID:
			ToggleVerticesAndSprings();
			break;
		case SPRINGS_ID:
			ToggleVerticesAndSprings();
			break;
		case SEARCH_ID:
			if(strcmp(searchBox->get_text(), "") != 0) {
				for(unsigned int i = 0; i < fileNames.size(); i++) {
					if(fileNames[i].find(string(searchBox->get_text()),0) != string::npos) {
						searchBox->set_text( "" );
						fileBox->set_int_val(i);
						printf("Found %d\n",i);
						DeleteSystem();
						DisableGLUIandInit();
						break;
					}
				}
			}
			break;
		case PREV_ID:
			nextButton->enable();
			fileBox->set_int_val(fileBox->get_int_val()-1);
			
			DeleteSystem();
			DisableGLUIandInit();
			break;
		case NEXT_ID:
			prevButton->enable();
			fileBox->set_int_val(fileBox->get_int_val()+1);
		
			DeleteSystem();
			DisableGLUIandInit();
			break;
		case CACHE_ID:
			cacheAllFiles();
			break;
		case FLATTENING_ID:
			reloadButton->enable();
			// glui->disable();
			performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_TRUE ); 
			break;
		case FILE_SELECT_ID:
			DeleteSystem();
			DisableGLUIandInit();
			break;
		case ZOOM_ID:
			// do something here where we can zoom in very slowly up close and very quickly far away
  		// printf("%f\n",obj_pos[2]+initial_obj_pos[2]+1.635);
			// trans_z->set_speed( );
			break;
		case WRINKLING_ID:
			NewSystem();
			DisableGLUIandInit();
			break;
		case QUIT_ID:
			glui->close();
			performAction( PERFORM_ACTION_QUIT, PERFORM_ACTION_TRUE );
			exit(0);	
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

void MyKeyboard( unsigned char value, int x, int y )
{
	char inputval[] = "0";

	// printf("MyKeyboard\n");
	if(strcmp(searchBox->get_text(),"") == 0) {
		// printf("Empty, setting %c\n",value);
		glui->activate_control( searchBox, GLUI_ACTIVATE_MOUSE );
		inputval[0] = value;
		searchBox->set_text(inputval);
		searchBox->insertion_pt = 1;
	}
	else if(GLUI_Master.active_control == searchBox) {
		// printf("Passing %c\n",value);
		glui_keyboard_func(value,x,y);
	}
}

void MyReshapeCanvas( int width, int height )
{
	int tx, ty, tw, th;
	if( DETACHED_CONTROLS /*|| glexExtensionsSupported("GL_CR_state_parameter") */) {
		xy_aspect = (float)width / (float)height;
	}
	else {
		GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
		glViewport( tx, ty, tw, th );
		xy_aspect = (float)tw / (float)th;
	}

	ReshapeCanvas(width,height);

	glutPostRedisplay();
}

void MyGlutIdle( void )
{
	if(glutGetWindow() != main_window) {
		glutSetWindow(main_window);
	}

	glutPostRedisplay();
}

void setup_glui(void)
{
	if( DETACHED_CONTROLS /*|| glexExtensionsSupported("GL_CR_state_parameter")*/ ) {
		glui = GLUI_Master.create_glui( "Controls" );
		glutReshapeFunc( MyReshapeCanvas );
	}
	else {	
		GLUI_Master.set_glutReshapeFunc(MyReshapeCanvas);
		/*** Create the side subwindow ***/
		glui = GLUI_Master.create_glui_subwindow( main_window, 
								GLUI_SUBWINDOW_LEFT );
	}
	
  //******Add list box containing image file names**********
  fileBox = glui->add_listbox( "File: ", NULL, FILE_SELECT_ID, control_cb);
  getFileNames();
  for(unsigned int i=0; i < fileNames.size(); i++)
  {
		fileBox->add_item( i, (char*)fileNames[i].c_str() );
  }
	// fileBox->add_item(0, "default"); 

	searchBox = glui->add_edittext( "Search: ", GLUI_EDITTEXT_TEXT, NULL, SEARCH_ID, control_cb);
	searchBox->set_w(175);
  
	glui->add_statictext( "" );

  prevButton = glui->add_button( "Previous" , PREV_ID, control_cb);
	prevButton->disable();
 
  nextButton = glui->add_button( "Next" , NEXT_ID, control_cb);

	if(CACHE_BUTTON) {
		glui->add_button( "Cache All", CACHE_ID, control_cb);
	}

  glui->add_statictext( "" );
	
	//*****Control spheres for rotation, translation, and zoom controls********
 
  GLUI_Rotation *view_rot = glui->add_rotation( "Rotate", view_rotate );
  view_rot->set_spin( 1.0 );

  GLUI_Translation *trans_xy = 
    glui->add_translation( "Move", GLUI_TRANSLATION_XY, obj_pos );
  trans_xy->set_speed( .01 );

	// obj_pos[2] = 8.964999;
  trans_z = 
    glui->add_translation( "Zoom", GLUI_TRANSLATION_Z, &obj_pos[2], ZOOM_ID, control_cb );
  trans_z->set_speed( 0.1 );

  glui->add_statictext( "" );
 
  

 //*********Panel for showing the flat pane and for showing flattening******
  GLUI_Panel *actions_panel = glui->add_panel("Actions");

  glui->add_checkbox_to_panel( actions_panel, "Show Vertices", &show_vertices, VERTICES_ID, control_cb ); 
  glui->add_checkbox_to_panel( actions_panel, "Show Springs", &show_springs, SPRINGS_ID, control_cb ); 

	/*
  GLUI_Spinner *flatteningSpinner = 
    glui->add_spinner_to_panel( actions_panel, "Flattening", GLUI_SPINNER_FLOAT,
				&flattening, FLATTENING_ID,	control_cb );
  flatteningSpinner->set_float_limits( 0.0, 100.0 );
  flatteningSpinner->set_speed( .05 );
	*/

  glui->add_button_to_panel( actions_panel, "Flatten" , FLATTENING_ID, control_cb);

  reloadButton = glui->add_button_to_panel( actions_panel, "Reload" , WRINKLING_ID, control_cb);
	reloadButton->disable();

  glui->add_statictext( "" );
  

   /****** A 'quit' button *****/
  glui->add_button( "Quit", QUIT_ID, control_cb );


  /**** Link windows to GLUI, and register idle callback ******/
  
  glui->set_main_gfx_window( main_window );

  /**** We register the idle callback with GLUI, *not* with GLUT ****/
  GLUI_Master.set_glutIdleFunc( MyGlutIdle );
}

int main( int argc, char** argv )
{
	string mesh_file, image_file, script_file, output_geometry;
	
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
	glutInitWindowSize( width, height );
	glutInitWindowPosition( 0, 0 );
	
	glutInit( &argc, argv );

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | /*GLUT_ALPHA |*/ GLUT_DEPTH );
	main_window = glutCreateWindow( "Venetus A Viewer" );
	/*
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		fprintf(stderr,"Error: %s\n", glewGetErrorString(err));
		exit(1);
	}
	*/

	glutDisplayFunc( MyDisplay );
	glutKeyboardFunc( MyKeyboard );

	setup_glui();
	
	InitFromFileNames(0);

	update_once();

	// glutIdleFunc( Idle );
	glutMainLoop();
	
	return 0;
}
