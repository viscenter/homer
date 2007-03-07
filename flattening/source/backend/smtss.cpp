#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jpeglib.h>
#include "smtss.h"

unsigned char *flipdata( unsigned char *data, int width, int height )
{
	unsigned char *temp = data;
	temp = (unsigned char *)malloc(sizeof(unsigned char) * width * height * 3 );
	int i, j, tempHeight = height;
	
	for( i = 0; i < height; i++ )
	{
		tempHeight--;
		for( j = 0; j < width * 3; j++ )
		{
			temp[(tempHeight*width*3)+j] = data[(i*width*3)+j];
		}
	}
	free( data );
	return temp;
}

int write_jpeg( char *filename, unsigned char *data, int width, int height, int quality )
{
	data = flipdata( data, width, height );
	
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *output_file;
	JDIMENSION num_scanlines;
	JSAMPROW row_pointer[1];
	
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress( &cinfo );
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	
	jpeg_set_defaults( &cinfo );
	jpeg_set_quality( &cinfo, quality, TRUE );
	if( (output_file = fopen( filename, "wb" ) ) == NULL) return 1;
	jpeg_default_colorspace( &cinfo );
	jpeg_stdio_dest( &cinfo, output_file );
	jpeg_start_compress( &cinfo, TRUE );
	
	int row_stride = width * 3;
	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer[0] = &data[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	fclose( output_file );
	free( data );
	return 0;
}

int Screenshot_JPEG( char filename[], int width, int height, int quality )
{
	unsigned char *data = (unsigned char*) malloc( sizeof(unsigned char) * width * height * 3 );
	int i;
	for( i = 0; i < width * height * 3; i++ ) data[i] = 0;
	
	glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );
	
	int result = write_jpeg( filename, data, width, height, quality );
	
	return result;
}

