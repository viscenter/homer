/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                   Completed - August 10th, 2003                                  *
*                                 Last Modified - October 14th, 2003                               *
*                                                                                                  *
*    Filename: "PropertiesPanel.java"                                                              *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class manages the Properties Panel.  The properties panel is designed           *
*          to let the user change the current simulation properties.                               *
*                                                                                                  *
*    Modifications:                                                                                *
*             Added buttons for adjusting intergration type.                                       *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       None                                                                                       *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       boolean:                                                                                   *
*          xAxisLocked - true is particles can not move in the x axis.                             *
*          yAxisLocked - true is particles can not move in the y axis.                             *
*          zAxisLocked - true is particles can not move in the z axis.                             *
*       float:                                                                                     *
*          coefRest[] - Current, minimum, and maximum values for coefRest.                         *
*          damping[] - Current, minimum, and maximum values for damping.                           *
*          gravX[] - Current, minimum, and maximum values for gravX.                               *
*          gravY[] - Current, minimum, and maximum values for gravY.                               *
*          gravZ[] - Current, minimum, and maximum values for gravZ.                               *
*          springConst[] - Current, minimum, and maximum values for springConst.                   *
*          springDamp[] - Current, minimum, and maximum values for springDamp.                     *
*          userForceMag[] - Current, minimum, and maximum values for userForceMag.                 *
*       JavaGL:                                                                                    *
*          handle - Handle to the OpenGL canvas class - which communicates with the c side.        *
*       JButton:                                                                                   *
*          useXAxisButton - Used to Lock/Unlock the X Axis.                                        *
*          useYAxisButton - Used to Lock/Unlock the Y Axis.                                        *
*          useZAxisButton - Used to Lock/Unlock the Z Axis.                                        *
*          commitButton - Used to send JFormattedTextField values to the c side.                   *
*          resetButton - Used to reset the JFormattedTextField values.                             *
*       JFormattedTextField:                                                                       *
*          coefRestField - Text Field for setting coefRest.                                        *
*          dampingField - Text Field for setting damping.                                          *
*          gravityXField - Text Field for setting gravX.                                           *
*          gravityYField - Text Field for setting gravY.                                           *
*          gravityZField - Text Field for setting gravZ.                                           *
*          springConstField - Text Field for setting springConst.                                  *
*          springDampField - Text Field for setting springDamp.                                    *
*          userForceMagField - Text Field for setting userForceMag.                                *
*       JRadioButton:                                                                              *
*          eulerRadioButton - Indicates that Euler intergration is being used.                     *
*          midpointRadioButton - Indicates that Midpoint intergration is being used.               *
*          rungeKuttaRadioButton - Indicates that Runge Kutta 4 intergration is being used.        *
*       int:                                                                                       *
*          integratorType - Indicates which intergration type is being used.                       *
*             - positive = midpoint, zero = euler, negative = rungeKutta4.                         *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       PropertiesPanel( JavaGL )                                                                  *
*          - Calls CreatePanel() sending "settings.smt" as the settings file.                      *
*       PropertiesPanel( JavaGL, String )                                                          *
*          - Calls CreatePanel() sending String as the settings file.                              *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       boolean SaveSettings( PrintWriter )                                                        *
*          - Writes the current values to the PrintWriter.                                         *
*       void actionPerformed( ActionEvent )                                                        *
*          - Commits values when commit is pressed, and calls reset() when reset is pressed.       *
*       void propertyChange( PropertyChangeEvent )                                                 *
*          - Used to update the JFormattedTextFields.                                              *
*       void updateButton( JButton, boolean )                                                      *
*          - Used to toggle axis locked buttons                                                    *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       void CreatePanel( String )                                                                 *
*          - Creates the Properties Panel.                                                         *
*       void LoadDefaults()                                                                        *
*          - Loads predefined default values.                                                      *
*       boolean LoadSettings( String )                                                             *
*          - Loads values from a file into the corresponding variables.                            *
*       boolean ParseLine( String, int )                                                           *
*          - Called by LoadSettings() to load the correct value into the correct variable.         *
*       void reset()                                                                               *
*          - Sets all fields back to their original values. ( Startup values )                     *
*                                                                                                  *
***************************************************************************************************/

