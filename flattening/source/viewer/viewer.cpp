#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include <cstdio>
#include <cstdlib>
#include "smtss.h"

#include <boost/program_options.hpp>

#include <glui.h>

namespace po = boost::program_options;

#include <iostream>
#include <string>
using namespace std;

#include "smc.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

float xy_aspect;
float view_rotate[16] = { 0.776922,-0.418051,0.470771,0, -0.038177,0.715077,0.698002,0, -0.628438,-0.560266,0.539599,0, 0,0,0,1 };
// float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
float obj_pos[] = { 1.48, 0.685, 8.964999 };
// float obj_pos[] = { 0.0, 0.0, 0.0 };
int	  show_plane = 0;
float flattening = 0;

// User IDs for callbacks
enum { FLATTENING_ID = 300, FILE_SELECT_ID, WRINKELING_ID };
/*
#define FLATTENING_ID 305
#define FILE_SELECT_ID 307
#define WRINKELING_ID		 306
*/

int main_window;

bool countdisplay = true, screenshot = false, springs = false, vertices = false;

bool auto_start = true, auto_quit = true;

string output_filename, integrator;
int output_width, output_height;

extern GLint width, height;

int mouse_state;
int mouseX = 0, mouseY = 0;

float TotalTime = 0.0f;
float time_limit;

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

	if(integrator.compare("rk4") == 0) {
		performAction( PERFORM_ACTION_SET_INTEGRATOR_TYPE, RK4_INTEGRATOR); 
	}
	else if(integrator.compare("euler") == 0) {
		performAction( PERFORM_ACTION_SET_INTEGRATOR_TYPE, EULER_INTEGRATOR); 
	}
	else if(integrator.compare("midpoint") == 0) {
		performAction( PERFORM_ACTION_SET_INTEGRATOR_TYPE, MIDPOINT_INTEGRATOR); 
	}
	else {
		printf("Invalid integrator type specified.\n");
		exit(1);
	}

	performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, PERFORM_ACTION_TRUE );
	performAction( PERFORM_ACTION_DISPLAY_SPRINGS, PERFORM_ACTION_FALSE );
	performAction( PERFORM_ACTION_DISPLAY_VERTICES, PERFORM_ACTION_FALSE );
	if(auto_start) {
		performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE );
	}

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
	
	obj_pos[0] = 1.48;
	obj_pos[1] = 0.685;
 	obj_pos[2] = 8.964999;

	glutPostRedisplay();
}

void rotate( int deltaX, int deltaY )
{
	static int xAxis = 0, yAxis = 0;

	if(deltaX != 0) {
		xAxis += deltaX;
		performAction( PERFORM_ACTION_ROTATE_ANGLE_X, xAxis );
	}
	if(deltaY != 0) {
		yAxis += deltaY;
		performAction( PERFORM_ACTION_ROTATE_ANGLE_Y, yAxis );
	}
}

void zoom( int deltaY )
{
	static int zoomfactor = 0;

	if(deltaY != 0) {
		zoomfactor += deltaY;
		performAction( PERFORM_ACTION_ADJUST_DISTANCE, zoomfactor );
	}
}

void MouseHandler( int button, int state, int x, int y )
{
	if(state == GLUT_DOWN) {
		mouse_state = button;
	}
	else {
		mouse_state = 0;
	}

	mouseX = x;
	mouseY = y;
}

void MotionHandler( int x, int y )
{
	switch(mouse_state) {
		case GLUT_LEFT_BUTTON:
			rotate(mouseX - x, mouseY - y);
			break;
		case GLUT_RIGHT_BUTTON:
			zoom(mouseY - y);
			break;
		case GLUT_MIDDLE_BUTTON:
			rotate(mouseX - x, 0);
			break;
		default:
			break;
	}

	mouseX = x;
	mouseY = y;

	glutPostRedisplay();
}

void Toggle_Mouse(bool enable) {
	if(enable) {
		glutMouseFunc( MouseHandler );
		glutMotionFunc( MotionHandler );
	}
	else {
		glutMouseFunc( NULL );
		glutMotionFunc( NULL );
	}
}

void Take_Screenshot()
{
	printf("Taking screenshot\n");
		
	Screenshot_TR((char *)output_filename.c_str(),output_width,output_height);
	performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_FALSE );
		
	if(auto_quit) {
		exit(0);
	}
	else {
		Toggle_Mouse(true);		
	}
}

