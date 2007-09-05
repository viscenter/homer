
public class Screenshot
{
	public Screenshot(){}
	
	// Connect java side to c side
	static
	{
		String name = "\\Screenshot.dll";
		String os = System.getProperty( "os.name" );
		
		// If operating system is not windows - assume linux
		if( os.charAt(0) != 'W' && os.charAt(0) != 'w' )
		{
			name = "/Screenshot.so";
		}
		
// This can be changed to reflect the location of the shared library
		String pwd = System.getProperty( "user.dir" );
		System.load( pwd + name );
	}
	
	public native int BMP( String filename, int width, int height );
	public native int JPEG( String filename, int width, int height, int quality );
}
