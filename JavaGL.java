/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                Last Modified - February 20th, 2004                               *
*                                                                                                  *
*    Filename: "JavaGL.java"                                                                       *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This function is the OpenGL window.  All communication between the c side and        *
*          the java side occur in this calls.  The function performAction( int, int ) handles      *
*          must of the communication to the c side - using the PERFORM_ACTION_ variables to        *
*          tell the c side what function to perform.                                               *
*             The class uses gl4Java.  Doing so causes several problems.  Iconifing, hiding,       *
*          and sometimes reshaping the GLAnimCanvas can cause early termination.  Deleting the     *
*          GLAnimCanvas without KNOWING that the c side has stopped can cause early                *
*          termination of java.                                                                    *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       static int PERFORM_ACTION_...                                                              *
*          - used to communicate with the c side via performAction().                              *
*       smt:                                                                                       *
*          handle - handle to the main java class.                                                 *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       int:                                                                                       *
*          canvasHeight - height of OpenGL window.                                                 *
*          canvasWidth - width of OpenGL window.                                                   *
*          mouseX - previous mouse X coordinate for calculation of deltaX.                         *
*          mouseY - previous mouse Y coordinate for calculation of deltaY.                         *
*       boolean:                                                                                   *
*          cameraPanelLoaded - has the cameraPanel been loaded.                                    *
*          clickedOnce - Simple fix to mousePressed() being executed twice per one press.          *
*          debug - Is output debug infomation requested.                                           *
*          loaded - Is the JavaGL instance loaded.                                                 *
*          renderOutputOn - option used to render frames to a image folder.                        *
*          takeScreenshot - Is a screenshot requested.                                             *
*       CameraPanel:                                                                               *
*          cameraPanel - handle to the Camera Panel. (needed to sync camera with mouse)            *
*       Screenshot:                                                                                *
*          screenshot - used to output a screenshot in jpeg format.                                *
*       String:                                                                                    *
*          screenshotName - needed to take a screenshot.                                           *
*          renderName - needed to output series of screenshots.                                    *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       JavaGL( smt, int, int )                                                                    *
*          - Constructs a OpenGL window of the inputted size.                                      *
*                                                                                                  *
*    PUBLIC NATIVE FUNTIONS:                                                                       *
*       int[] getLockedArray()                                                                     *
*          - Returns a list containing the locked points.                                          *
*       void loadFilename( String, int )                                                           *
*          - Loads a filename String into the c side based on the int.                             *
*       void performAction( int, int )                                                             *
*          - Peforms a c side action based on PERFORM_ACTION_ and int value.                       *
*                                                                                                  *
*    PRIVATE NATIVE FUNTIONS:                                                                      *
*       int nativeInit()                                                                           *
*          - Non-gl part of init().                                                                *
*       void reshapeCanvas( int, int )                                                             *
*          - Update c side window size info when OpenGL window changes size.                       *
*       void renderScene()                                                                         *
*          - Displays the canvas.                                                                  *
*       pickPoint( int, int )                                                                      *
*          - pick a point in the OpenGL window.                                                    *
*       lockPoint( int, int )                                                                      *
*          - pick a point in the OpenGL window.                                                    *
*       dragPoint( int, int )                                                                      *
*          - lock a point in the OpenGL window.                                                    *
*       void mouseRelease()                                                                        *
*          - Tell the c side that the mouse button has been released.                              *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       void SetDebug( boolean )                                                                   *
*          - Sets debug to boolean.                                                                *
*       void setCamera( CameraPanel )                                                              *
*          - Sets the handle to the camera panel.                                                  *
*       boolean Debug()                                                                            *
*          - Returns debug().                                                                      *
*       int getWidth()                                                                             *
*          - Returns canvasWidth.                                                                  *
*       int getHeight()                                                                            *
*          - Returns canvasHeight.                                                                 *
*       boolean loadStatus()                                                                       *
*          - Returns loaded.                                                                       *
*       void preInit()                                                                             *
*          - Overrides GLAnimCanvas' preInit().                                                    *
*       void init()                                                                                *
*          - Overrides GLAnimCanvas' init().  Calls nativeInit().                                  *
*       void destroy()                                                                             *
*          - Sets loaded = false.                                                                  *
*       void quit()                                                                                *
*          - Tells c side to quit.  Disposes of OpenGL window.                                     *
*          - This function CANNOT RUN CONCURRENTLY WITH renderScene().                             *
*       void renderOutput( String, boolean )                                                       *
*          - Turns render output to boolean and renderName to String.                              *
*       void saveScreenshot()                                                                      *
*          - Sets takeScreenshot to true.                                                          *
*       void reshape( int, int )                                                                   *
*          - Overrides GLAnimCanvas' reshape().                                                    *
*       void display()                                                                             *
*          - Overrides GLAnimCanvas' display().                                                    *
*          - Calls renderScene() if loaded else calls quit().                                      *
*          - The quit() call HAS TO BE LOCATED in this function.                                   *
*       void mouseDragged( MouseEvent )                                                            *
*          - Handles mouse dragged event.                                                          *
*       void mouseMoved( MouseEvent )                                                              *
*          - VOID                                                                                  *
*       void mouseClicked( MouseEvent )                                                            *
*          - VOID                                                                                  *
*       void mouseEntered( MouseEvent )                                                            *
*          - VOID                                                                                  *
*       void mouseExited( MouseEvent )                                                             *
*          - VOID                                                                                  *
*       void mousePressed( MouseEvent )                                                            *
*          - Handles mouse pressed event.                                                          *
*       void mouseReleased( MouseEvent )                                                           *
*          - Calls mouseRelease().                                                                 *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       String IncreamentScreenshotFilename( String )                                              *
*          - Increaments the number part of a string (xxxx0000.jpg)                                *
*          - Returns the new name of the file.                                                     *
*                                                                                                  *
***************************************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import gl4java.GLContext;
import gl4java.awt.GLAnimCanvas;
import gl4java.utils.textures.TGATextureGrabber;

