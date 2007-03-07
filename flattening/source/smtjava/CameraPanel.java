/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
************************************************************************************************** *
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                   Completed - August 9th, 2003                                   *
*                                                                                                  *
*    Filename: "CameraPanel.java"                                                                  *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class manages the "Camera Panel".  Other classes call this class when they      *
*          manipulate the camera. ( To make sure the all camera information is up to date. )       *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       None                                                                                       *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       JavaGL:                                                                                    *
*          handle - Handle to the OpenGL canvas class - which communicates with the c side.        *
*       JButton:                                                                                   *
*          resetButton - Reset's all JSliders to default values.                                   *
*       JSlider:                                                                                   *
*          distance - Camera Panel's Slider for distance from "look at point".                     *
*          lookAtX - Camera Panel's Slider for "look at point" x coordinate.                       *
*          lookAtY - Camera Panel's Slider for "look at point" y coordinate.                       *
*          lookAtZ - Camera Panel's Slider for "look at point" z coordinate.                       *
*          xAxis - Camera Panel's Slider for x angle from "look at point".                         *
*          yAxis - Camera Panel's Slider for y angle from "look at point".                         *
*          distance2 - Display Panel's Slider for distance from "look at point".                   *
*          lookAtY2 - Display Panel's Slider for "look at point" y coordinate.                     *
*          xAxis2 - Display Panel's Slider x angle from "look at point".                           *
*          yAxis2 - Display Panel's Slider y angle from "look at point".                           *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       CameraPanel( JavaGL )                                                                      *
*          - Calls CreatePanel(). Passing "this" to the inputted handle.                           *
*       CameraPanel( JavaGL, String filename )                                                     *
*          - Same as CameraPanel( JavaGL ), but calls LoadSettings() after CreatePanel().          *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       JPanel CreateSubPanel()                                                                    *
*          - Creates a Panel with main camera features for the Display Panel.                      *
*       boolean SaveSettings( PrintWriter )                                                        *
*          - Writes the current values to the PrintWriter.                                         *
*       void rotate( int, int )                                                                    *
*          - Called outside of class - to keep information up to date.                             *
*       void height( int )                                                                         *
*          - Called outside of class - to keep information up to date.                             *
*       void zoom( int )                                                                           *
*          - Called outside of class - to keep information up to date.                             *
*       void reset()                                                                               *
*          - Used to reset the camera outside of this class.                                       *
*       void actionPerformed( ActionEvent )                                                        *
*          - Called when the reset button is pushed.                                               *
*       void stateChanged( ChangeEvent )                                                           *
*          - Called when a slider is adjusted.                                                     *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       void CreatePanel()                                                                         *
*          - Creates the Camera Panel.                                                             *
*       boolean LoadSettings( String )                                                             *
*          - Loads variables from a file into the JSliders.                                        *
*       boolean ParseLine( String, int )                                                           *
*          - Called by LoadSettings() to load the correct value into the correct JSlider.          *
*                                                                                                  *
***************************************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;

public class CameraPanel extends JPanel implements ActionListener, ChangeListener
{
	private JavaGL handle;
	private JButton resetButton;
	private JSlider lookAtX, lookAtY, lookAtZ, distance, xAxis, yAxis;
	private JSlider lookAtY2, distance2, xAxis2, yAxis2;
	
	public CameraPanel( JavaGL javaGL )
	{
		handle = javaGL;
		handle.setCamera( this );
		
		CreatePanel();
	}
	
	public CameraPanel( JavaGL javaGL, String filename )
	{
		handle = javaGL;
		handle.setCamera( this );
		
		CreatePanel();
		LoadSettings( filename );
	}
	
	public JPanel CreateSubPanel()
	{
		JPanel subPanel = new JPanel( new GridLayout( 4, 1 ) );
		
		subPanel.add( xAxis2 );
		subPanel.add( yAxis2 );
		subPanel.add( distance2 );
		subPanel.add( lookAtY2 );
		
		return subPanel;
	} // END OF - public JPanel CreateSubPanel()
	
	public boolean SaveSettings( PrintWriter writer )
	{
		if( writer == null ) return false;
		writer.println( "" );
		writer.println( "**Camera Panel" );
		writer.println( "" + lookAtX.getValue() );
		writer.println( "" + lookAtY.getValue() );
		writer.println( "" + lookAtZ.getValue() );
		writer.println( "" + distance.getValue() );
		writer.println( "" + xAxis.getValue() );
		writer.println( "" + yAxis.getValue() );
		writer.println( "***" );
		writer.println( "" );
		writer.println( "" );
		return true;
	} // END OF - public boolean SaveSettings( PrintWriter )
	
	public void rotate( int deltaX, int deltaY )
	{
		if( deltaX != 0 ) xAxis.setValue( (int)xAxis.getValue() + deltaX );
		if( deltaY != 0 ) yAxis.setValue( (int)yAxis.getValue() + deltaY );
	} // END OF - public void rotate( int, int )
	
