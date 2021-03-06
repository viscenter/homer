#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
extern "C" {
#include <jpeglib.h>
}
*/
#include "smtss.h"
#include "tr.h"
#include "smc.h"

#include "cv.h"
#include "highgui.h"

extern GLint width, height;

extern GLfloat axisAngleX, axisAngleY, distance, fov;
extern GLfloat lookAtX, lookAtY, lookAtZ;


/*
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
*/

extern CPhysEnv *m_PhysEnv;

int Screenshot_TR( char filename[], int image_width, int image_height )
{
	TRcontext *tr;
	GLubyte *buffer;
	GLubyte *tile;
	FILE *f;
	int more;
	int i;
	IplImage* output_image;

	int tile_width = width;
	int tile_height = height;
	int tile_border = 10;

	bool is_ppm = false;
	int len = strlen(filename);
	if(strcasecmp(".ppm",&filename[len-4]) == 0) {
		is_ppm = true;
	}

	printf("Generating %d by %d image file...\n", image_width, image_height);

	/* allocate buffer large enough to store one tile */
	tile = (GLubyte*)malloc(tile_width * tile_height * 3 * sizeof(GLubyte));
	if (!tile) {
		printf("Malloc of tile buffer failed!\n");
		return 1;
	}

	/* allocate buffer to hold a row of tiles */
	buffer = (GLubyte*)malloc(image_width * tile_height * 3 * sizeof(GLubyte));
	if (!buffer) {
		free(tile);
		printf("Malloc of tile row buffer failed!\n");
		return 1;
	}

	/* Setup.  Each tile is tile_width x tile_height pixels. */
	tr = trNew();
	trTileSize(tr, tile_width, tile_height, tile_border);
	trTileBuffer(tr, GL_RGB, GL_UNSIGNED_BYTE, tile);
	trImageSize(tr, image_width, image_height);
	trRowOrder(tr, TR_TOP_TO_BOTTOM);

	trPerspective(tr, 60.0, (GLfloat) image_width/(GLfloat) image_height, 1.0, 2000.0);

	if(is_ppm) {
		/* Prepare ppm output file */
		f = fopen(filename, "w");
		if (!f) {
			printf("Couldn't open image file: %s\n", filename);
			return 1;
		}
		fprintf(f,"P6\n");
		fprintf(f,"# ppm-file created by %s\n", "trdemo2");
		fprintf(f,"%i %i\n", image_width, image_height);
		fprintf(f,"255\n");
		fclose(f);
		f = fopen(filename, "ab");  // now append binary data
		if (!f) {
			printf("Couldn't append to image file: %s\n", filename);
			return 1;
		}
	}
	else {
		output_image = cvCreateImage( cvSize(image_width, image_height), IPL_DEPTH_8U, 3);
	}

	/*
	* Should set GL_PACK_ALIGNMENT to 1 if the image width is not
	* a multiple of 4, but that seems to cause a bug with some NVIDIA
	* cards/drivers.
	*/
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	/* Draw tiles */
	more = 1;
	int curheight = 0;
	while (more) {
		int curColumn;
		trBeginTile(tr);
		curColumn = trGet(tr, TR_CURRENT_COLUMN);
		// RenderScene();      /* draw our stuff here */   
		glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
		// glPushMatrix();
		// glLoadIdentity();
		glPushMatrix();
	
		glLoadIdentity();	
		
		GLfloat cameraX, cameraY, cameraZ;
		 cameraX = ( sinf(axisAngleX) * ( cosf(axisAngleY) * distance ) ) + lookAtX;
		 cameraY = ( sinf(axisAngleY) * distance ) + lookAtY;
		 cameraZ = ( cosf(axisAngleX) * ( cosf(axisAngleY) * distance ) ) + lookAtZ;
		 
		 gluLookAt ( cameraX, cameraY, cameraZ, lookAtX, lookAtY, lookAtZ, 0.0, 1.0, 0.0);

		m_PhysEnv->RenderWorld();
		glPopMatrix();
		more = trEndTile(tr);

		/* save tile into tile row buffer*/
		{
			int curTileWidth = trGet(tr, TR_CURRENT_TILE_WIDTH);
			int bytesPerImageRow = image_width*3*sizeof(GLubyte);
			int bytesPerTileRow = (tile_width-2*tile_border) * 3*sizeof(GLubyte);
			int xOffset = curColumn * bytesPerTileRow;
			int bytesPerCurrentTileRow = (curTileWidth-2*tile_border)*3*sizeof(GLubyte);
			int i;
			int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
			for (i=0;i<curTileHeight;i++) {
				memcpy(buffer + i*bytesPerImageRow + xOffset, /* Dest */
				tile + i*bytesPerTileRow,              /* Src */
				bytesPerCurrentTileRow);               /* Byte count*/
			}
		}

		if (curColumn == trGet(tr, TR_COLUMNS)-1) {
			/* write this buffered row of tiles to the file */
			int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
			int bytesPerImageRow = image_width*3*sizeof(GLubyte);
			int i;
			GLubyte *rowPtr;
			unsigned char* data;
			int step;
			if(!is_ppm) {
				data = (unsigned char*)output_image->imageData;
				step = output_image->widthStep;
			}

			/* The arithmetic is a bit tricky here because of borders and
			* the up/down flip.  Thanks to Marcel Lancelle for fixing it.
			*/
			for (i=2*tile_border;i<curTileHeight;i++) {
				/* Remember, OpenGL images are bottom to top.  Have to reverse. */
				rowPtr = buffer + (curTileHeight-1-i) * bytesPerImageRow;
				if(is_ppm) {
					fwrite(rowPtr, 1, image_width*3, f);
				}
				else {
					for(int j = 0; j < image_width; j++) {
						long offset2 = (curheight * step);
						data[offset2+j*3+2] = rowPtr[j*3];
						data[offset2+j*3+1] = rowPtr[j*3+1];
						data[offset2+j*3] = rowPtr[j*3+2];
					}
					curheight++;
				}
			}
		}
	}

	trDelete(tr);

	if(is_ppm) {
		fclose(f);
	}
	else {
		cvSaveImage( filename, output_image );
		cvReleaseImage( &output_image );
	}
	printf("%s complete.\n", filename);

	free(tile);
	free(buffer);

	return 0;
}