public class JavaGL extends GLAnimCanvas implements MouseListener, MouseMotionListener
{
	public static int PERFORM_ACTION_FALSE                  =   0;
	public static int PERFORM_ACTION_TRUE                   =   1;
	public static int PERFORM_ACTION_DEFINE_MESH_TYPE       =  10;
	public static int PERFORM_ACTION_DEBUG                  =  20;
        public static int PERFORM_ACTION_RECORD_SCRIPT_FILE     = 100;
        public static int PERFORM_ACTION_PLAY_SCRIPT_FILE       = 101;
        public static int PERFORM_ACTION_LOCK_POINT             = 120;
	public static int PERFORM_ACTION_ROTATE                 = 200;
	public static int PERFORM_ACTION_TRANSLATE_X            = 201;
	public static int PERFORM_ACTION_TRANSLATE_Y            = 202;
	public static int PERFORM_ACTION_TRANSLATE_Z            = 203;
	public static int PERFORM_ACTION_ADJUST_COEF_REST       = 300;
	public static int PERFORM_ACTION_ADJUST_DAMPING         = 301;
	public static int PERFORM_ACTION_ADJUST_GRAVITY_X       = 302;
	public static int PERFORM_ACTION_ADJUST_GRAVITY_Y       = 303;
	public static int PERFORM_ACTION_ADJUST_GRAVITY_Z       = 304;
	public static int PERFORM_ACTION_ADJUST_SPRING_CONSTANT = 305;
	public static int PERFORM_ACTION_ADJUST_SPRING_DAMPING  = 306;
	public static int PERFORM_ACTION_ADJUST_USER_FORCE_MAG  = 307;
	public static int PERFORM_ACTION_COMMIT_SIM_PROPERTIES  = 308;
	public static int PERFORM_ACTION_SET_INTEGRATOR_TYPE    = 500;
	public static int PERFORM_ACTION_RESET_WORLD            = 501;
	public static int PERFORM_ACTION_SET_RUNNING            = 502;
	public static int PERFORM_ACTION_LOCK_PARTICLES         = 503;
	public static int PERFORM_ACTION_CLEAR_PARTICLE_LOCK    = 504;
	public static int PERFORM_ACTION_SET_COLLISION_ACTIVE   = 505;
	public static int PERFORM_ACTION_SET_USE_GRAVITY        = 506;
	public static int PERFORM_ACTION_SET_USE_DAMPING        = 507;
	public static int PERFORM_ACTION_DISPLAY_SPRINGS        = 508;
	public static int PERFORM_ACTION_DISPLAY_VERTICES       = 509;
	public static int PERFORM_ACTION_ADJUST_LOCK_X_AXIS     = 510;
	public static int PERFORM_ACTION_ADJUST_LOCK_Y_AXIS     = 511;
	public static int PERFORM_ACTION_ADJUST_LOCK_Z_AXIS     = 512;
	public static int PERFORM_ACTION_RESET_CAMERA           = 700;
	public static int PERFORM_ACTION_ADJUST_AXIS_X          = 701;
	public static int PERFORM_ACTION_ADJUST_AXIS_Y          = 702;
	public static int PERFORM_ACTION_ADJUST_AXIS_Z          = 703;
	public static int PERFORM_ACTION_ADJUST_DISTANCE        = 704;
	public static int PERFORM_ACTION_ROTATE_ANGLE_X         = 705;
	public static int PERFORM_ACTION_ROTATE_ANGLE_Y         = 706;
	public static int PERFORM_ACTION_ADJUST_FIELD_OF_VIEW   = 707;
	public static int PERFORM_ACTION_SAVE_MESH_FILE         = 900;
	public static int PERFORM_ACTION_QUIT                   = 999;
	