void Display()
{
	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 15.0 );

  glMatrixMode( GL_MODELVIEW );

  glLoadIdentity();
  glTranslatef( 0.0, 0.0, -2.6f );
  glTranslatef( obj_pos[0], obj_pos[1], -obj_pos[2] ); 
  glMultMatrixf( view_rotate );

	glPushMatrix();

	RenderScene();

	glPopMatrix();
	
	glutSwapBuffers();
	
	if(time_limit && screenshot) {
		if(TotalTime > time_limit) {
			printf("Time limit reached\n");
			Take_Screenshot();
			screenshot = false;	
		}
	}
	else if(screenshot) {
		printf("Flattening stable\n");
		Take_Screenshot();	
		screenshot = false;
	}
	
	glutPostRedisplay();
}

void Print_Controls()
{
	cout << "\nKeyboard shortcuts:\n" <<
			"\tq\texit\n" <<
			"\tr,R\trun script\n" <<
			"\tp\trun simulation\n" <<
			"\te,o\tpause simulation\n" <<
			"\ts\ttake screenshot\n" <<
			"\tt\tprint current simulation time\n" <<
			"\t.\ttoggle spring display\n" <<
			"\t,\ttoggle vertex display\n" <<
			"\t?\tprint this message\n" <<
			"\nMouse controls:\n" <<
			"\tleft-click\tfree-rotate around document\n" <<
			"\tright-click\tchange distance\n" <<
			"\tmiddle-click\trotate around x-axis\n\n";
}

void print_camera(void)
{
	for(int i = 0; i < 16; i++) {
		printf("%f\t",view_rotate[i]);
	}
	printf("\n");
	for(int i = 0; i < 3; i++) {
		printf("%f\t",obj_pos[i]);
	}
	printf("\n");
}

void Keyboard( unsigned char value, int x, int y )
{
	switch( value )
	{
		case 'r': case 'R': 
			performAction( PERFORM_ACTION_PLAY_SCRIPT_FILE, PERFORM_ACTION_TRUE );
			Toggle_Mouse(false);
			break;
		case 't':
			cout << "Current time: " << TotalTime << endl;
			break;
		case 'q': 
			printf("Quitting...\n");
			exit(0);
		case 'p':
			printf("Running simulation\n");
			performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_TRUE ); 
			break;
		case 'o':
		case 'e':
			printf("Pausing simulation\n");
			performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_FALSE ); 
			Toggle_Mouse(true);
			break;
		case 'g':
			printf("Enabling gravity\n");
			performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_TRUE );
			break;
		case 'G':
			printf("Disabling gravity\n");
			performAction( PERFORM_ACTION_SET_USE_GRAVITY, PERFORM_ACTION_FALSE );
			break;
		case 's':
			Take_Screenshot();
			break;
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
		case 'z':
			print_camera();
			break;
		case '?':
			Print_Controls();
			break;
	}
	glutPostRedisplay();
}

void control_cb(int control)
{
}

void MyReshapeCanvas( int width, int height )
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

	ReshapeCanvas(width,height);

	xy_aspect = (float)tw / (float)th; 
}

