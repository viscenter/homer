#ifndef Scroll_Manipulation_Toolkit_Graphical_User_Interface_Core
#define Scroll_Manipulation_Toolkit_Graphical_User_Interface_Core

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Skeleton.h"
#include "PhysEnv.h"
#include "manuModel.h"

// Basic Information
#define PERFORM_ACTION_FALSE                           0
#define PERFORM_ACTION_TRUE                            1

// Load Information
#define PERFORM_ACTION_DEFINE_MESH_TYPE               10
#define PERFORM_ACTION_DEBUG                          20

// Other Actions
#define PERFORM_ACTION_RECORD_SCRIPT_FILE            100
#define PERFORM_ACTION_PLAY_SCRIPT_FILE              101
#define PERFORM_ACTION_LOCK_POINT                    120

// Object Manipulation
#define PERFORM_ACTION_ROTATE                        200
#define PERFORM_ACTION_TRANSLATE_X                   201
#define PERFORM_ACTION_TRANSLATE_Y                   202
#define PERFORM_ACTION_TRANSLATE_Z                   203

// Simulation Properties
#define PERFORM_ACTION_ADJUST_COEF_REST              300
#define PERFORM_ACTION_ADJUST_DAMPING                301
#define PERFORM_ACTION_ADJUST_GRAVITY_X              302
#define PERFORM_ACTION_ADJUST_GRAVITY_Y              303
#define PERFORM_ACTION_ADJUST_GRAVITY_Z              304
#define PERFORM_ACTION_ADJUST_SPRING_CONSTANT        305
#define PERFORM_ACTION_ADJUST_SPRING_DAMPING         306
#define PERFORM_ACTION_ADJUST_USER_FORCE_MAG         307
#define PERFORM_ACTION_COMMIT_SIM_PROPERTIES         308

// Display Actions
#define PERFORM_ACTION_SET_INTEGRATOR_TYPE           500
#define PERFORM_ACTION_RESET_WORLD                   501
#define PERFORM_ACTION_SET_RUNNING                   502
#define PERFORM_ACTION_LOCK_PARTICLES                503
#define PERFORM_ACTION_CLEAR_PARTICLE_LOCK           504
#define PERFORM_ACTION_SET_COLLISION_ACTIVE          505
#define PERFORM_ACTION_SET_USE_GRAVITY               506
#define PERFORM_ACTION_SET_USE_DAMPING               507
#define PERFORM_ACTION_DISPLAY_SPRINGS               508
#define PERFORM_ACTION_DISPLAY_VERTICES              509
#define PERFORM_ACTION_ADJUST_LOCK_X_AXIS            510
#define PERFORM_ACTION_ADJUST_LOCK_Y_AXIS            511
#define PERFORM_ACTION_ADJUST_LOCK_Z_AXIS            512

// Camera Actions
#define PERFORM_ACTION_RESET_CAMERA                  700
#define PERFORM_ACTION_ADJUST_AXIS_X                 701
#define PERFORM_ACTION_ADJUST_AXIS_Y                 702
#define PERFORM_ACTION_ADJUST_AXIS_Z                 703
#define PERFORM_ACTION_ADJUST_DISTANCE               704
#define PERFORM_ACTION_ROTATE_ANGLE_X                705
#define PERFORM_ACTION_ROTATE_ANGLE_Y                706
#define PERFORM_ACTION_ADJUST_FIELD_OF_VIEW          707

// Quit Action
#define PERFORM_ACTION_SAVE_MESH_FILE                900
#define PERFORM_ACTION_QUIT                          999


#define PI 3.141592653


int GetCurrentFrameCount();
int Init();
void CreateSelectionArray();
void ClearSelectionArray();
void LoadLockPoints();
int GetLockedArray( int *intLockedArray ); // If called need to free intLockedArray
void AddToSelectionArray( int number );
void changeAngle();
void ReshapeCanvas( int inWidth, int inHeight );
void selectionBoxWindow();
void RenderScene();
int ScriptIsDone();
void PickPoint( int x, int y );
void LockPoint( int x, int y );
void boxPick();
void DragPoint( int x, int y );
void MouseRelease();
void NewSystem();
void DeleteSystem();
void RunSim();
void LoadFile( char file1[] );
int CreateClothPatch( char *meshfile, char *texturefile );
int SaveMeshFile();
void LoadFilename( char inFilename[], int which );
void performAction( int action, int value );

// Part of original windows version - source?
void MBglDrawString( int x, int y, char *string );

#endif
