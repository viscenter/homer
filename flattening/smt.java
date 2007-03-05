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
*    Filename: "smt.java"                                                                          *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class is designed to start the application and manage simple interactions       *
*          like: toolbar buttons, load, and quit.  It parses command line arguments and holds      *
*          handles to the neccesary panel classes.                                                 *
*                                                                                                  *
*    Modifications:                                                                                *
*             Added public void SaveProject( String ) and public void LoadProject( String ).       *
*          Added cameraFilename and displayFilename for loading purposes.                          *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       JavaGL:                                                                                    *
*          canvas - handle to the JavaGL canvas (OpenGL window).                                   *
*       SavePanel:                                                                                 *
*          savePanel - Handle to the Save Panel.                                                   *
*       CameraPanel:                                                                               *
*          cameraPanel - Handle to the Camera Panel.                                               *
*       DisplayPanel:                                                                              *
*          displayPanel - Handle to the Display Panel.                                             *
*       PropertiesPanel:                                                                           *
*          propertiesPanel - Handle to the Properties Panel.                                       *
*       SmtKeyListener:                                                                            *
*          smtKeyListener - handle keyboard input.                                                 *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       Container:                                                                                 *
*          frameContainer - container of this JFrame.                                              *
*       JPanel:                                                                                    *
*          loadPanel - panel used to display load information.                                     *
*          controlPanel - side panel that holds various panels.                                    *
*          canvasPanel - panel that holds the JavaGL canvas.                                       *
*          toolbarPanel - panel that holds the toolbar buttons.                                    *
*       JTextField:                                                                                *
*          meshTextField - Part of loadPanel.  Current filename of mesh file.                      *
*          textureTextField - Part of loadPanel.  Current filename of texture file.                *
*       JButton:                                                                                   *
*          loadMeshFileButton - Part of loadPanel.  Change the mesh file.                          *
*          loadTextureFileButton - Part of loadPanel.  Change the texture file.                    *
*          commitLoadButton - Part of loadPanel.  Start the program.                               *
*          loadButton - Toolbar button to open Load Panel.                                         *
*          saveButton - Toolbar button to load the Save Panel into the side panel.                 *
*          cameraControls - Toolbar button to load the Camera Panel into the side panel.           *
*          displayControls - Toolbar button to load the Display Panel into the side panel.         *
*          propertiesControls - Button to load the Properties Panel into the side panel.           *
*       String:                                                                                    *
*          cameraFilename - Filename of the saved camera settings.                                 *
*          displayFilename - Filename of the saved display settings.                               *
*          meshFilename - Filename of the mesh file.                                               *
*          textureFilename - Filename of the texture file.                                         *
*          settingsFilename - Filename to use when loading settings.                               *
*          lockedFilename - Filename of a file with lock particle information.                     *
*          scriptFilename - Filename of a script file for recording or playing.                    *
*       boolean:                                                                                   *
*          viewLoadPanel - Does the Load Panel have to be seen.                                    *
*          canvasLoaded - Is the canvas loaded.                                                    *
*          debug - Is output debug infomation requested.                                           *
*       int:                                                                                       *
*          windowHeight - height of the window.                                                    *
*          windowWidth - width of the window.                                                      *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       smt( String[] )                                                                            *
*          - Create the application.                                                               *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       void UpdateControlPanel( int )                                                             *
*          - Places the correct panel into the side panel.                                         *
*       void LoadProject( String )                                                                 *
*          - Loads a saved project file.                                                           *
*       void SaveProject( String )                                                                 *
*          - Saves all of the current settings.                                                    *
*       void SaveCamera( String )                                                                  *
*          - Saves camera settings.                                                                *
*       void SaveProperties( String )                                                              *
*          - Save smt properties.                                                                  *
*       void SaveLocked( String )                                                                  *
*          - Save locked points.                                                                   *
*       void QUIT()                                                                                *
*          - Exit the program safely.                                                              *
*       void actionPerformed( ActionEvent )                                                        *
*          - Handle all of the button events.                                                      *
*       void windowActivated( WindowEvent )                                                        *
*          - VOID                                                                                  *
*       void windowClosed( WindowEvent )                                                           *
*          - VOID                                                                                  *
*       void windowClosing( WindowEvent )                                                          *
*          - Calls QUIT().                                                                         *
*       void windowDeactivated( WindowEvent )                                                      *
*          - VOID                                                                                  *
*       void windowDeiconified( WindowEvent )                                                      *
*          - VOID                                                                                  *
*       void windowGainedFocus( WindowEvent )                                                      *
*          - VOID                                                                                  *
*       void windowIconified( WindowEvent )                                                        *
*          - Calls QUIT().                                                                         *
*       void windowLostFocus( WindowEvent )                                                        *
*          - VOID                                                                                  *
*       void windowOpened( WindowEvent )                                                           *
*          - VOID                                                                                  *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       void CreateNewCanvas()                                                                     *
*          - Create and load the JavaGL canvas into the window.                                    *
*       void DisplayLoadPanel()                                                                    *
*          - Display the Load Panel - Closes and destroys any open Canvas.                         *
*       void LoadPanels()                                                                          *
*          - Setups up all of the needed side panels.                                              *
*       void ParseArguments( String[] )                                                            *
*          - Parses the command line arguments.                                                    *
*                                                                                                  *
***************************************************************************************************/

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.*;