void setup_glui(void)
{
	GLUI_Master.set_glutReshapeFunc(MyReshapeCanvas);
  /*** Create the side subwindow ***/
  GLUI *glui = GLUI_Master.create_glui_subwindow( main_window, 
					    GLUI_SUBWINDOW_LEFT );


   //******Add list box containing image file names**********
  GLUI_Listbox *fileBox = glui->add_listbox( "File: ", NULL, FILE_SELECT_ID, control_cb);
  //getFileNames();
  // for(int i=0; i<MAX_FILES && fileNames[i] != ""; i++)
  //{
	//fileBox->add_item( i, (char*)fileNames[i].c_str() );
  //}
	fileBox->add_item(0, "default"); 
  glui->add_statictext( "" );


  //*****Control spheres for rotation, translation, and zoom controls********
 
  GLUI_Rotation *view_rot = glui->add_rotation( "Rotate", view_rotate );
  view_rot->set_spin( 1.0 );

  GLUI_Translation *trans_xy = 
    glui->add_translation( "Translate", GLUI_TRANSLATION_XY, obj_pos );
  trans_xy->set_speed( .005 );

  GLUI_Translation *trans_z = 
    glui->add_translation( "Zoom", GLUI_TRANSLATION_Z, &obj_pos[2] );
  trans_z->set_speed( .005 );

  glui->add_statictext( "" );
 
  

 //*********Panel for showing the flat pane and for showing flattening******
  GLUI_Panel *actions_panel = glui->add_panel("Actions");

  glui->add_checkbox_to_panel( actions_panel, "Show Plane", &show_plane ); 

  GLUI_Spinner *flatteningSpinner = 
    glui->add_spinner_to_panel( actions_panel, "Flattening", GLUI_SPINNER_FLOAT,
				&flattening, FLATTENING_ID,	control_cb );
  flatteningSpinner->set_float_limits( 0.0, 100.0 );
  flatteningSpinner->set_speed( .05 );

  glui->add_button_to_panel( actions_panel, "Flatten" , FLATTENING_ID, control_cb);

  glui->add_button_to_panel( actions_panel, "Wrinkle" , WRINKELING_ID, control_cb);

  glui->add_statictext( "" );
  

   /****** A 'quit' button *****/
  glui->add_button( "Quit", 0,(GLUI_Update_CB)exit );


  /**** Link windows to GLUI, and register idle callback ******/
  
  glui->set_main_gfx_window( main_window );

  /**** We register the idle callback with GLUI, *not* with GLUT ****/
  // GLUI_Master.set_glutIdleFunc( myGlutIdle );
}

int main( int argc, char** argv )
{
	string mesh_file, image_file, script_file, output_geometry;
	
	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
	glutInitWindowSize( width, height );
	glutInitWindowPosition( 0, 0 );
	
	glutInit( &argc, argv );
	
	po::options_description generic("Program options");
	generic.add_options()
		("help", "produce help message")
		("output,o",
		 	po::value<string>(&output_filename)->default_value("output.png"),
			"output image file")
		("output-geometry,g",
		 	po::value<string>(&output_geometry)->default_value("2048x2048"),
			"output image file geometry in WxH format")
		("integrator,i",
		 	po::value<string>(&integrator)->default_value("euler"),
			"point integrator (valid values are \"euler\", \"rk4\" "
			"and \"midpoint\")")
		("time-limit,t",
		 	po::value<float>(&time_limit)->default_value(0),
			"restrict the total simulation time (0=disabled)")
		("no-auto-start,s",
			"don't automatically play script file")
		("no-auto-quit,q",
			"don't automatically quit after screenshot")
		;

	po::options_description hidden("Hidden options");
	hidden.add_options()
		("mesh-file,M",
		 	po::value<string>(&mesh_file)->default_value("data/ski.surf"),
			"input mesh file")
		("image-file,I",
		 	po::value<string>(&image_file)->default_value("data/ski-1.ppm"),
			"input image file")
		("script-file,S",
		 	po::value<string>(&script_file)->default_value("data/test.ssf"),
			"input script file")
		;
	
	po::options_description cmdline_options;
	cmdline_options.add(generic).add(hidden);
	
	po::positional_options_description pd;
	pd.add("mesh-file",1);
	pd.add("image-file",1);
	pd.add("script-file",1);
	
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
			options(cmdline_options).positional(pd).run(), vm);
	po::notify(vm);
	
	if(vm.count("help")) {
		cout << "Usage:\n";
		cout << "\t" << argv[0] << " [mesh file] [image file] [script file]\n";
		cout << generic << "\n";
		return 1;
	}

	if(vm.count("no-auto-start")) {
		auto_start = false;
	}
	if(vm.count("no-auto-quit")) {
		auto_quit = false;
	}

	if(time_limit) {
		screenshot = true;
	}

	string::size_type x_position = output_geometry.find("x");
	output_width = atoi(output_geometry.substr(0,(int)x_position).c_str());
	output_height = atoi(output_geometry.substr((int)x_position + 1).c_str());

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	main_window = glutCreateWindow( "Venetus A Viewer" );

	setup_glui();
	
	init((char *)mesh_file.c_str(),	(char *)image_file.c_str(), (char *)script_file.c_str());

	glutDisplayFunc( Display );
	glutReshapeFunc( MyReshapeCanvas );
	glutKeyboardFunc( Keyboard );
	if(!auto_start) {
		Toggle_Mouse(true);
	}

	update_once();

	Print_Controls();
	
	// glutIdleFunc( Idle );
	glutMainLoop();
	
	return 0;
}