import java.awt.*;
import java.awt.event.*;
import java.text.*;
import java.lang.Number;
import java.io.*;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;
import javax.swing.*;
import javax.swing.text.*;

public class PropertiesPanel extends JPanel implements ActionListener, PropertyChangeListener
{
        private boolean xAxisLocked, yAxisLocked, zAxisLocked;
	private JavaGL handle;
	private JButton useXAxisButton, useYAxisButton, useZAxisButton, commitButton, resetButton;
	private JFormattedTextField coefRestField, dampingField, gravityXField, gravityYField,  gravityZField;
	private JFormattedTextField springConstField, springDampField, userForceMagField;
	private JRadioButton eulerRadioButton, midpointRadioButton, rungeKuttaRadioButton;
	private float coefRest[], damping[], gravX[], gravY[], gravZ[], springConst[], springDamp[], userForceMag[];
	private int integratorType;
	
	public PropertiesPanel( JavaGL javaGL )
	{
		handle = javaGL;
		CreatePanel( "settings.smt" );
	} // END OF - public PropertiesPanel( JavaGL )
	
	public PropertiesPanel( JavaGL javaGL, String filename )
	{
		handle = javaGL;
		CreatePanel( filename );
	} // END OF - public PropertiesPanel( JavaGL, String )
	
	public boolean SaveSettings( PrintWriter writer )
	{
		if( writer == null ) return false;
		writer.println( "" );
		writer.println( "**Properties Panel" );
		writer.println( "" + coefRest[1] + " " + coefRest[2] + " " + coefRest[0] );
		writer.println( "" + damping[1] + " " + damping[2] + " " + damping[0] );
		writer.println( "" + gravX[1] + " " + gravX[2] + " " + gravX[0] );
		writer.println( "" + gravY[1] + " " + gravY[2] + " " + gravY[0] );
		writer.println( "" + gravZ[1] + " " + gravZ[2] + " " + gravZ[0] );
		writer.println( "" + springConst[1] + " " + springConst[2] + " " + springConst[0] );
		writer.println( "" + springDamp[1] + " " + springDamp[2] + " " + springDamp[0] );
		writer.println( "" + userForceMag[1] + " " + userForceMag[2] + " " + userForceMag[0] );
		writer.println( "" + integratorType );
                if( xAxisLocked ) writer.println( "1" ); else writer.println( "0" );
                if( yAxisLocked ) writer.println( "1" ); else writer.println( "0" );
                if( zAxisLocked ) writer.println( "1" ); else writer.println( "0" );
                writer.println( "***" );
		writer.println( "" );
		return true;
	} // END OF - public boolean SaveSettings( PrintWriter )
        
	public void updateButton( JButton update, boolean flag )
	{
		if( flag )
		{
			update.setText( "YES" );
			update.setForeground( Color.RED );
		}
		else
		{
			update.setText( "NO" );
			update.setForeground( Color.BLACK );
		}
	} // END OF - public void updateButton( JButton, boolean )
	