public class smt extends JFrame implements ActionListener, WindowListener, WindowFocusListener
{
	public SavePanel savePanel;
	public CameraPanel cameraPanel;
	public DisplayPanel displayPanel;
	public PropertiesPanel propertiesPanel;
	public JavaGL canvas;
        public SmtKeyListener smtKeyListener;
	private Container frameContainer;
	private JPanel loadPanel, controlPanel, canvasPanel, toolbarPanel;
	private JTextField meshTextField, textureTextField;
	private JButton loadMeshFileButton, loadTextureFileButton, commitLoadButton;
	private JButton loadButton, saveButton, cameraControls, displayControls, propertiesControls;
        private String cameraFilename, displayFilename;
	private String meshFilename, textureFilename, settingsFilename, lockedFilename, scriptFilename;
	private boolean viewLoadPanel, canvasLoaded, debug;
	private int windowHeight, windowWidth;
	
	public smt( String args[] )
	{
		super( "University of Kentucky's - Scroll Manipulation Toolkit" );
		
                cameraFilename = "";
                displayFilename = "";
		meshFilename = "";
		textureFilename = "";
		lockedFilename = "";
		scriptFilename = "";
		settingsFilename = "settings.smt";
		viewLoadPanel = true;
		canvasLoaded = false;
		debug = false;
		
		windowHeight = 800;
		windowWidth = 600;
		
                smtKeyListener = new SmtKeyListener( this );
                
		// Get the JFrame's contant pane
		frameContainer = getContentPane();
		frameContainer.setLayout( new BorderLayout() );
		
		LoadPanels();
		
		// Parse the command line arguments
		ParseArguments( args );
		
		// If a mesh needs to be load, then display the load page, else load the mesh
		if( viewLoadPanel ) DisplayLoadPanel();
		else CreateNewCanvas();
		
		// Window Listeners for this class
		addWindowListener( this );
		addWindowFocusListener( this );
		
		setSize( windowHeight, windowWidth );
		setVisible( true );
	} // END OF - public smt( String args[] )
	
	public static void main( String args[] )
	{
		smt appl = new smt( args );
	} // END OF - public static void main( String[] )
	