        public smt handle;
	private int canvasWidth, canvasHeight, mouseX, mouseY;
	private CameraPanel cameraPanel;
	private boolean cameraPanelLoaded, clickedOnce, loaded, debug;
        private boolean renderOutputOn, takeScreenshot;
        private String screenshotName, renderName;
        
	public JavaGL( smt inHandle, int width, int height )
	{
		super( width, height );
		
                handle = inHandle;
		canvasWidth = width;
		canvasHeight = height;
		
		cameraPanelLoaded = false;
		clickedOnce = false;
		loaded = false;
		debug = false;
		takeScreenshot = false;
                renderOutputOn = false;
		
		addMouseListener( this );
		addMouseMotionListener( this );
	} // END OF - public JavaGL( smt, int, int )
	
	// Connect java side to c side
	static
	{
		String name = "\\JavaGL.dll";
		String os = System.getProperty( "os.name" );
		
		// If operating system is not windows - assume linux
		if( os.charAt(0) != 'W' && os.charAt(0) != 'w' )
		{
			name = "/JavaGL.so";
		}
		
// This can be changed to reflect the location of the shared library
		String pwd = System.getProperty( "user.dir" );
		System.load( pwd + name );
	} // END OF - static
	
	// OpenGL Native Functions
	private native int nativeInit();
	private native void reshapeCanvas( int width, int height );
	private native void renderScene();
	private native int scriptIsDone();
	// Native Functions for passing mouse information to the c side
	private native void pickPoint( int x, int y );
	private native void lockPoint( int x, int y );
	private native void dragPoint( int x, int y );
	private native void mouseRelease();
	// Native Functions for passing information to the c side.
        public native int[] getLockedArray();
	public native void loadFilename( String filename, int which );
	public native void performAction( int action, int value );
	
	public void SetDebug( boolean input )
	{
		debug = input;
		
		if( debug )
			performAction( JavaGL.PERFORM_ACTION_DEBUG, JavaGL.PERFORM_ACTION_TRUE );
		else
			performAction( JavaGL.PERFORM_ACTION_DEBUG, JavaGL.PERFORM_ACTION_FALSE );
	} // END OF - public void SetDebug( boolean )
	
	public void setCamera( CameraPanel handle )
	{
		cameraPanel = handle;
		cameraPanelLoaded = true;
	} // END OF - public void setCamera( CameraPanel )
	
	public boolean Debug()
	{
		return debug;
	} // END OF - public boolean Debug()
	
	public boolean loadStatus()
	{
		return loaded;
	} // END OF - public boolean loadStatus()
	
        public int getWidth()
	{
		return canvasWidth;
	} // END OF - public int getWidth()
	
	public int getHeight()
	{
		return canvasHeight;
	} // END OF - public int getHeight()
	
	public void preInit()
	{
		doubleBuffer = true;
		stereoView = false;
	} // END OF - public void preInit()
	
	public void init()
	{
		gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		gl.glShadeModel(GL_SMOOTH);
		
		gl.glClearDepth(1.0);
		gl.glDepthFunc(GL_LEQUAL);
		gl.glEnable(GL_DEPTH_TEST);
		
		gl.glPolygonMode(GL_FRONT,GL_FILL);
		gl.glLineWidth(2.0f);
		gl.glPointSize(8.0f);
		gl.glDisable(GL_CULL_FACE);
		gl.glDisable(GL_LIGHTING);
		
		gl.glEnable(GL_FRONT_FACE); // both sides of polygon
		
		if( nativeInit() == 1 ) loaded = true;
	} // END OF - public void init()
	
	public void destroy()
	{
		loaded = false;
	} // END OF - public void destroy()
	
	public void quit()
	{
		performAction( JavaGL.PERFORM_ACTION_QUIT, 0 );
		freeGLContext();
		cvsDispose();
	} // END OF - public void quit()
	
