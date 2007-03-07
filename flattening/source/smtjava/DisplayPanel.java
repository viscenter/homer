/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                   Completed - August 9th, 2003                                   *
*                                 Last Modified - October 23th, 2003                               *
*                                                                                                  *
*    Filename: "DisplayPanel.java"                                                                 *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class manages the "Display Panel".                                              *
*                                                                                                  *
*    Modifications:                                                                                *
*             Removed buttons for adjusting intergration type.  Changed private                    *
*          LoadSettings( String ) to public and removed a constructor.                             *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       JButton:                                                                                   *
*          collisionActiveButton - Button indicating the value of collisionActive.                 *
*          displaySpringsButton - Button indicating the value of displayingSprings.                *
*          displayVerticesButton - Button indicating the value of displayingVertices.              *
*          lockParticlesButton - Button indicating the value of lockParticles.                     *
*          particleClearButton - Button used to clear the lock particle array.                     *
*          resetButton - Button used to reset the simulation.                                      *
*          runningButton - Button indicating the value of running.                                 *
*          useDampingButton - Button indicating the value of useDamping.                           *
*          useGravityButton - Button indicating the value of useGravity.                           *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       boolean:                                                                                   *
*          collisionActive - Is collision active.                                                  *
*          displayingSprings - Are the springs being displayed.                                    *
*          displayingVertices - Are the vertices being displayed.                                  *
*          lockParticles - Is particle lock on.                                                    *
*          running - Is the simulation running.                                                    *
*          useDamping - Is damping on.                                                             *
*          useGravity - Is gravity on.                                                             *
*       JavaGL:                                                                                    *
*          handle - Handle to the OpenGL canvas class - which communicates with the c side.        *
*       JPanel:                                                                                    *
*          cameraControls - Result of CameraPanel.CreateSubPanel().                                *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       DisplayPanel( JavaGL, CameraPanel )                                                        *
*          - Calls CreatePanel().                                                                  *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       boolean LoadSettings( String )                                                             *
*          - Loads values from a file into the corresponding variables.                            *
*       boolean SaveSettings( PrintWriter )                                                        *
*          - Writes the current values to the PrintWriter.                                         *
*       void updateButton( JButton, boolean )                                                      *
*          - Updates the given button, and the button's boolean, to the correct value.             *
*       void actionPerformed( ActionEvent )                                                        *
*          - Handles all Button events.                                                            *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       void CreatePanel( CameraPanel )                                                            *
*          - Creates the Display Panel calling CameraPanel.CreateSubPanel().                       *
*       boolean ParseLine( String, int )                                                           *
*          - Called by LoadSettings() to load the correct value into the correct variable.         *
*                                                                                                  *
***************************************************************************************************/

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;

public class DisplayPanel extends JPanel implements ActionListener
{
	public JButton resetButton, runningButton, lockParticlesButton, particleClearButton, collisionActiveButton;
	public JButton useGravityButton, useDampingButton, displaySpringsButton, displayVerticesButton;
	private JavaGL handle;
	private JPanel cameraControls;
	private boolean running, lockParticles, collisionActive, useGravity, useDamping, displayingSprings, displayingVertices;
	
	public DisplayPanel( JavaGL javaGL, CameraPanel cameraPanel )
	{
		handle = javaGL;
		CreatePanel( cameraPanel );
	} // END OF - public DisplayPanel( JavaGL, CameraPanel )
	
	public boolean LoadSettings( String filename )
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
			