	private void ParseArguments( String arguments[] )
	{
		int index;
		String temp;
		
		for( int i = 0; i < arguments.length; i++ )
		{
			try
			{
				if( arguments[i].charAt( 0 ) != '-' )
				{
					index = arguments[i].lastIndexOf( "." );
					temp = arguments[i].substring( index );
					if( temp.equals( ".quad" ) || temp.equals( ".surf" ) )
						meshFilename = arguments[i];
					else if( temp.equals( ".ppm" ) )
						textureFilename = arguments[i];
					else
						System.out.println( "Command line parse error: " + arguments[i] );
				}
				else
				{
					if( arguments[i].equals( "--help" ) || arguments[i].equals( "-h" ) || arguments[i].equals( "-?" ) )
					{
						System.out.println( "Usage: java smt (options) (files) " );
						System.out.println( "  Options: " );
						System.out.println( "    -debug                Allows debug information to be displayed" );
						System.out.println( "    -ll (filename)        Load a file containing locked points" );
						System.out.println( "    -lc (filename)        Load a file containing camera information" );
						System.out.println( "    -ls (filename)        Load the given settings file" );
						System.out.println( "    -lp (filename)        Load the given project file" );
						System.out.println( "    -script (filename)    Load the given script file" );
						System.out.println( "    -?, -h, --help        List standard options" );
						System.out.println( "  Files: " );
						System.out.println( "    *.surf *.quad   - possible mesh files" );
						System.out.println( "    *.ppm           - possible texture files" );
						QUIT();
					}
					else if( arguments[i].equals( "-debug" ) )
					{
						debug = true;
					}
					else if( arguments[i].equals( "-ll" ) )
					{
						index = arguments[i+1].lastIndexOf( "." );
						temp = arguments[i+1].substring( index );
						if( temp.equals( ".slf" ) )
						{
							lockedFilename = arguments[i+1];
							i++;
						}
					}
					else if( arguments[i].equals( "-ls" ) )
					{
						index = arguments[i+1].lastIndexOf( "." );
						temp = arguments[i+1].substring( index );
						if( temp.equals( ".smt" ) )
						{
							settingsFilename = arguments[i+1];
							i++;
						}
					}
					else if( arguments[i].equals( "-lp" ) )
					{
						index = arguments[i+1].lastIndexOf( "." );
						temp = arguments[i+1].substring( index );
						if( temp.equals( ".spf" ) )
						{
							LoadProject( arguments[i+1] );
							i++;
						}
					}
					else if( arguments[i].equals( "-lc" ) )
					{
						index = arguments[i+1].lastIndexOf( "." );
						temp = arguments[i+1].substring( index );
						if( temp.equals( ".scf" ) )
						{
							cameraFilename = arguments[i+1];
							i++;
						}
					}
					else if( arguments[i].equals( "-script" ) )
					{
						index = arguments[i+1].lastIndexOf( "." );
						temp = arguments[i+1].substring( index );
						if( temp.equals( ".ssf" ) )
						{
							scriptFilename = arguments[i+1];
							i++;
						}
					}
				}
			}
			catch( IndexOutOfBoundsException exception )
			{
				System.out.println( "Command line parse error: " + arguments[i] );
			}
		}
		
		int index1 = meshFilename.lastIndexOf( "." );
		int index2 = textureFilename.lastIndexOf( "." );
		
		if( index1 > 0 && index2 > 0 )
			viewLoadPanel = false;
	} // END OF - private void ParseArguments( String[] )
	
	private void LoadPanels()
	{
		loadButton = new JButton( "Load" );
		loadButton.addActionListener( this );
		saveButton = new JButton( "Save" );
		saveButton.addActionListener( this );
		
		cameraControls = new JButton( "Camera Controls" );
		cameraControls.addActionListener( this );
		
		displayControls = new JButton( "Display Controls" );
		displayControls.addActionListener( this );
		
		propertiesControls = new JButton( "Simulation Properties" );
		propertiesControls.addActionListener( this );
		
		JButton quitButton = new JButton( "Quit" );
		quitButton.addActionListener( this );
		
		toolbarPanel = new JPanel( new GridLayout( 1, 5 ) );
		JPanel toolbarSubPanel = new JPanel( new GridLayout( 1, 2 ) );
		toolbarSubPanel.add( loadButton );
		toolbarSubPanel.add( saveButton );
		toolbarPanel.add( toolbarSubPanel );
		toolbarPanel.add( displayControls );
		toolbarPanel.add( cameraControls );
		toolbarPanel.add( propertiesControls );
		toolbarPanel.add( quitButton );
		
		controlPanel = new JPanel( new BorderLayout() );
		controlPanel.setPreferredSize( new Dimension( 220, 0 ) );
	} // END OF - private void LoadPanels()
	