	public void height( int deltaY )
	{
		if( deltaY != 0 ) lookAtY.setValue( (int)lookAtY.getValue() + deltaY );
	} // END OF - public void height( int )
	
	public void zoom( int deltaY )
	{
		if( deltaY != 0 ) distance.setValue( (int)distance.getValue() + deltaY );
	} // END OF - public void zoom( int )
	
	public void reset()
	{
		resetButton.doClick();
	} // END OF - public void reset()
	
	public void actionPerformed( ActionEvent event )
	{
		if( event.getSource() == resetButton )
		{
			xAxis.setValue(0);
			yAxis.setValue(0);
			distance.setValue(50);
			lookAtX.setValue(0);
			lookAtY.setValue(0);
			lookAtZ.setValue(0);
			handle.performAction( JavaGL.PERFORM_ACTION_RESET_CAMERA, 0 );
		}
	} // END OF - public void actionPerformed( ActionEvent )

	public void stateChanged( ChangeEvent event )
	{
		if( event.getSource() == lookAtX )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_AXIS_X, (int)lookAtX.getValue() );
		}
		else if( event.getSource() == lookAtY )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_AXIS_Y, (int)lookAtY.getValue() );
			if( lookAtY2.getValue() != lookAtY.getValue() )
				lookAtY2.setValue( lookAtY.getValue() );
		}
		else if( event.getSource() == lookAtZ )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_AXIS_Z, (int)lookAtZ.getValue() );
		}
		else if( event.getSource() == distance )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_DISTANCE, (int)distance.getValue() );
			if( distance2.getValue() != distance.getValue() )
				distance2.setValue( distance.getValue() );
		}
		else if( event.getSource() == xAxis )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ROTATE_ANGLE_X, (int)xAxis.getValue() );
			if( xAxis2.getValue() != xAxis.getValue() )
				xAxis2.setValue( xAxis.getValue() );
		}
		else if( event.getSource() == yAxis )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_ROTATE_ANGLE_Y, (int)yAxis.getValue() );
			if( yAxis2.getValue() != yAxis.getValue() )
				yAxis2.setValue( yAxis.getValue() );
		}
		else if( event.getSource() == lookAtY2 )
		{
			if( lookAtY.getValue() != lookAtY2.getValue() )
				lookAtY.setValue( lookAtY2.getValue() );
		}
		else if( event.getSource() == distance2 )
		{
			if( distance.getValue() != distance2.getValue() )
				distance.setValue( distance2.getValue() );
		}
		else if( event.getSource() == xAxis2 )
		{
			if( xAxis.getValue() != xAxis2.getValue() )
				xAxis.setValue( xAxis2.getValue() );
		}
		else if( event.getSource() == yAxis2 )
		{
			if( yAxis.getValue() != yAxis2.getValue() )
				yAxis.setValue( yAxis2.getValue() );
		}
	} // END OF - public void stateChanged( ChangeEvent )
	
	
	private void CreatePanel()
	{
		xAxis = new JSlider( JSlider.HORIZONTAL, -180, 180, 1 );
		xAxis.setMajorTickSpacing(180);
		xAxis.setMinorTickSpacing(30);
		xAxis.setPaintTicks(true);
		xAxis.setValue(0);
		xAxis.setBorder( BorderFactory.createTitledBorder( "Camera Angle X" )  );
		xAxis.addChangeListener(this);
		
		yAxis = new JSlider( JSlider.HORIZONTAL, -89, 89, 1 );
		yAxis.setMajorTickSpacing(89);
		yAxis.setMinorTickSpacing(30);
		yAxis.setPaintTicks(true);
		yAxis.setValue(0);
		yAxis.setBorder( BorderFactory.createTitledBorder( "Camera Angle Y" )  );
		yAxis.addChangeListener(this);
		
		distance = new JSlider( JSlider.HORIZONTAL, 1, 101, 1 );
		distance.setMajorTickSpacing(50);
		distance.setMinorTickSpacing(10);
		distance.setPaintTicks(true);
		distance.setValue(50); // divide by 2
		distance.setBorder( BorderFactory.createTitledBorder( "Distance" )  );
		distance.addChangeListener(this);
		
		lookAtX = new JSlider( JSlider.HORIZONTAL, -500, 500, 1 );
		lookAtX.setMajorTickSpacing(100);
		lookAtX.setPaintTicks(true);
		lookAtX.setValue(0);
		lookAtX.setBorder( BorderFactory.createTitledBorder( "Camera X" )  );
		lookAtX.addChangeListener(this);
		
		lookAtY = new JSlider( JSlider.HORIZONTAL, -500, 500, 1 );
		lookAtY.setMajorTickSpacing(100);
		lookAtY.setPaintTicks(true);
		lookAtY.setValue(0);
		lookAtY.setBorder( BorderFactory.createTitledBorder( "Camera Height" )  );
		lookAtY.addChangeListener(this);
		
		lookAtZ = new JSlider( JSlider.HORIZONTAL, -500, 500, 1 );
		lookAtZ.setMajorTickSpacing(100);
		lookAtZ.setPaintTicks(true);
		lookAtZ.setValue(0);
		lookAtZ.setBorder( BorderFactory.createTitledBorder( "Camera Z" )  );
		lookAtZ.addChangeListener(this);
		
		xAxis2 = new JSlider( JSlider.HORIZONTAL, -180, 180, 1 );
		xAxis2.setMajorTickSpacing(180);
		xAxis2.setMinorTickSpacing(30);
		xAxis2.setPaintTicks(true);
		xAxis2.setValue( xAxis.getValue() );
		xAxis2.setBorder( BorderFactory.createTitledBorder( "Camera Angle X" )  );
		xAxis2.addChangeListener(this);
		
		yAxis2 = new JSlider( JSlider.HORIZONTAL, -89, 89, 1 );
		yAxis2.setMajorTickSpacing(89);
		yAxis2.setMinorTickSpacing(30);
		yAxis2.setPaintTicks(true);
		yAxis2.setValue( yAxis.getValue() );
		yAxis2.setBorder( BorderFactory.createTitledBorder( "Camera Angle Y" )  );
		yAxis2.addChangeListener(this);
		
		distance2 = new JSlider( JSlider.HORIZONTAL, 1, 101, 1 );
		distance2.setMajorTickSpacing(50);
		distance2.setMinorTickSpacing(10);
		distance2.setPaintTicks(true);
		distance2.setValue( distance.getValue() ); // divide by 2
		distance2.setBorder( BorderFactory.createTitledBorder( "Distance" )  );
		distance2.addChangeListener(this);
		
		lookAtY2 = new JSlider( JSlider.HORIZONTAL, -500, 500, 1 );
		lookAtY2.setMajorTickSpacing(100);
		lookAtY2.setPaintTicks(true);
		lookAtY2.setValue( lookAtY.getValue() );
		lookAtY2.setBorder( BorderFactory.createTitledBorder( "Camera Height" )  );
		lookAtY2.addChangeListener(this);
		
		resetButton = new JButton( "Reset Camera Position" );
		resetButton.addActionListener( this );
		
		JPanel controlPanel = new JPanel( new GridLayout( 7, 1 ) );
		controlPanel.add( resetButton );
		controlPanel.add( xAxis );
		controlPanel.add( yAxis );
		controlPanel.add( distance );
		controlPanel.add( lookAtX );
		controlPanel.add( lookAtY );
		controlPanel.add( lookAtZ );
		
		add( controlPanel );
	} // END OF - private void CreatePanel()
	
	private boolean LoadSettings( String filename )
	{
		if( filename == null || filename.length() < 0 ) return false;
		
		BufferedReader inputStream = null;
		
		// Open file
		try
		{
			inputStream = new BufferedReader( new InputStreamReader( new FileInputStream( filename )));
		}
		catch( IOException exception )
		{
			if( handle.Debug() ) System.out.println( "Unable to open: \"" + filename  + "\"" );
			return false;
		}
		
		try
		{
			String line = inputStream.readLine();
			if( line == null || !line.equals( "*SMT Settings" ) )
			{
				inputStream.close();
				if( handle.Debug() ) System.out.println( "Wrong file format" );
				return false;
			}
			
			while( line != null && !line.equals( "**Camera Panel" ) )
			{
				line = inputStream.readLine();
			}
			
			line = inputStream.readLine();
			int i = 0;
			while( line != null && !line.equals( "***" ) && i < 6 )
			{
				if( !ParseLine( line, i ) )
				{
					if( handle.Debug() ) System.out.println( "Number Format Error line: \"" + line );
					return false;
				}
				
				i++;
				line = inputStream.readLine();
			}
		}
		catch( IOException exception )
		{
			if( handle.Debug() ) System.out.println( "Unable to read File" );
			return false;
		}
		
		// Close file
		try
		{
			if( inputStream != null ) inputStream.close();
		}
		catch( IOException exception ){ if( handle.Debug() ) System.out.println( "Unable to close: \"" + filename  + "\"" ); }
		
		return true;
	} // END OF - private boolean LoadSettings( String )
	
	private boolean ParseLine( String line, int number )
	{
		try
		{
			switch( number )
			{
				case 0:{ lookAtX.setValue( Integer.parseInt( line ) ); } break;
				case 1:{ lookAtY.setValue( Integer.parseInt( line ) ); } break;
				case 2:{ lookAtZ.setValue( Integer.parseInt( line ) ); } break;
				case 3:{ distance.setValue( Integer.parseInt( line ) ); } break;
				case 4:{ xAxis.setValue( Integer.parseInt( line ) ); } break;
				case 5:{ yAxis.setValue( Integer.parseInt( line ) ); } break;
				default: { return false; }
			}
		}
		catch( NumberFormatException exception ){ return false; }
		return true;
	} // END OF - private boolean ParseLine( String, int )
} // END OF - public class CameraPanel extends JPanel implements ActionListener, ChangeListener