	public void actionPerformed( ActionEvent event )
	{
		Object source = event.getSource();
		
		if( source == commitButton )
		{
			Number value = (Number)coefRestField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_COEF_REST, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)dampingField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_DAMPING, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)gravityXField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_X, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)gravityYField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_Y, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)gravityZField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_Z, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)springConstField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_SPRING_CONSTANT, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)springDampField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_SPRING_DAMPING, (int)( value.floatValue() * 1000.0 ) );
			value = (Number)userForceMagField.getValue();
			handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_USER_FORCE_MAG, (int)( value.floatValue() ) );
			handle.performAction( JavaGL.PERFORM_ACTION_COMMIT_SIM_PROPERTIES, 0 );
			commitButton.setText( "Commit" );
		}
		else if( source == resetButton )
		{
			reset();
		}
		else if( event.getSource() == eulerRadioButton )
		{
			integratorType = 0;
			handle.performAction( JavaGL.PERFORM_ACTION_SET_INTEGRATOR_TYPE, integratorType );
		}
		else if( event.getSource() == midpointRadioButton )
		{
			integratorType = 1;
			handle.performAction( JavaGL.PERFORM_ACTION_SET_INTEGRATOR_TYPE, integratorType );
		}
		else if( event.getSource() == rungeKuttaRadioButton )
		{
			integratorType = -1;
			handle.performAction( JavaGL.PERFORM_ACTION_SET_INTEGRATOR_TYPE, integratorType );
		}
		else if( event.getSource() == useXAxisButton )
		{
			xAxisLocked = !xAxisLocked;
			if( !xAxisLocked ) handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_X_AXIS, JavaGL.PERFORM_ACTION_TRUE );
                        else handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_X_AXIS, JavaGL.PERFORM_ACTION_FALSE );
                        updateButton( useXAxisButton, xAxisLocked );
		}
		else if( event.getSource() == useYAxisButton )
		{
			yAxisLocked = !yAxisLocked;
			if( !yAxisLocked ) handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_Y_AXIS, JavaGL.PERFORM_ACTION_TRUE );
                        else handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_Y_AXIS, JavaGL.PERFORM_ACTION_FALSE );
                        updateButton( useYAxisButton, yAxisLocked );
		}
		else if( event.getSource() == useZAxisButton )
		{
			zAxisLocked = !zAxisLocked;
			if( !zAxisLocked ) handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_Z_AXIS, JavaGL.PERFORM_ACTION_TRUE );
                        else handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_LOCK_Z_AXIS, JavaGL.PERFORM_ACTION_FALSE );
                        updateButton( useZAxisButton, zAxisLocked );
		}
		else
		{
			try
			{
				JFormattedTextField temp = (JFormattedTextField)source;
				temp.commitEdit();
			}
			catch( ParseException exception ){}
		}
	} // END OF - public void actionPerformed( ActionEvent )
	
	public void propertyChange( PropertyChangeEvent event )
	{
		float value, min, max;
		if( "value".equals( event.getPropertyName() ) )
		{
			Object source = event.getSource();
			
			if( source == coefRestField )
			{
				value = ((Number)coefRestField.getValue()).floatValue();
				if( value > coefRest[2] ) coefRestField.setValue( new Float( coefRest[2] ) );
				else if( value < coefRest[1] ) coefRestField.setValue( new Float( coefRest[1] ) );
			}
			else if( source == dampingField )
			{
				value = ((Number)dampingField.getValue()).floatValue();
				if( value > damping[2] ) dampingField.setValue( new Float( damping[2] ) );
				else if( value < damping[1] ) dampingField.setValue( new Float( damping[1] ) );
			}
			else if( source == gravityXField )
			{
				value = ((Number)gravityXField.getValue()).floatValue();
				if( value > gravX[2] ) gravityXField.setValue( new Float( gravX[2] ) );
				else if( value < gravX[1] ) gravityXField.setValue( new Float( gravX[1] ) );
			}
			else if( source == gravityYField )
			{
				value = ((Number)gravityYField.getValue()).floatValue();
				if( value > gravY[2] ) gravityYField.setValue( new Float( gravY[2] ) );
				else if( value < gravY[1] ) gravityYField.setValue( new Float( gravY[1] ) );
			}
			else if( source == gravityZField )
			{
				value = ((Number)gravityZField.getValue()).floatValue();
				if( value > gravZ[2] ) gravityZField.setValue( new Float( gravZ[2] ) );
				else if( value < gravZ[1] ) gravityZField.setValue( new Float( gravZ[1] ) );
			}
			else if( source == springConstField )
			{
				value = ((Number)springConstField.getValue()).floatValue();
				if( value > springConst[2] ) springConstField.setValue( new Float( springConst[2] ) );
				else if( value < springConst[1] ) springConstField.setValue( new Float( springConst[1] ) );
			}
			else if( source == springDampField )
			{
				value = ((Number)springDampField.getValue()).floatValue();
				if( value > springDamp[2] ) springDampField.setValue( new Float( springDamp[2] ) );
				else if( value < springDamp[1] ) springDampField.setValue( new Float( springDamp[1] ) );
			}
			else if( source == userForceMagField )
			{
				value = ((Number)userForceMagField.getValue()).floatValue();
				if( value > userForceMag[2] ) userForceMagField.setValue( new Float( userForceMag[2] ) );
				else if( value < userForceMag[1] ) userForceMagField.setValue( new Float( userForceMag[1] ) );
			}
			commitButton.setText( "(Commit)" ); 
		}
	} // END OF - public void propertyChange( PropertyChangeEvent )
	
	private void CreatePanel( String filename )
	{
		coefRest = new float[3];
		damping = new float[3];
		gravX = new float[3];
		gravY = new float[3];
		gravZ = new float[3];
		springConst = new float[3];
		springDamp = new float[3];
		userForceMag = new float[3];
		integratorType = -1;
		
		commitButton = new JButton( "Commit" );
		resetButton = new JButton( "Reset" );
		
		commitButton.addActionListener( this );
		resetButton.addActionListener( this );
		
		if( !LoadSettings( filename ) && !LoadSettings( "settings.smt" ) )
		{
			// If unable to load the default settings file, then create it.
			LoadDefaults();
			
			try
			{
				PrintWriter writer = new PrintWriter( new BufferedWriter( new FileWriter( "settings.smt" )));
				
				writer.println( "*SMT Settings" );
				writer.println( "" );
				writer.println( "" );
				
				if( !SaveSettings( writer ) && handle.Debug() )
					System.out.println( "Unable to write: \"" + filename + "\"" );
				
				writer.close();
			}
			catch( IOException exception )
			{
				if( handle.Debug() ) System.out.println( "Unable to write: \"settings.smt\"" );
			}
		}
		
		// Send defaults to physics enivroment
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_COEF_REST, (int)( coefRest[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_DAMPING, (int)( damping[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_X, (int)( gravX[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_Y, (int)( gravY[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_GRAVITY_Z, (int)( gravZ[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_SPRING_CONSTANT, (int)( springConst[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_SPRING_DAMPING, (int)( springDamp[0] * 1000.0 ) );
		handle.performAction( JavaGL.PERFORM_ACTION_ADJUST_USER_FORCE_MAG, (int)( userForceMag[0] ) );
		
		NumberFormat threePlaces = NumberFormat.getNumberInstance();
		threePlaces.setMinimumFractionDigits(3);
		threePlaces.setMaximumFractionDigits(3);
		
		NumberFormat zeroPlaces = NumberFormat.getNumberInstance();
		zeroPlaces.setMinimumFractionDigits(0);
		zeroPlaces.setMaximumFractionDigits(0);
		
		coefRestField = new JFormattedTextField( threePlaces );
		dampingField = new JFormattedTextField( threePlaces );
		gravityXField = new JFormattedTextField( threePlaces );
		gravityYField = new JFormattedTextField( threePlaces );
		gravityZField = new JFormattedTextField( threePlaces );
		springConstField = new JFormattedTextField( threePlaces );
		springDampField = new JFormattedTextField( threePlaces );
		userForceMagField = new JFormattedTextField( zeroPlaces );
		
		reset(); // (re)set all the text field values
		
		coefRestField.setHorizontalAlignment( JTextField.RIGHT );
		dampingField.setHorizontalAlignment( JTextField.RIGHT );
		gravityXField.setHorizontalAlignment( JTextField.RIGHT );
		gravityYField.setHorizontalAlignment( JTextField.RIGHT );
		gravityZField.setHorizontalAlignment( JTextField.RIGHT );
		springConstField.setHorizontalAlignment( JTextField.RIGHT );
		springDampField.setHorizontalAlignment( JTextField.RIGHT );
		userForceMagField.setHorizontalAlignment( JTextField.RIGHT );
		
		coefRestField.addPropertyChangeListener( this );
		dampingField.addPropertyChangeListener( this );
		gravityXField.addPropertyChangeListener( this );
		gravityYField.addPropertyChangeListener( this );
		gravityZField.addPropertyChangeListener( this );
		springConstField.addPropertyChangeListener( this );
		springDampField.addPropertyChangeListener( this );
		userForceMagField.addPropertyChangeListener( this );
		
                useXAxisButton = new JButton( "NO" );
                useYAxisButton = new JButton( "NO" );
                useZAxisButton = new JButton( "NO" );
                
		coefRestField.addActionListener( this );
		dampingField.addActionListener( this );
		gravityXField.addActionListener( this );
		gravityYField.addActionListener( this );
		gravityZField.addActionListener( this );
		springConstField.addActionListener( this );
		springDampField.addActionListener( this );
		userForceMagField.addActionListener( this );
                useXAxisButton.addActionListener( this );
                useYAxisButton.addActionListener( this );
                useZAxisButton.addActionListener( this );
		
		coefRestField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		dampingField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		gravityXField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		gravityYField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		gravityZField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		springConstField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		springDampField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		userForceMagField.setFocusLostBehavior( JFormattedTextField.COMMIT_OR_REVERT );
		
		ButtonGroup integratorButtonGroup = new ButtonGroup();
		
		eulerRadioButton = new JRadioButton( "Euler", false );
		midpointRadioButton = new JRadioButton( "Midpoint", false );
		rungeKuttaRadioButton = new JRadioButton( "Runge-Kutta4 ", true );
		eulerRadioButton.addActionListener( this );
		midpointRadioButton.addActionListener( this );
		rungeKuttaRadioButton.addActionListener( this );
		
		integratorButtonGroup.add( eulerRadioButton );
		integratorButtonGroup.add( midpointRadioButton );
		integratorButtonGroup.add( rungeKuttaRadioButton );
		
		JPanel controlPanel = new JPanel( new GridLayout( 9, 2 ) );
		controlPanel.add( new JLabel( "Coef Rest " ) );
		controlPanel.add( coefRestField );
		controlPanel.add( new JLabel( "Damping " ) );
		controlPanel.add( dampingField );
		controlPanel.add( new JLabel( "Gravity X " ) );
		controlPanel.add( gravityXField );
		controlPanel.add( new JLabel( "Gravity Y " ) );
		controlPanel.add( gravityYField );
		controlPanel.add( new JLabel( "Gravity Z " ) );
		controlPanel.add( gravityZField );
		controlPanel.add( new JLabel( "Spring Constant " ) );
		controlPanel.add( springConstField );
		controlPanel.add( new JLabel( "Spring Damping " ) );
		controlPanel.add( springDampField );
		controlPanel.add( new JLabel( "User Force Mag " ) );
		controlPanel.add( userForceMagField );
		controlPanel.add( commitButton );
		controlPanel.add( resetButton );
		
                JPanel integratorPanel = new JPanel( new GridLayout( 2, 1 ) );
		JPanel integratorSubPanel = new JPanel( new GridLayout( 3, 1 ) );
		integratorSubPanel.add( new JLabel( "Select Integrator Type" ) );
		JPanel integratorSubPanel2 = new JPanel( new GridLayout( 1, 2 ) );
		integratorSubPanel2.add( eulerRadioButton );
		integratorSubPanel2.add( midpointRadioButton );
		integratorSubPanel.add( integratorSubPanel2 );
		integratorSubPanel.add( rungeKuttaRadioButton );
                integratorPanel.add( integratorSubPanel );
                JPanel integratorSubPanel3 = new JPanel( new GridLayout( 3, 2 ) );
                integratorSubPanel3.add( new JLabel( "Lock X Axis: " ) );
                integratorSubPanel3.add( useXAxisButton );
                integratorSubPanel3.add( new JLabel( "Lock Y Axis: " ) );
                integratorSubPanel3.add( useYAxisButton );
                integratorSubPanel3.add( new JLabel( "Lock Z Axis: " ) );
                integratorSubPanel3.add( useZAxisButton );
                integratorPanel.add( integratorSubPanel3 );
                add( integratorPanel, BorderLayout.SOUTH );
		
		add( controlPanel, BorderLayout.NORTH );
	} // END OF - private void CreatePanel( String )
	
	private void LoadDefaults()
	{
		coefRest[0] = 0.1f;
		coefRest[1] = -10.0f; // Not Adjusted
		coefRest[2] = 10.0f; // Not Adjusted
		damping[0] = 0.04f;
		damping[1] = 0.0f; // Not Negative
		damping[2] = 1.0f; // 
		gravX[0] = 0.0f;
		gravX[1] = -10.0f; // 
		gravX[2] = 10.0f; // 
		gravY[0] = -0.2f;
		gravY[1] = -10.0f; // 
		gravY[2] = 10.0f; // 
		gravZ[0] = 0.0f;
		gravZ[1] = -10.0f; // 
		gravZ[2] = 10.0f; // 
		springConst[0] = 2.5f;
		springConst[1] = 0.1f; // Greater than Zero
		springConst[2] = 4.5f;
		springDamp[0] = 0.1f;
		springDamp[1] = 0.0f; // 
		springDamp[2] = 2.5f; // 
		userForceMag[0] = 100.0f;
		userForceMag[1] = 0.0f; // 
		userForceMag[2] = 300.0f; //
	} // END OF - private void LoadDefaults()
	
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
				if( handle.Debug() ) System.out.println( "Wrong file format: \"" + filename  + "\"" );
				return false;
			}
			
			while( line != null && !line.equals( "**Properties Panel" ) )
			{
				line = inputStream.readLine();
			}
			
			line = inputStream.readLine();
			int i = 0;
			while( line != null && !line.equals( "***" ) && i < 8 )
			{
				if( !ParseLine( line, i ) )
				{
					if( handle.Debug() ) System.out.println( "Number Format Error line: \"" + line + "\" file: \"" + filename + "\"" );
					return false;
				}
				
				i++;
				line = inputStream.readLine();
			}
			
			line = inputStream.readLine();
			integratorType = Integer.parseInt( line );
			if( integratorType == 0 ){ eulerRadioButton.doClick(); }
			else if( integratorType > 0 ){ midpointRadioButton.doClick(); }
			else{ rungeKuttaRadioButton.doClick(); }
			
			line = inputStream.readLine();
			if( line.indexOf( "1" ) >= 0 ) xAxisLocked = true; else xAxisLocked = false;
			line = inputStream.readLine();
			if( line.indexOf( "1" ) >= 0 ) yAxisLocked = true; else yAxisLocked = false;
			line = inputStream.readLine();
			if( line.indexOf( "1" ) >= 0 ) zAxisLocked = true; else zAxisLocked = false;
		}
		catch( IOException exception )
		{
			if( handle.Debug() ) System.out.println( "Unable to read: \"" + filename  + "\"" );
			return false;
		}
		catch( NumberFormatException exception ){ return false; }
		catch( NullPointerException exception ){ return false; }
		
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
		int spaceIndex1, spaceIndex2;
		float min, max, current;
		String temp;
		
		try
		{
			spaceIndex1 = line.indexOf( " " );
			spaceIndex2 = line.indexOf( " ", spaceIndex1 + 1 );
			
			temp = line.substring( 0, spaceIndex1 );
			min = Float.parseFloat( temp );
			
			temp = line.substring( spaceIndex1 + 1, spaceIndex2 );
			max = Float.parseFloat( temp );
			
			temp = line.substring( spaceIndex2 + 1, line.length() );
			current = Float.parseFloat( temp );
		}
		catch( NumberFormatException exception ){ return false; }
		catch( IndexOutOfBoundsException exception ){ return false; }
		
		switch( number )
		{
			case 0:{ coefRest[0] = current; coefRest[1] = min; coefRest[2] = max; 	} break;
			case 1:{ damping[0] = current; damping[1] = min; damping[2] = max; } break;
			case 2:{ gravX[0] = current; gravX[1] = min; gravX[2] = max; } break;
			case 3:{ gravY[0] = current; gravY[1] = min; gravY[2] = max; } break;
			case 4:{ gravZ[0] = current; gravZ[1] = min; gravZ[2] = max; } break;
			case 5:{ springConst[0] = current; springConst[1] = min; springConst[2] = max; } break;
			case 6:{ springDamp[0] = current; springDamp[1] = min; springDamp[2] = max; } break;
			case 7:{ userForceMag[0] = current; userForceMag[1] = min; userForceMag[2] = max; } break;
			default: { return false; }
		}
		
		return true;
	} // END OF - private boolean ParseLine( String, int )
	
	private void reset()
	{
		coefRestField.setValue( new Float( coefRest[0] ) );
		dampingField.setValue( new Float( damping[0] ) );
		gravityXField.setValue( new Float( gravX[0] ) );
		gravityYField.setValue( new Float( gravY[0] ) );
		gravityZField.setValue( new Float( gravZ[0] ) );
		springConstField.setValue( new Float( springConst[0] ) );
		springDampField.setValue( new Float( springDamp[0] ) );
		userForceMagField.setValue( new Float( userForceMag[0] ) );
	} // END OF - private void reset()
} // END OF - public class PropertiesPanel extends JPanel implements ActionListener, PropertyChangeListener