	private void DisplayLoadPanel()
	{
		UpdateControlPanel( 0 );
		frameContainer.removeAll();
		
		// 
		
		if( loadPanel == null )
		{
			loadPanel = new JPanel( new BorderLayout() );
			loadPanel.add( new JLabel( new ImageIcon( "images/smtlogo.jpg" ) ), BorderLayout.NORTH );
			
			loadMeshFileButton = new JButton( "Change" );
			loadMeshFileButton.addActionListener( this );
			loadTextureFileButton = new JButton( "Change" );
			loadTextureFileButton.addActionListener( this );
			commitLoadButton = new JButton( "Commit" );
			commitLoadButton.addActionListener( this );
			
			meshTextField = new JTextField( meshFilename );
			meshTextField.setEditable( false );
			textureTextField = new JTextField( textureFilename );
			textureTextField.setEditable( false );
			
			JPanel centerLoadPanel = new JPanel( new BorderLayout() );
			centerLoadPanel.setPreferredSize( new Dimension( 600, 100 ) );
			JPanel centerLoadSubPanelOne = new JPanel( new GridLayout( 3, 1 ) );
			JPanel centerLoadSubPanelTwo = new JPanel( new GridLayout( 3, 1 ) );
			JPanel centerLoadSubPanelThree = new JPanel( new GridLayout( 3, 1 ) );
			centerLoadSubPanelOne.setPreferredSize( new Dimension( 100, 0 ) );
			centerLoadSubPanelThree.setPreferredSize( new Dimension( 100, 0 ) );
			centerLoadSubPanelOne.add( new JLabel( " Mesh File:    " ) );
			centerLoadSubPanelOne.add( new JLabel( " Texture File: " ) );
			centerLoadSubPanelOne.add( new JLabel( "" ) );
			centerLoadSubPanelTwo.add( meshTextField );
			centerLoadSubPanelTwo.add( textureTextField );
			centerLoadSubPanelTwo.add( new JLabel( "" ) );
			centerLoadSubPanelThree.add( loadMeshFileButton );
			centerLoadSubPanelThree.add( loadTextureFileButton );
			centerLoadSubPanelThree.add( commitLoadButton );
			centerLoadPanel.add( centerLoadSubPanelOne, BorderLayout.WEST );
			centerLoadPanel.add( centerLoadSubPanelTwo, BorderLayout.CENTER );
			centerLoadPanel.add( centerLoadSubPanelThree, BorderLayout.EAST );
			
			JPanel bb = new JPanel();
			bb.add( centerLoadPanel );
			loadPanel.add( bb, BorderLayout.CENTER );
			
// options panel needs to be added
		}
		
		frameContainer.add( toolbarPanel, BorderLayout.NORTH );
		
		loadButton.setVisible( false );
		saveButton.setVisible( false );
		displayControls.setVisible( false );
		cameraControls.setVisible( false );
		propertiesControls.setVisible( false );
		
		frameContainer.add( loadPanel, BorderLayout.CENTER );
		frameContainer.validate();
	} // END OF - private void DisplayLoadPanel()
	
	public void UpdateControlPanel( int panel )
	{
		controlPanel.removeAll();
		controlPanel.setVisible( true );
		
		switch( panel )
		{
			case 1:{ controlPanel.add( cameraPanel ); } break;
			case 2:{ controlPanel.add( displayPanel ); } break;
			case 3:{ controlPanel.add( propertiesPanel ); } break;
                        case 4:{ controlPanel.add( savePanel ); } break;
			default:{ controlPanel.setVisible( false ); }
		}
		
		controlPanel.updateUI();
	} // END OF - public void UpdateControlPanel( int )
	
