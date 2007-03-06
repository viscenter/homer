/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                  Completed - February 20th, 2004                                 *
*                                                                                                  *
*    Filename: "SavePanel.java"                                                                    *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class is needed to perform various save functions.  Like saving                 *
*          preferences, screenshots, script files, ...                                             *
*             The file chooser saves the file unless the selected file type has a                  *
*          text field, then the text field is altered to represent the "saved" file                *
*          but nothing is saved.                                                                   *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       JButton:                                                                                   *
*          screenshotButton - button used to take a screenshot.                                    *
*          scriptButton - button used to record a script file.                                     *
*          scriptRunButton - button used to test a script file.                                    *
*          scriptRunPlusCaptureButton - button used to render a script.                            *
*       boolean:                                                                                   *
*          scriptRunning - true if the script file is running. ( java side only )                  *
*          scriptRecording - true if the script is recording. ( java side only )                   *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       JavaGL:                                                                                    *
*          handle - Handle to the OpenGL canvas class - which communicates with the c side.        *
*       JFileChooser:                                                                              *
*          saveFileChooser - file chooser used when saving a file.                                 *
*       JButton:                                                                                   *
*          saveButton - button tells the file chooser to open.                                     *
*       JTextField:                                                                                *
*          screenshotTextField - text field that holds screenshot filename.                        *
*          scriptTextField -  text field that holds script filename.                               *
*       String:                                                                                    *
*          screenshotName - Name used when taking a screenshot.                                    *
*          scriptName - Name of the script file.                                                   *
*          renderOutputName - Generic filename used to render frames.                              *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       SavePanel( JavaGL )                                                                        *
*          - Creates the Save Panel - and setups the file chooser.                                 *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       void actionPerformed( ActionEvent )                                                        *
*          - Handle all of the button events.                                                      *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       String IncreamentScreenshotFilename( String )                                              *
*          - Increaments the number part of a string and returns the new string.                   *
*                                                                                                  *
***************************************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;

public class SavePanel extends JPanel implements ActionListener
{
        public JButton screenshotButton, scriptButton, scriptRunButton, scriptRunPlusCaptureButton;
        public boolean scriptRunning, scriptRecording;
	private JavaGL handle;
        private JFileChooser saveFileChooser;
        private JButton saveButton;
        private JTextField screenshotTextField, scriptTextField;
        private String screenshotName, scriptName, renderOutputName;
        
