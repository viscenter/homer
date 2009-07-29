/***************************************************************************************************
*                                                                                                  *
*                                    Scroll Manipulation Toolkit                                   *
*                                                                                                  *
****************************************************************************************************
*                                                                                                  *
*                                      Author: Daniel G Olson                                      *
*                                      Email: dgolso0@uky.edu                                      *
*                                   Completed - August 10th, 2003                                  *
*                                                                                                  *
*    Filename: "MeshFilter.java"                                                                   *
*                                                                                                  *
*    Purpose:                                                                                      *
*             This class is a file filter for Mesh files.                                          *
*                                                                                                  *
*    PUBLIC VARIABLES:                                                                             *
*       None                                                                                       *
*                                                                                                  *
*    PROTECTED VARIABLES:                                                                          *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE VARIABLES:                                                                            *
*       None                                                                                       *
*                                                                                                  *
*    CONSTRUCTOR FUNTIONS:                                                                         *
*       None                                                                                       *
*                                                                                                  *
*    PUBLIC FUNTIONS:                                                                              *
*       boolean accept( File )                                                                     *
*          - Returns true for ".surf" and ".quad" files.                                           *
*       String getDescription()                                                                    *
*          - Return "Mesh Files".                                                                  *
*       static String getExtension( File )                                                         *
*          - Returns the extension of a file - includes the "."                                    *
*    PROTECTED FUNTIONS:                                                                           *
*       None                                                                                       *
*                                                                                                  *
*    PRIVATE FUNTIONS:                                                                             *
*       None                                                                                       *
*                                                                                                  *
***************************************************************************************************/

import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;

public class MeshFilter extends FileFilter
{
	public boolean accept( File file )
	{
        	if( file.isDirectory() )
		{
            	return false;
		}
		
		String extension = getExtension( file );
		
		if( extension != null )
		{
			if( extension.equals( ".surf" ) || extension.equals( ".quad" ) )
			{
				return true;
			}
			else
			{
				return false;
			}
        	}
		return false;
	} // END OF - public boolean accept( File )
	
	public String getDescription()
	{
        	return "SMT Mesh Files";
	} // END OF - public String getDescription()
	
	public static String getExtension( File file )
	{
		try
		{
			String path = file.getAbsolutePath();
			int lastPeriod = path.lastIndexOf( "." );
			return path.substring( lastPeriod );
		}
		catch( NullPointerException exception ){}
		catch( IndexOutOfBoundsException exception ){}
		return null;
	} // END OF - public static String getExtension( File )
} // END OF - public class MeshFilter extends FileFilter