	private void CreateNewCanvas()
	{
		int index1 = meshFilename.lastIndexOf( "." );
		int index2 = textureFilename.lastIndexOf( "." );
		if( index1 > 0 && index2 > 0 )
		{
			if( canvas != null )
			{
				canvas.destroy();
				canvas = null;
			}
                        
			canvas = new JavaGL( this, 500, 500 );
			canvas.requestFocus();
                        canvas.addKeyListener( smtKeyListener );
			
			canvas.SetDebug( debug );
			
			// Load files into the program
			if( MeshFilter.getExtension( new File( meshFilename ) ).equals( ".quad" ) )
				canvas.performAction( JavaGL.PERFORM_ACTION_DEFINE_MESH_TYPE, JavaGL.PERFORM_ACTION_TRUE );
			else
				canvas.performAction( JavaGL.PERFORM_ACTION_DEFINE_MESH_TYPE, JavaGL.PERFORM_ACTION_FALSE );
			canvas.loadFilename( meshFilename, 0 );
			canvas.loadFilename( textureFilename, 1 );
			
			index1 = lockedFilename.lastIndexOf( "." );
			if( index1 > 0 )
			{
				canvas.loadFilename( lockedFilename, 2 );
			}
                        
			index1 = scriptFilename.lastIndexOf( "." );
			if( index1 > 0 )
			{
				canvas.loadFilename( scriptFilename, 3 );
			}
			
			frameContainer.removeAll();
			frameContainer.add( toolbarPanel, BorderLayout.NORTH );
			frameContainer.add( controlPanel, BorderLayout.WEST );
			frameContainer.add( canvas, BorderLayout.CENTER );
			frameContainer.validate();
			
			savePanel =	new SavePanel( canvas );
			if( cameraFilename != "" ) cameraPanel = new CameraPanel( canvas, cameraFilename );
			else cameraPanel = new CameraPanel( canvas );
			displayPanel = new DisplayPanel( canvas, cameraPanel );
			propertiesPanel = new PropertiesPanel( canvas, settingsFilename );
			
			loadButton.setVisible( true );
			saveButton.setVisible( true );
			displayControls.setVisible( true );
			cameraControls.setVisible( true );
			propertiesControls.setVisible( true );
			
			UpdateControlPanel( 2 );
			
			//canvas.init(); - called by gl4java
			canvas.start();
			
			if( displayFilename != "" ) displayPanel.LoadSettings( displayFilename );
		}
	} // END OF - private void CreateNewCanvas()
	
	public void actionPerformed( ActionEvent event )
	{
		Object source = event.getSource();
		
		if( source == loadMeshFileButton )
		{
			JFileChooser chooser = new JFileChooser();
			MeshFilter filter = new MeshFilter();
			chooser.setCurrentDirectory( new File( System.getProperty( "user.dir" ) ) );
			chooser.setFileFilter(filter);
			chooser.setDialogTitle( "Select Mesh File" );
			chooser.setApproveButtonText( "Change" );
			int returnVal = chooser.showOpenDialog(this);
			if( returnVal == JFileChooser.APPROVE_OPTION )
			{
				File file = chooser.getSelectedFile();
				meshFilename = file.getName();
				meshTextField.setText( meshFilename );
			}
		}
		else if( source == loadTextureFileButton )
		{
			JFileChooser chooser = new JFileChooser();
			TextureFilter filter = new TextureFilter();
			chooser.setCurrentDirectory( new File( System.getProperty( "user.dir" ) ) );
			chooser.setFileFilter(filter);
			chooser.setDialogTitle( "Select Texture File" );
			chooser.setApproveButtonText( "Change" );
			int returnVal = chooser.showOpenDialog(this);
			if( returnVal == JFileChooser.APPROVE_OPTION )
			{
				File file = chooser.getSelectedFile();
				textureFilename = file.getName();
				textureTextField.setText( textureFilename );
			}
		}
		else if( source == commitLoadButton )
		{
			CreateNewCanvas();
		}
		else if( source == loadButton )
		{
			DisplayLoadPanel();
		}
		else if( source == saveButton )
		{
                	UpdateControlPanel(4);
		}
		else if( event.getSource() == cameraControls )
		{
			UpdateControlPanel(1);
		}
		else if( event.getSource() == displayControls )
		{
			UpdateControlPanel(2);
		}
		else if( event.getSource() == propertiesControls )
		{
			UpdateControlPanel(3);
		}
		else
		{
			QUIT();
		}
	} // END OF - public void actionPerformed( ActionEvent )
        
	public void LoadProject( String filename )
        {
            if( filename == null || filename.length() < 0 ) return;
            
            BufferedReader inputStream = null;
            
            // Open file
            try
            {
                inputStream = new BufferedReader( new InputStreamReader( new FileInputStream( filename )));
            }
            catch( IOException exception ){ return; }
            
            try
            {
                String line = inputStream.readLine();
		if( line == null || !line.equals( "*SMT Settings" ) )
		{
                    inputStream.close();
                    if( debug ) System.out.println( "Wrong file format: \"" + filename  + "\"" );
                    return;
		}
                while( line != null && !line.equals( "**Filenames" ) )
		{
			line = inputStream.readLine();
		}
                
                meshFilename = inputStream.readLine();
                textureFilename = inputStream.readLine();
            }
            catch( IOException exception ){ return; }
            
            settingsFilename = filename;
            cameraFilename = filename;
            displayFilename = filename;
            lockedFilename = filename;
            
            // Close file
            try
            {
                if( inputStream != null ) inputStream.close();
            }
            catch( IOException exception ){}
	} // END OF - public void LoadProject( String )
	