	public SavePanel( JavaGL javaGL )
	{
            handle = javaGL;
            screenshotName = "output/cap000.jpg";
            renderOutputName = "output/frame00000.jpg";
            scriptName = "test.ssf";
            scriptRecording = false;
            scriptRunning = false;
            
            saveFileChooser = new JFileChooser();
            saveFileChooser.setCurrentDirectory( new File( System.getProperty( "user.dir" ) ) );
            saveFileChooser.setDialogTitle( "Save File" );
            saveFileChooser.setApproveButtonText( "Save" );
            
            SmtFileFilters filter = new SmtFileFilters( ".jpg", "JPEG Image Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".ssf", "SMT Script Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".quad", "Quad Mesh File" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".surf", "Surf Mesh File" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".scf", "SMT Camera Orientation Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".slf", "SMT Locked Points Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".smt", "SMT Properties Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            filter = new SmtFileFilters( ".spf", "SMT Project Files" );
            saveFileChooser.addChoosableFileFilter(filter);
            
            setLayout( new BorderLayout() );
            addKeyListener( handle.handle.smtKeyListener );
            JPanel controlPanel = new JPanel( new GridLayout( 3, 1 ) );
            controlPanel.addKeyListener( handle.handle.smtKeyListener );
            
            JPanel savePanel = new JPanel( new GridLayout( 1, 1 ) );
            savePanel.setBorder( BorderFactory.createTitledBorder( "SMT File" ) );
            saveButton = new JButton( "Save" );
            saveButton.addActionListener( this );
            saveButton.addKeyListener( handle.handle.smtKeyListener );
            savePanel.add( saveButton );
            savePanel.addKeyListener( handle.handle.smtKeyListener );
            
            controlPanel.add( savePanel );
            
            JPanel screenshotPanel = new JPanel( new GridLayout( 2, 1 ) );
            screenshotPanel.setBorder( BorderFactory.createTitledBorder( "Screenshot" ) );
            screenshotTextField = new JTextField( screenshotName );
            screenshotTextField.addKeyListener( handle.handle.smtKeyListener );
            screenshotPanel.add( screenshotTextField );
            screenshotButton = new JButton( "Take Screenshot" );
            screenshotButton.addActionListener( this );
            screenshotButton.addKeyListener( handle.handle.smtKeyListener );
            screenshotPanel.add( screenshotButton );
            screenshotPanel.addKeyListener( handle.handle.smtKeyListener );
            
            controlPanel.add( screenshotPanel );
            
            JPanel scriptPanel = new JPanel( new BorderLayout() );
            scriptPanel.setBorder( BorderFactory.createTitledBorder( "Script" ) );
            scriptPanel.addKeyListener( handle.handle.smtKeyListener );
            JPanel scriptSubPanel = new JPanel( new GridLayout( 3, 1 ) );
            scriptTextField = new JTextField( scriptName );
            scriptTextField.addKeyListener( handle.handle.smtKeyListener );
            scriptSubPanel.add( scriptTextField );
            scriptSubPanel.addKeyListener( handle.handle.smtKeyListener );
            scriptButton = new JButton( "Start recording" );
            scriptButton.addActionListener( this );
            scriptButton.addKeyListener( handle.handle.smtKeyListener );
            scriptSubPanel.add( scriptButton );
            scriptPanel.add( scriptSubPanel );
            scriptRunButton = new JButton( "Test" );
            scriptRunButton.addActionListener( this );
            scriptRunButton.addKeyListener( handle.handle.smtKeyListener );
            scriptRunPlusCaptureButton = new JButton( "Run" );
            scriptRunPlusCaptureButton.addActionListener( this );
            scriptRunPlusCaptureButton.addKeyListener( handle.handle.smtKeyListener );
            JPanel scriptRunSubPanel = new JPanel( new GridLayout( 1, 2 ) );
            scriptRunSubPanel.add( scriptRunButton );
            scriptRunSubPanel.add( scriptRunPlusCaptureButton );
            scriptRunSubPanel.addKeyListener( handle.handle.smtKeyListener );
            scriptSubPanel.add( scriptRunSubPanel );
            
            controlPanel.add( scriptPanel );
            
            add( controlPanel, BorderLayout.NORTH );
	} // END OF - public SavePanel( JavaGL )
	
	public void actionPerformed( ActionEvent event )
	{
            Object source = event.getSource();
            
            if( source == saveButton )
            {
                int returnVal = saveFileChooser.showOpenDialog(this);
                if( returnVal == JFileChooser.APPROVE_OPTION )
                {
                    File file = saveFileChooser.getSelectedFile();
                    String fileName = file.getPath();
                    SmtFileFilters filter = (SmtFileFilters)saveFileChooser.getFileFilter();
                    String filterExtension = filter.getExtension();
                    String currentExtension = filter.getExtension( file );
                    if( currentExtension == null || !currentExtension.equals( filterExtension ) )
                    {
                        fileName += filterExtension;
                    }
                    
                    if( filterExtension.equals( ".smt" ) )
                    {
                        if( handle.Debug() ) System.out.println( "Saving: " + fileName );
                        handle.handle.SaveProperties( fileName );
                    }
                    else if( filterExtension.equals( ".slf" ) )
                    {
                        if( handle.Debug() ) System.out.println( "Saving: " + fileName );
                        handle.handle.SaveLocked( fileName );
                    }
                    else if( filterExtension.equals( ".spf" ) )
                    {
                        if( handle.Debug() ) System.out.println( "Saving: " + fileName );
                        handle.handle.SaveProject( fileName );
                    }
                    else if( filterExtension.equals( ".scf" ) )
                    {
                        if( handle.Debug() ) System.out.println( "Saving: " + fileName );
                        handle.handle.SaveCamera( fileName );
                    }
                    else if( filterExtension.equals( ".ssf" ) )
                    {
                        scriptName = fileName;
                        scriptTextField.setText( scriptName );
                    }
                    else if( filterExtension.equals( ".jpg" ) )
                    {
                        screenshotName = fileName;
                        screenshotTextField.setText( screenshotName );
                    }
                    else if( filterExtension.equals( ".surf" ) || filterExtension.equals( ".quad" ) )
                    {
                        if( handle.Debug() ) System.out.println( "Saving: " + fileName );
                        handle.loadFilename( fileName, 4 );
                        handle.performAction( JavaGL.PERFORM_ACTION_SAVE_MESH_FILE, JavaGL.PERFORM_ACTION_TRUE );
                    }
                }
            }
            else if( source == screenshotButton )
            {
                try
                {
                    String filename = screenshotTextField.getText();
                    String extension = SmtFileFilters.getExtension( new File( filename ) );
                    if( extension == null || !extension.equals( ".jpg" ) ) filename += ".jpg";
                    screenshotName = filename;
                    screenshotTextField.setText( screenshotName );
                    if( handle.Debug() ) System.out.println( "Saving: " + filename );
                    handle.saveScreenshot( filename );
                    screenshotName = IncreamentScreenshotFilename( filename );
                    screenshotTextField.setText( screenshotName );
                }
                catch( NullPointerException exception ){}
            }
            else if( !scriptRunning && source == scriptButton )
            {
                if( scriptRecording == false )
                {
                    try
                    {
                        String filename = scriptTextField.getText();
                        String extension = SmtFileFilters.getExtension( new File( filename ) );
                        if( extension == null || !extension.equals( ".ssf" ) )
                        {
                            filename += ".ssf";
                            scriptName = filename;
                        }
                        if( handle.Debug() ) System.out.println( "Recording to: " + filename );
                        handle.loadFilename( filename, 3 );
                        scriptRecording = true;
                        scriptButton.setText( "STOP Recording" );
                        handle.performAction( handle.PERFORM_ACTION_RECORD_SCRIPT_FILE, handle.PERFORM_ACTION_TRUE );
                    }
                    catch( NullPointerException exception ){}
                }
                else
                {
                    if( handle.Debug() ) System.out.println( "Stop Recording" );
                    scriptRecording = false;
                    scriptButton.setText( "Start Recording" );
                    handle.performAction( handle.PERFORM_ACTION_RECORD_SCRIPT_FILE, handle.PERFORM_ACTION_FALSE );
                }
            }
            else if( !scriptRecording && ( source == scriptRunButton || source == scriptRunPlusCaptureButton ) )
            {
                if( scriptRunning == false )
                {
                    try
                    {
                        String filename = scriptTextField.getText();
                        String extension = SmtFileFilters.getExtension( new File( filename ) );
                        if( extension == null || !extension.equals( ".ssf" ) )
                        {
                            filename += ".ssf";
                            scriptName = filename;
                        }
                        if( handle.Debug() ) System.out.println( "Running Script: " + filename );
                        handle.loadFilename( filename, 3 );
                        scriptRunning = true;
                        scriptRunButton.setText( "STOP" );
                        scriptRunPlusCaptureButton.setText( "STOP" );
                        handle.performAction( handle.PERFORM_ACTION_PLAY_SCRIPT_FILE, handle.PERFORM_ACTION_TRUE );
                        if( source == scriptRunPlusCaptureButton )
                        {
                            handle.renderOutput( renderOutputName, true );
                        }
                    }
                    catch( NullPointerException exception ){}
                }
                else
                {
                    if( handle.Debug() ) System.out.println( "Stop Recording" );
                    scriptRunning = false;
                    scriptRunButton.setText( "Test" );
                    scriptRunPlusCaptureButton.setText( "Run" );
                    handle.renderOutput( renderOutputName, false );
                    handle.performAction( handle.PERFORM_ACTION_PLAY_SCRIPT_FILE, handle.PERFORM_ACTION_FALSE );
                }
            }
        } // END OF - public void actionPerformed( ActionEvent )
        
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
} // END OF - public class SavePanel extends JPanel implements ActionListener, ChangeListener
 