			while( line != null && !line.equals( "**Display Panel" ) )
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
	} // END OF - public boolean LoadSettings( String )
	
	public boolean SaveSettings( PrintWriter writer )
	{
		if( writer == null ) return false;
		writer.println( "" );
		writer.println( "**Display Panel" );
		if( lockParticles ) writer.println( "1" ); else writer.println( "0" );
		if( collisionActive ) writer.println( "1" ); else writer.println( "0" );
		if( useGravity ) writer.println( "1" ); else writer.println( "0" );
		if( useDamping ) writer.println( "1" ); else writer.println( "0" );
		if( displayingSprings ) writer.println( "1" ); else writer.println( "0" );
		if( displayingVertices ) writer.println( "1" ); else writer.println( "0" );
		writer.println( "***" );
		writer.println( "" );
		return true;
	} // END OF - public boolean SaveSettings( PrintWriter )
	
	public void updateButton( JButton update, boolean flag )
	{
		if( flag )
		{
			update.setText( "On" );
			update.setForeground( Color.BLACK );
		}
		else
		{
			update.setText( "Off" );
			update.setForeground( Color.RED );
		}
	} // END OF - public void updateButton( JButton, boolean )
	
	public void actionPerformed( ActionEvent event )
	{
		if( event.getSource() == resetButton )
		{
			running = false;
			runningButton.setText( "Stopped" );
			runningButton.setForeground( Color.RED );
			handle.performAction( JavaGL.PERFORM_ACTION_SET_RUNNING, JavaGL.PERFORM_ACTION_FALSE );
			handle.performAction( JavaGL.PERFORM_ACTION_RESET_WORLD, JavaGL.PERFORM_ACTION_FALSE );
		}
		else if( event.getSource() == runningButton )
		{
			running = !running;
			if( running )
			{
				runningButton.setText( "Running" );
				runningButton.setForeground( Color.BLACK );
				handle.performAction( JavaGL.PERFORM_ACTION_SET_RUNNING, JavaGL.PERFORM_ACTION_TRUE );
			}
			else
			{
				runningButton.setText( "Stopped" );
				runningButton.setForeground( Color.RED );
				handle.performAction( JavaGL.PERFORM_ACTION_SET_RUNNING, JavaGL.PERFORM_ACTION_FALSE );
			}
		}
		else if( event.getSource() == lockParticlesButton )
		{
			lockParticles = !lockParticles;
			updateButton( lockParticlesButton, lockParticles );
			if( lockParticles ){ handle.performAction( JavaGL.PERFORM_ACTION_LOCK_PARTICLES, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_LOCK_PARTICLES, JavaGL.PERFORM_ACTION_FALSE ); }
		}
		else if( event.getSource() == particleClearButton )
		{
			handle.performAction( JavaGL.PERFORM_ACTION_CLEAR_PARTICLE_LOCK, 0 );
		}
		else if( event.getSource() == collisionActiveButton )
		{
			collisionActive = !collisionActive;
			updateButton( collisionActiveButton, collisionActive );
			if( collisionActive ){ handle.performAction( JavaGL.PERFORM_ACTION_SET_COLLISION_ACTIVE, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_SET_COLLISION_ACTIVE, JavaGL.PERFORM_ACTION_FALSE ); }
		}
		else if( event.getSource() == useGravityButton )
		{
			useGravity = !useGravity;
			updateButton( useGravityButton, useGravity );
			if( useGravity ){ handle.performAction( JavaGL.PERFORM_ACTION_SET_USE_GRAVITY, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_SET_USE_GRAVITY, JavaGL.PERFORM_ACTION_FALSE ); }
		}
		else if( event.getSource() == useDampingButton )
		{
			useDamping = !useDamping;
			updateButton( useDampingButton, useDamping );
			if( useDamping ){ handle.performAction( JavaGL.PERFORM_ACTION_SET_USE_DAMPING, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_SET_USE_DAMPING, JavaGL.PERFORM_ACTION_FALSE ); }
		}
		else if( event.getSource() == displaySpringsButton )
		{
			displayingSprings = !displayingSprings;
			updateButton( displaySpringsButton, displayingSprings );
			if( displayingSprings ){ handle.performAction( JavaGL.PERFORM_ACTION_DISPLAY_SPRINGS, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_DISPLAY_SPRINGS, JavaGL.PERFORM_ACTION_FALSE ); }
		}
		else if( event.getSource() == displayVerticesButton )
		{
			displayingVertices = !displayingVertices;
			updateButton( displayVerticesButton, displayingVertices );
			if( displayingVertices ){ handle.performAction( JavaGL.PERFORM_ACTION_DISPLAY_VERTICES, JavaGL.PERFORM_ACTION_TRUE ); }
			else{ handle.performAction( JavaGL.PERFORM_ACTION_DISPLAY_VERTICES, JavaGL.PERFORM_ACTION_FALSE ); }
		}
	} // END OF - public void actionPerformed( ActionEvent )
	
	public void CreatePanel( CameraPanel cameraPanel )
	{
		cameraControls = cameraPanel.CreateSubPanel();
		
		running = false;
		lockParticles = true;
//		collisionActive = true;
		collisionActive = false;
		useGravity = true;
		useDamping = true;
		displayingSprings = true;
		displayingVertices = true;
		
		resetButton = new JButton( "Reset Simulation" );
		runningButton = new JButton( "Stopped" );
		lockParticlesButton = new JButton( "On" );
		particleClearButton = new JButton( "Clear" );
//		collisionActiveButton = new JButton( "On" );
		collisionActiveButton = new JButton( "Off" );
		useGravityButton = new JButton( "On" );
		useDampingButton = new JButton( "On" );
		displaySpringsButton = new JButton( "On" );
		displayVerticesButton = new JButton( "On" );
		
		runningButton.setForeground( Color.RED );
		lockParticlesButton.setForeground( Color.BLACK );
//		collisionActiveButton.setForeground( Color.BLACK );
		collisionActiveButton.setForeground( Color.RED );
		useGravityButton.setForeground( Color.BLACK );
		useDampingButton.setForeground( Color.BLACK );
		displaySpringsButton.setForeground( Color.BLACK );
		displayVerticesButton.setForeground( Color.BLACK );
		
		resetButton.addActionListener( this );
		runningButton.addActionListener( this );
		lockParticlesButton.addActionListener( this );
		particleClearButton.addActionListener( this );
		collisionActiveButton.addActionListener( this );
		useGravityButton.addActionListener( this );
		useDampingButton.addActionListener( this );
		displaySpringsButton.addActionListener( this );
		displayVerticesButton.addActionListener( this );
		
		JPanel controlPanel = new JPanel( new BorderLayout() );
		controlPanel.add( resetButton, BorderLayout.NORTH );
		
		JPanel center = new JPanel( new GridLayout( 8, 2 ) );
		center.add( new JLabel( "Simulation is: " ) );
		center.add( runningButton );
		center.add( new JLabel( "Particle Lock is: " ) );
		center.add( lockParticlesButton );
		center.add( new JLabel( "Locked Particles: " ) );
		center.add( particleClearButton );
		center.add( new JLabel( "Collision is: " ) );
		center.add( collisionActiveButton );
		center.add( new JLabel( "Gravity is: " ) );
		center.add( useGravityButton );
		center.add( new JLabel( "Damping is: " ) );
		center.add( useDampingButton );
		center.add( new JLabel( "Display Springs: " ) );
		center.add( displaySpringsButton );
		center.add( new JLabel( "Display Vertices: " ) );
		center.add( displayVerticesButton );
		controlPanel.add( center, BorderLayout.CENTER );
		
		add( controlPanel );
		add( cameraControls );
	} // END OF -  private void CreatePanel( CameraPanel )
	
	private boolean ParseLine( String line, int number )
	{
		boolean temp = true;
		if( line.equals( "0" ) ) temp = false;
		
		switch( number )
		{
			case 0:{ lockParticles = !temp; lockParticlesButton.doClick(); } break;
			case 1:{ collisionActive = !temp; collisionActiveButton.doClick(); } break;
			case 2:{ useGravity = !temp; useGravityButton.doClick(); } break;
			case 3:{ useDamping = !temp; useDampingButton.doClick(); } break;
			case 4:{ displayingSprings = !temp; displaySpringsButton.doClick(); } break;
			case 5:{ displayingVertices = !temp; displayVerticesButton.doClick(); } break;
		}
		return true;
	} // END OF - private boolean ParseLine( String, int )
} // END OF - public class DisplayPanel extends JPanel implements ActionListener