	public void SaveProject( String filename )
        {
            try
            {
                PrintWriter writer = new PrintWriter( new BufferedWriter( new FileWriter( filename )));
                writer.println( "*SMT Settings" );
                writer.println( "" );
                writer.println( "**Filenames" );
                writer.println( meshFilename );
                writer.println( textureFilename );
                writer.println( "***" );
                writer.println( "" );
                
                propertiesPanel.SaveSettings( writer );
                displayPanel.SaveSettings( writer );
                cameraPanel.SaveSettings( writer );
                
                int lockedPoints[] = canvas.getLockedArray();
                
                if( lockedPoints.length > 0 )
                {
                    writer.println( "" );
                    writer.println( "**Locked Points" );
                    for( int i = 0; i < lockedPoints.length; i++ )
                    {
                        writer.println( "" + lockedPoints[i] );
                    }
                    
                    writer.println( "***" );
                    writer.println( "" );
                }
                writer.close();
            }
            catch( IOException exception )
            {
		if( canvas.Debug() ) System.out.println( "Unable to save project." );
            }
	} // END OF - public void SaveProject( String )
	
	public void SaveCamera( String filename )
        {
            try
            {
                PrintWriter writer = new PrintWriter( new BufferedWriter( new FileWriter( filename )));
                writer.println( "*SMT Settings" );
                writer.println( "" );
                cameraPanel.SaveSettings( writer );
                writer.close();
            }
            catch( IOException exception )
            {
		if( canvas.Debug() ) System.out.println( "Unable to save camera settings." );
            }
	} // END OF - public void SaveCamera( String )
	
	public void SaveProperties( String filename )
        {
            try
            {
                PrintWriter writer = new PrintWriter( new BufferedWriter( new FileWriter( filename )));
                writer.println( "*SMT Settings" );
                writer.println( "" );
                propertiesPanel.SaveSettings( writer );
                writer.close();
            }
            catch( IOException exception )
            {
		if( canvas.Debug() ) System.out.println( "Unable to save smt settings." );
            }
	} // END OF - public void SaveProperties( String )
        
	public void SaveLocked( String filename )
        {
            try
            {
                PrintWriter writer = new PrintWriter( new BufferedWriter( new FileWriter( filename )));
                writer.println( "*SMT Settings" );
                writer.println( "" );
            int lockedPoints[] = canvas.getLockedArray();
                
                if( lockedPoints.length > 0 )
                {
                    writer.println( "" );
                    writer.println( "**Locked Points" );
                    for( int i = 0; i < lockedPoints.length; i++ )
                    {
                        writer.println( "" + lockedPoints[i] );
                    }
                    
                    writer.println( "***" );
                    writer.println( "" );
                }
                writer.close();
            }
            catch( IOException exception )
            {
		if( canvas.Debug() ) System.out.println( "Unable to save locked points." );
            }
	} // END OF - public void SaveLocked( String )
        
        public void QUIT()
	{
		if( canvas != null ) canvas.destroy();
		System.exit(1);
	} // END OF - public void QUIT()
        
	public void windowActivated( WindowEvent event ){}
	public void windowClosed( WindowEvent event ){}
	public void windowDeactivated( WindowEvent event ){}
	public void windowDeiconified( WindowEvent event ){}
	public void windowIconified( WindowEvent event ){ QUIT(); }
	public void windowOpened( WindowEvent event ){}
	public void windowClosing( WindowEvent event ){ QUIT(); }
	public void windowGainedFocus(WindowEvent event ){}
	public void windowLostFocus(WindowEvent event ){}
} // END OF - public class smt extends JFrame implements ActionListener, WindowListener, WindowFocusListener