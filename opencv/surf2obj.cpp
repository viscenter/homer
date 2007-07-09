#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

struct Point {
	float u1, v1, x, y, z;
};

struct Triangle {
	int idx1, idx2, idx3;
};

int main( int argc, char * argv[] )
{
	int nVer, nTrig;

	if( argc < 2 ) {
		printf( "Usage: surf2obj.exe surf obj\n" );
		return 1;
	}

	char * surfname = argv[1];
	char * objname = argv[2];


	FILE * surffile = fopen( surfname, "r" );

	// Read in all the comments (lines that begin with a "#")
	char buffer[256], token[10]; 
	while( true )
	{
		fgets( buffer, 255, surffile );
		if ( buffer[0] != '#' ) break;
	}
	
	if ( buffer[0] != 'V' )
	{
		fprintf( stderr, "The mesh file %s is not correct.\n", surfname );
		return 1;
	}
	else sscanf( buffer, "%s %i \n", token, &nVer );
	
	Point * verList = (Point*)malloc( sizeof(Point)*nVer );

	for( int i=0; i < nVer; i++ )	{
		float x, y, z, u, v;
		
		fgets( buffer, 255, surffile );
		sscanf( buffer, "%f %f %f %f %f\n", &x, &y, &z, &u, &v );
		verList[i].u1 = u;
		verList[i].v1 = v;
	
		verList[i].x = x;
		verList[i].y = z;
		verList[i].z = y;
		
	}
	printf( "Read in %i Vertices \n", nVer );
	
	fgets( buffer, 255, surffile );
	if( buffer[0] != 'T' )	{
		fprintf( stderr, "The mesh file %s is not correct.\n", surfname );
		return 1;
	}
	else sscanf( buffer, "%s %i \n", token, &nTrig );
	Triangle * trigList = (Triangle*)malloc( sizeof(Triangle)*nTrig );
	for( int i = 0; i< nTrig; i++ ) {
		int idx1, idx2, idx3;
		fgets( buffer, 255, surffile );
		sscanf( buffer, "%i %i %i\n", &idx1, &idx2, &idx3 );
		trigList[i].idx1 = idx1;
		trigList[i].idx2 = idx2;
		trigList[i].idx3 = idx3;
	}
	
	printf( "Read in %i Triangles \n", nTrig );
	fclose( surffile );

	FILE * objfile = fopen( objname, "w" );

	fprintf( objfile, "# Vertices %d\n", nVer );
	for( int i = 0; i < nVer; i++ ) {
		fprintf( objfile, "v %.6f %.6f %.6f\nvt %.6f %.6f\n",
				verList[i].x,
				verList[i].y,
				verList[i].z,
				verList[i].u1,
				1.0 - verList[i].v1 );
	}

	fprintf( objfile, "# Triangles %d\n", nTrig );
	for( int i = 0; i < nTrig; i++ ) {
		fprintf( objfile, "f %d/%d %d/%d %d/%d\n",
				trigList[i].idx1, trigList[i].idx1,
				trigList[i].idx2, trigList[i].idx2,
				trigList[i].idx3, trigList[i].idx3 );
	}

	fclose( objfile );

	free( verList );
	free( trigList );

	return 0;
}
