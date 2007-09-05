/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                  Completed - February 12th, 2003                                 *
*                                                                                                  *
*    Filename: "smt.java"                                                                          *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class handles keyboard input.  All buttons, text fields, and panels have        *
*          this KeyListener as their KeyListener.                                                  *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       None                                                                                       *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       smt:                                                                                       *
*          handle - handle to the main java class.                                                 *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       SmtKeyListener( smt )                                                                      *
*          - Sets handle to the main java class                                                    *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       void keyReleased( KeyEvent )                                                               *
*          - VOID                                                                                  *
*       void keyTyped( KeyEvent )                                                                  *
*          - VOID                                                                                  *
*       void keyPressed( KeyEvent )                                                                *
*          - Large switch that handles keyboard input.                                             *
*                                                                                                  *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       None                                                                                       *
*                                                                                                  *
***************************************************************************************************/

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.*;

public class SmtKeyListener implements KeyListener
{
    private smt handle;
    
    public SmtKeyListener( smt smtHandle )
    {
        handle = smtHandle;
    } // END OF - public SmtKeyListener( smt )
    
    public void keyReleased( KeyEvent event ){}
    public void keyTyped( KeyEvent event ){}
    
    public void keyPressed( KeyEvent event )
    {
        switch( event.getKeyCode() )
        {
    /*************** SMT Controls ***************/
            case KeyEvent.VK_C:{ handle.UpdateControlPanel( 1 ); } break;
            case KeyEvent.VK_D:{ handle.UpdateControlPanel( 2 ); } break;
            case KeyEvent.VK_P:{ handle.UpdateControlPanel( 3 ); } break;
            case KeyEvent.VK_S:{ handle.UpdateControlPanel( 4 ); } break;
            
    /*************** Display Controls ***************/
            case KeyEvent.VK_R:{ handle.displayPanel.resetButton.doClick(); } break;
            case KeyEvent.VK_O:{ handle.displayPanel.runningButton.doClick(); } break;
            case KeyEvent.VK_L:{ handle.displayPanel.lockParticlesButton.doClick(); } break;
            case KeyEvent.VK_K:{ handle.displayPanel.particleClearButton.doClick(); } break;
            case KeyEvent.VK_H:{ handle.displayPanel.collisionActiveButton.doClick(); } break;
            case KeyEvent.VK_G:{ handle.displayPanel.useGravityButton.doClick(); } break;
            case KeyEvent.VK_J:{ handle.displayPanel.useDampingButton.doClick(); } break;
            case KeyEvent.VK_Z:{ handle.displayPanel.displaySpringsButton.doClick(); } break;
            case KeyEvent.VK_X:{ handle.displayPanel.displayVerticesButton.doClick(); } break;
            
    /*************** Camera Controls ***************/
            case KeyEvent.VK_F:{ handle.cameraPanel.reset(); } break;
            case KeyEvent.VK_UP:{ handle.cameraPanel.rotate( 0, 1 ); } break;
            case KeyEvent.VK_DOWN:{ handle.cameraPanel.rotate( 0, -1 ); } break;
            case KeyEvent.VK_LEFT:{ handle.cameraPanel.rotate( -1, 0 ); } break;
            case KeyEvent.VK_RIGHT:{ handle.cameraPanel.rotate( 1, 0 ); } break;
            case KeyEvent.VK_ADD:
            case KeyEvent.VK_EQUALS:{ handle.cameraPanel.zoom( -1 ); } break;
            case KeyEvent.VK_MINUS:
            case KeyEvent.VK_UNDERSCORE:{ handle.cameraPanel.zoom( 1 ); } break;
            case KeyEvent.VK_PAGE_UP:{ handle.cameraPanel.height( 1 ); } break;
            case KeyEvent.VK_PAGE_DOWN:{ handle.cameraPanel.height( -1 ); } break;
            
    /*************** Save Controls ***************/
            case KeyEvent.VK_F9:{ handle.savePanel.screenshotButton.doClick(); } break;
            case KeyEvent.VK_F11:
            {
                if( !handle.savePanel.scriptRunning && !handle.savePanel.scriptRecording )
                { // write script file
                    handle.savePanel.scriptButton.doClick();
                }
            } break;
            case KeyEvent.VK_F12:
            {
                if( !handle.savePanel.scriptRunning && handle.savePanel.scriptRecording )
                { // stop script file
                    handle.savePanel.scriptButton.doClick();
                }
            } break;
            
            default:{}
        }
    } // END OF - public void keyPressed( KeyEvent )
} // END OF - public class SmtKeyListener implements KeyListener