	public void reshape( int width, int height )
	{
		canvasWidth = width;
		canvasHeight = height;
		
		reshapeCanvas( canvasWidth, canvasHeight );
	} // END OF - public void reshape( int, int )
	
	public void display()
	{
		if( glj.gljMakeCurrent() == false ) return;
		
		// Quit function MUST wait for renderScene() to finish COMPLETELY
		if( loaded ) renderScene();
		else quit();
		
		glj.gljSwap();
		glj.gljFree();
	} // END OF - public void display()
	
	public void mouseDragged( MouseEvent event )
	{
		if( event.getModifiers() == (MouseEvent.SHIFT_MASK | MouseEvent.BUTTON1_MASK) )
		{
			if( cameraPanelLoaded )
			cameraPanel.rotate( mouseX - event.getX(), mouseY - event.getY() );
		}
		else if( event.getModifiers() == (MouseEvent.SHIFT_MASK | MouseEvent.BUTTON3_MASK) )
		{
			if( cameraPanelLoaded )
			cameraPanel.zoom( mouseY - event.getY() );
		}
		else if( event.getModifiers() == (MouseEvent.SHIFT_MASK | MouseEvent.BUTTON2_MASK) )
		{
			if( cameraPanelLoaded )
			cameraPanel.height( mouseY - event.getY() );
		}
		else if( event.getModifiers() == (MouseEvent.ALT_MASK | MouseEvent.BUTTON1_MASK) )
		{
			performAction( JavaGL.PERFORM_ACTION_TRANSLATE_X, event.getX() - mouseX );
			performAction( JavaGL.PERFORM_ACTION_TRANSLATE_Y, mouseY - event.getY() );
		}
		else if( event.getModifiers() == (MouseEvent.ALT_MASK | MouseEvent.BUTTON3_MASK) )
		{
			performAction( JavaGL.PERFORM_ACTION_TRANSLATE_Z, mouseY - event.getY() );
		}
		else if( event.getModifiers() == (MouseEvent.CTRL_MASK | MouseEvent.BUTTON1_MASK) )
		{
			performAction( JavaGL.PERFORM_ACTION_ROTATE, mouseY - event.getY() );
		}
		else
		{
			dragPoint( event.getX(), event.getY() );
		}
		
		mouseX = event.getX();
		mouseY = event.getY();
	} // END OF - public void mouseDragged( MouseEvent )
	
	public void mouseMoved( MouseEvent event ){}
	public void mouseClicked( MouseEvent event ){}
	public void mouseEntered( MouseEvent event ){}
	public void mouseExited( MouseEvent event ){}
	public void mousePressed( MouseEvent event )
	{
		clickedOnce = !clickedOnce;
		mouseX = event.getX();
		mouseY = event.getY();
		
		if( clickedOnce )
		{
			if( event.getModifiers() == MouseEvent.BUTTON1_MASK )
			{
				pickPoint( mouseX, mouseY );
			}
			else if( event.getModifiers() == MouseEvent.BUTTON3_MASK )
			{
				lockPoint( mouseX, mouseY );
			}
		}
	} // END OF - public void mousePressed( MouseEvent )
        
	public void mouseReleased( MouseEvent event )
	{
		mouseRelease();
	} // END OF - public void mouseReleased( MouseEvent )
	
	public void saveScreenshot( String filename )
	{
            if( filename != null )
            {
		takeScreenshot = true;
                screenshotName = filename;
            }
	} // END OF - public void saveScreenshot( String )
        
	public void renderOutput( String filename, boolean value )
	{
		renderOutputOn = value;
                renderName = filename;
	} // END OF - public void renderOutput( String, boolean )
        
        private String IncreamentScreenshotFilename( String filename )
        {
            int stringLength = filename.length();
            int location = stringLength - 5;
            int location2 = location - 1;
            char array[] = new char[stringLength];
            filename.getChars( 0, stringLength, array, 0 );
            
            if( array[location] >= '0' && array[location] <= '8' ) array[location]++;
            else
            {
                while( location2 >= 0 )
                {
                    if( array[location2] >= '0' && array[location2] <= '8' )
                    {
                        for( int i = location2 + 1; i <= location; i++ ) array[i] = '0';
                        array[location2]++;
                        location2 = -1;
                    }
                    location2--;
                }
            }
            if( location > 0 || location == -1 )
            {
                filename = new String( array );
            }
            return filename;
        } // END OF - private String IncreamentScreenshotFilename( String )
} // END OF - public class JavaGL extends GLAnimCanvas implements MouseListener, MouseMotionListener
