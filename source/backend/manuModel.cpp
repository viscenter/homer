#include <new>
#ifdef __APPLE__
#include <GL/glew.h>
#include <GLUT/glut.h>
#else
#include "GLee.h"
#include <GL/glut.h>
#endif
#include <math.h>
#include "MButils.h"
#include <string.h>
#include "manuModel.h"

#include "cv.h"
#include "highgui.h"

/*
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
*/
#include <unistd.h>

GLint TEXW;
GLint TEXH;

void errtest(const char * file, const int line, const char * fun)
{
	char * nodir = strrchr(file,'/')+1;
	GLint errval = glGetError();
	if(errval) {
		printf("Error at %s:%d\t%s\n",nodir,line,gluErrorString(errval));
	}
}

manuModel::manuModel()
{
	textureID = 3903;
	firstTexture = NULL;
	currentTexture = NULL;
	nVer = 0;
	nQuad = 0;
	nEdges = 0;
	nTrig = 0;
	verList = NULL;
	firstRun = true;
	TRIGTEXRES = 1;
	NUMTRIGPERROW = 1000;
	YL_UseTriangularTextureMap = false;

	border = 0;
	tileW = 0;
	tileH = 0;
	texArray = false;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &TEXW);
	TEXH = TEXW;
	// if( SMT_DEBUG ) printf("Max texture size is: %d x %d\n",TEXW,TEXH);
}

manuModel::~manuModel()
{
	if( verList != NULL ) delete[] verList;
	if( trigList != NULL ) delete[] trigList;
	if( originalList != NULL ) delete[] originalList;
	
	texture *next = firstTexture->nextTexture;
	delete[] firstTexture->ima;
	glDeleteTextures(1,(GLuint*)(&(firstTexture->id)));
	// delete firstTexture->subIma;
	delete firstTexture;
	while( next != NULL )
	{
		currentTexture = next;
		next = currentTexture->nextTexture;
		delete[] currentTexture->ima;
		glDeleteTextures(1,(GLuint*)(&(currentTexture->id)));
		// delete currentTexture->subIma;
		delete currentTexture;
	}
}

#ifndef max
float max( float first, float second )
{ // math.h does not having this function
	if( first > second ) return first;
	return second;
}
#endif

bool manuModel::readMesh(char *filename)
{	
	meshFile=filename; 
	FILE* fp = MBopenFile(filename, "r");
	char * extension = filename+(strlen(filename)-strlen(".obj"));
	// printf("Extension: %s\n",extension);
	if( strcmp(extension,".obj") == 0 ) { // loading an OBJ file
		fclose(fp);
		readOBJ(filename);	
	}
	else {
		if( YL_UseQuad )
		{
			//
			// Read in all the comments (lines that begin with a "#")
			//
			char buffer[256];  
			while(true)
			{
				fgets(buffer, 255, fp );
				if (buffer[0] != '#') break;
			}
			
			// Read in number of X and Y samples!
			sscanf(buffer, "%i %i \n", &xSamples, &ySamples );
			nVer = xSamples * ySamples;
			
			// Read in the points
			verList = new Point[ nVer ];
			originalList = new Point[ nVer ];
			for(int i=0; i < nVer; i++)
			{
				float x, y, z, u, v;
				int idx;
				
				fgets(buffer, 255, fp );
				sscanf(buffer, "%i %f %f %f %f %f \n", &idx, &x, &y, &z, &u, &v ); 
				verList[idx].x = x;
				verList[idx].y = y;
				verList[idx].z = z;
				verList[idx].u1 =  u;
				verList[idx].v1 =  v;
				
				//			verList[idx].v1 = imaH -  v;
				originalList[idx].u1 = (float) u;
				originalList[idx].v1 = (float) v;
			}
			
			// Adds quadList for self-collision
			fgets(buffer, 255, fp );
			if (buffer[0] == 'Q')
			{
				char token[10];
				sscanf(buffer, "%s %i \n", token, &nQuad);
				quadList = new Quad[nQuad];
				for (int i = 0; i< nQuad; i++)
				{
					int idx1, idx2, idx3, idx4;
					fgets(buffer, 255, fp );
					sscanf(buffer, "%i %i %i %i\n", &idx1, &idx2, &idx3, &idx4 ); 
					quadList[i].v1 = idx1-1;
					quadList[i].v2 = idx2-1;
					quadList[i].v3 = idx3-1;
					quadList[i].v4 = idx4-1;
				}
			
			}
			
			if( SMT_DEBUG ) printf("Read in %i Vertices \n", nVer );
			fclose(fp);
		}
		else
		{
			//
			// Read in all the comments (lines that begin with a "#")
			//
			char buffer[256], token[10]; 
			while(true)
			{
				fgets(buffer, 255, fp );
				if (buffer[0] != '#') break;
			}
			
			if (buffer[0] != 'V')
			{
				if( SMT_DEBUG ) fprintf(stderr, "The mesh file %s is not correct.\n", filename);
				return false;
			}
			else sscanf(buffer, "%s %i \n", token, &nVer);
			
			verList = new Point[nVer];
			originalList = new Point[nVer];
			for(int i=0; i < nVer; i++)
			{
				float x, y, z, u, v, w;
				
				fgets(buffer, 255, fp );
				if( YL_UseTriangularTextureMap ){
					sscanf(buffer, "%f %f %f\n", &x, &y, &z);
					verList[i].u1 = 0.0;
					verList[i].v1 = 0.0;
				}
				else{
					sscanf(buffer, "%f %f %f %f %f\n", &x, &y, &z, &u, &v);
					verList[i].u1 = u;
					verList[i].v1 = v;
				}
				
				verList[i].x = x;
				verList[i].y = y;
				verList[i].z = z;
				
			}
			if( SMT_DEBUG ) printf("Read in %i Vertices \n", nVer );
			
			fgets(buffer, 255, fp );
			if (buffer[0] != 'T')
			{
				if( SMT_DEBUG ) fprintf(stderr, "The mesh file %s is not correct.\n", filename);
				return false;
			}
			else sscanf(buffer, "%s %i \n", token, &nTrig);
			trigList = new Triangle[nTrig];
			for (int i = 0; i< nTrig; i++)
			{
				int idx1, idx2, idx3;
				fgets(buffer, 255, fp );
				sscanf(buffer, "%i %i %i\n", &idx1, &idx2, &idx3); 
				trigList[i].idx1 = idx1-1;
				trigList[i].idx2 = idx2-1;
				trigList[i].idx3 = idx3-1;
			}
			
			if( SMT_DEBUG ) printf("Read in %i Triangles \n", nTrig );
			fclose(fp);
		}
	} // end surf file reading

	//
	// !Translate points to the geometric center!
	// !Scale the points to fit in unit sphere!
	//
	float minx, miny, minz, maxx, maxy, maxz, scale = 1;
	cz = cx = cy = 0;
	for(int i=0; i < nVer; i++)
	{
		if (i==0)
		{
			minx = maxx = verList[i].x;
			miny = maxy = verList[i].y;
			minz = maxz = verList[i].z;
		}
		else
		{
			if (verList[i].x < minx) minx = verList[i].x;
			if (verList[i].y < miny) miny = verList[i].y;
			if (verList[i].z < minz) minz = verList[i].z;
			
			if (verList[i].x > maxx) maxx = verList[i].x;
			if (verList[i].y > maxy) maxy = verList[i].y;
			if (verList[i].z > maxz) maxz = verList[i].z;
		}
		
		cx += verList[i].x;
		cy += verList[i].y;
		cz += verList[i].z;
	}
	
	cx /= nVer; minx -= cx; maxx -= cx;
	cy /= nVer; miny -= cy; maxy -= cy;
	cz /= nVer; minz -= cz; maxz -= cz;
	
	float ww, hh, dd;
	/* calculate model width, height, and depth */
	ww = fabs(maxx) + fabs(minx);
	hh = fabs(maxy) + fabs(miny);
	dd = fabs(maxz) + fabs(minz);
	
	/* calculate center of the model */
	//cx = (maxx + minx) / 2.0;
	//cy = (maxy + miny) / 2.0;
	//cz = (maxz + minz) / 2.0;
	
	/* calculate unitizing scale factor */
	//scale = 12.0 / max(max(w, h), d);
	scale = 7.0 / max(max(ww, hh), dd);
	
	this->minx = 100000;
	this->miny = 100000;
	this->minz = 100000;
	this->maxx = -100000;
	this->maxy = -100000;
	this->maxz = -100000;
	for(int i=0; i < nVer; i++)
	{
		originalList[i].x = verList[i].x;
		originalList[i].y = verList[i].y;
		originalList[i].z = verList[i].z;
		originalList[i].u1 = verList[i].u1;
		originalList[i].v1 = verList[i].v1;
		
		verList[i].x -= cx;
		verList[i].y -= cy;
		verList[i].z -= cz;
		
		verList[i].x *= scale;
		verList[i].y *= scale;
		verList[i].z *= scale;
		
		if (verList[i].x > this->maxx)
			this->maxx = verList[i].x;
		if (verList[i].y > this->maxy)
			this->maxy = verList[i].y;
		if (verList[i].z > this->maxz)
			this->maxz = verList[i].z;


		if (verList[i].x < this->minx)
			this->minx = verList[i].x;
		if (verList[i].y < this->miny)
			this->miny = verList[i].y;
		if (verList[i].z < this->minz)
			this->minz = verList[i].z;
		
		// originalList[i].x = verList[i].x;
		// originalList[i].y = verList[i].y;
		// originalList[i].z = verList[i].z;
	}
	
	scaleFactor = 1.0 / scale;
	this->maxz += 0.3;
	edgeSum = edgeLengthSum();
	return true;
}

int manuModel::readPPM( char* filename, unsigned char* &image, int &width, int &height )
{
//  Open .ppm of given name from local directory
	FILE* fp = fopen(filename, "rb");
	if( fp == NULL ) return -1;
	char buffer[100], buffer2[100];
	char temp;
	int intTemp;
	int maxVal;
	int readIntIndex = 0;
	
	fscanf( fp, "%s", buffer );
	if( strncmp( buffer, "P6", 2 ) ) return -1;
	
	TRIGTEXRES = -1;
	long int offset = -1;
	while( readIntIndex < 3 )
	{
		//read in header information
		temp = fgetc( fp );
		
		if( !( temp == ' ' || temp == '\t' || temp == '\r' || temp == '\n' ) )
		{
			fseek( fp, offset, SEEK_CUR ); // Move back one
			
			if( temp == '#' )
			{
				intTemp = -1;
				fgets( buffer, sizeof(buffer)-1, fp );
				sscanf( buffer, "#%s %d\n", buffer2, intTemp );
				
				if( strcmp( buffer2, "TRIGTEXRES" ) == 0 ) TRIGTEXRES = intTemp;
			}
			else
			{
				fscanf( fp, "%d", &intTemp );
				if( readIntIndex == 0 ) width = intTemp;
				else if( readIntIndex == 1 ) height = intTemp;
				else maxVal = intTemp;
				readIntIndex++;
			}
		}
	}
	temp = fgetc( fp ); // get newline character
	
	image = new unsigned char[width*height*3];
	fread( image, width*height*3, 1, fp );
	fclose( fp );
	
	return 0;
} 

int manuModel::readImage( char* filename, unsigned char* &image, int &width, int &height )
{
	IplImage* cvImage = cvLoadImage(filename);
	unsigned char* data;
	int step;

	height = cvImage->height;
	width = cvImage->width;
	data = (unsigned char *)cvImage->imageData;
	step = cvImage->widthStep;

	TRIGTEXRES = -1;
	
	image = new unsigned char[width*height*3];
	// copy the image data over (cvImage uses widthStep etc.)
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			long offset1 = (i * width + j) * 3;
			long offset2 = (i * step);
			
			image[offset1] = data[offset2+j*3+2]; // red
			image[offset1+1] = data[offset2+j*3+1]; // green
			image[offset1+2] = data[offset2+j*3]; // blue
		}
	}
	
	cvReleaseImage(&cvImage);
	return 0;
}

void saveCompressed(char * infilename, int i, int j) {
	GLint size_in_bytes;

	//if(!glGetCompressedTexImageEXT) {
	//	printf("No compressed tex image fetch!\n");
	//}

	infilename += strlen("venetus/");

	char filename[256];
	// save out the compressed mipmapped images
	for(int level = 0; level < 1; level++) {
		sprintf(filename,"venetus/cache/%s-%d-%d-%d.cmp",infilename,i,j,level);
			
				
		if(GLEE_ARB_texture_compression) {
			glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size_in_bytes);
			GLvoid * curimg = (GLvoid *)malloc(size_in_bytes);
		
			printf("%d: Saving %d bytes to %s\n",level,size_in_bytes,filename);
			
			glGetCompressedTexImageARB(GL_TEXTURE_2D, level, curimg);
			glCompressedTexImage2DARB(GL_TEXTURE_2D, level, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4096, 4096, 0, size_in_bytes, curimg);
			errtest(__FILE__,__LINE__,__FUNCTION__);

			FILE * outfile = fopen(filename,"w");
			fwrite(curimg,1,size_in_bytes,outfile);
			fclose(outfile);
			
			free(curimg);
		}
	}
}

void manuModel::readTextureSplit(pixel *colorIma, char * filename)
{
	// pixel *colorIma;
	
	// textureFile = filename;
	// readImage( filename, colorIma, imaW, imaH );

	// as texture resolution goes up, border size should go up
	// as nVer goes up, border size should go down
	// border = (int)((((double)(imaH*imaW))/(double) nVer)/2);
	border = 512;
	// printf("border width: %d\n",border);

	if( !YL_UseTriangularTextureMap) {
		if( colorIma != NULL ){
			int border_h = TEXH-border;
			int border_w = TEXW-border;

			tileH = (int)ceil((double) (imaH-TEXH)/((double) border_h))+1;
			tileW = (int)ceil((double) (imaW-TEXW)/((double) border_w))+1;

			if(SMT_DEBUG) printf("Splitting texture into %d x %d tiles\n",tileW,tileH);

			texArray = true;

			for(int i = 0; i < tileH; i++) {
				for(int j = 0; j < tileW; j++) {
					if( firstTexture == NULL )
					{
						firstTexture = new texture;
						currentTexture = firstTexture;
					}
					else
					{
						texture *newTexture = new texture;
						currentTexture->nextTexture = newTexture;
						currentTexture = newTexture;
					}
					currentTexture->nextTexture = NULL;
					glGenTextures(1,&textureID);
					currentTexture->id = textureID;
				
					currentTexture->ima = new pixel[ TEXW * 3 * TEXH ];
					for(int h = (i*border_h); (h < (i*border_h+TEXH)) && (h < imaH); h++) {
						int w = j*border_w;
						memcpy((currentTexture->ima)+((h-(i*border_h))*TEXW*3),(colorIma)+((h*imaW+w)*3),(TEXW*3)*sizeof(pixel));
						/*	
						for(int w = (j*border_w); (w < (j*border_w+TEXW)) && (w < imaW); w++) {
							long offset1 = ((h-(i*border_h)) * TEXW + (w-(j*border_w))) * 3;
							long offset2 = (h * imaW + w) * 3;
							currentTexture->ima[offset1] = colorIma[offset2];
							currentTexture->ima[offset1+1] = colorIma[offset2+1];
							currentTexture->ima[offset1+2] = colorIma[offset2+2];
						}
						*/
					}
					currentTexture->subIma = colorIma;
					currentTexture->w = TEXW;
					currentTexture->h = TEXH;
					currentTexture->ww = imaW;
					currentTexture->hh = imaH;
					textureFormat = COLOR;
					initTexture( currentTexture );
					saveCompressed(filename,i,j);
					
				}
			}
			if( SMT_DEBUG ) printf("Read texture size %i %i \n", imaW, imaH );
		}
		else if( SMT_DEBUG ) printf("Can't split texture \n" );
	}

}

/*
void manuModel::readTextureSplit2(pixel *colorIma)
{
	int num_sizes = (int)(log((double)TEXH)/log(2.0));
	int *allowed_sizes = malloc(sizeof(int)*num_sizes);
	int *texture_check = calloc(num_sizes*num_sizes,sizeof(int));
	for(int i = 0; i < num_sizes; i++) {
		allowed_sizes[i] = (int)pow(2.0,(double)(i+1));
	}
	bool tex_memory_full;
	GLint test_width = 0, test_height = 0;


	if( !YL_UseTriangularTextureMap) {
		if( colorIma != NULL ){
			// check what texture sizes we can allocate now
			tex_memory_full = true;
			for(int i = 0; i < num_sizes; i++) {
				for(int j = 0; j < num_sizes; j++) {
					glTexImage2D(GL_PROXY_TEXTURE_2D,  0,  GL_LUMINANCE, allowed_sizes[i], allowed_sizes[j], 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
					glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &test_width);
					glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &test_height);
					if((test_width != 0) && (test_height != 0)) {
						tex_memory_full = false;
						texture_check[i * num_sizes + j] = 1;
					}
					else {
						texture_check[i * num_sizes + j] = 0;
					}
				}
			}
			// read in the max allowed texture from the image

			// 
		}
	}

	free((int*)allowed_sizes);
}
*/

void manuModel::readCachedMipmap(char * infilename)
{
	int size_in_bytes = 8388608;
	int level = 0;
	//int size_in_bytes = 131072;
	//int level = 3;
	char filename[256];
	border = 512;

	int border_h = TEXH-border;
	int border_w = TEXW-border;

	imaW = 7230;
	imaH = 5428;

	tileH = (int)ceil((double) (imaH-TEXH)/((double) border_h))+1;
	tileW = (int)ceil((double) (imaW-TEXW)/((double) border_w))+1;

	if(SMT_DEBUG) printf("Splitting texture into %d x %d tiles\n",tileW,tileH);

	texArray = true;

	for(int i = 0; i < tileH; i++) {
		for(int j = 0; j < tileW; j++) {
			sprintf(filename,"venetus/cache/%s-%d-%d-%d.cmp",infilename+strlen("venetus/"),i,j,level);
			printf("Reading cached mipmap from %s\n",filename);

			if( firstTexture == NULL )
			{
				firstTexture = new texture;
				currentTexture = firstTexture;
			}
			else
			{
				texture *newTexture = new texture;
				currentTexture->nextTexture = newTexture;
				currentTexture = newTexture;
			}
			currentTexture->nextTexture = NULL;
			glGenTextures(1,&textureID);
			currentTexture->id = textureID;
		
			currentTexture->ima = new pixel[ 1 ];
			
			currentTexture->subIma = NULL;
			currentTexture->w = TEXW;
			currentTexture->h = TEXH;
			currentTexture->ww = imaW;
			currentTexture->hh = imaH;
			textureFormat = COLOR;
		
			// load in the file
			GLvoid * data = malloc(size_in_bytes);
			FILE * infile = fopen(filename,"r");
			if(!infile) {
				printf("ERROR OPENING FILE\n");
			}
			fread(data,1,size_in_bytes,infile);
			fclose(infile);

			glBindTexture(GL_TEXTURE_2D, currentTexture->id );
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			//glTexImage2D(GL_TEXTURE_2D, level, GL_COMPRESSED_RGB_ARB, TEXW, TEXH,
			//			0, GL_RGB, GL_UNSIGNED_BYTE, data );

			if(GLEE_ARB_texture_compression) {
				glCompressedTexImage2DARB(GL_TEXTURE_2D,level,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,4096,4096,0,size_in_bytes,data);
			printf("Reading as format %d\n",GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
			// glCompressedTexImage2DARB(GL_TEXTURE_2D,level,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,512,512,0,size_in_bytes,data);
			}
			GLint errval = glGetError();
			if(errval) {
				printf("Error: %s\n",gluErrorString(errval));
			}
			free(data);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level );
		}
	}
}

void manuModel::readTexture(char *filename)
{
	/**************************************************************/

 //	int imaW, imaH;
 pixel *colorIma;

 char testfile[256];
 sprintf(testfile,"venetus/cache/%s-0-0-0.cmp",filename + strlen("venetus/"));
	
 textureFile = filename;
 if(strcmp(filename+(strlen(filename)-strlen("-hi.jpg")),"-hi.jpg") == 0) {
	 printf("Hi res file\n");
	 if(access(testfile,R_OK) == 0) {
		 printf("Using cached mipmap\n");
		 readCachedMipmap(filename);
	 }
	 else {
		 readImage( filename, colorIma, imaW, imaH );
		 readTextureSplit(colorIma,filename);
		}
 }
 else {
	 readImage( filename, colorIma, imaW, imaH );

	 if (YL_UseTriangularTextureMap){
		
		if( TRIGTEXRES < 0 || TRIGTEXRES > imaW )
		{
			TRIGTEXRES = 39;
			//TRIGTEXRES = 9;
			NUMTRIGPERROW = imaW / (TRIGTEXRES+1);
			printf( "Using TRIGTEXRES: %d, NUMTRIGPERROW: %d\n", TRIGTEXRES, NUMTRIGPERROW );
		}
		else
		{
			NUMTRIGPERROW = imaW / ( TRIGTEXRES + 1 );
		}
		numberOfTrianglesInATexture = NUMTRIGPERROW * NUMTRIGPERROW;
		int maxTextureHeight = NUMTRIGPERROW * ( TRIGTEXRES + 1 );
		
		int ppmHeightDone = 0, imaMinHeight = 0, imaMaxHeight = 0;
		
		if( colorIma != NULL )
		{
			if( SMT_DEBUG ) printf( "Copying surface into a texture structure. " );
			while( ppmHeightDone < imaH )
			{
				if( firstTexture == NULL )
				{
					firstTexture = new texture;
					currentTexture = firstTexture;
					currentTexture->nextTexture = NULL;
					currentTexture->id = textureID;
					textureID++;
				}
				else
				{
					texture *newTexture = new texture;
					currentTexture->nextTexture = newTexture;
					currentTexture = newTexture;
					currentTexture->nextTexture = NULL;
					currentTexture->id = textureID;
					textureID++;
				}
				currentTexture->ima = new pixel[ TEXW * 3 * TEXH ];
				
				imaMinHeight = imaMaxHeight;
				imaMaxHeight = imaH;
				
				if( imaMaxHeight - imaMinHeight > maxTextureHeight )
				{
					imaMaxHeight = imaMinHeight + maxTextureHeight;
				}
				
				if( SMT_DEBUG ) printf( "Image height %d to %d.\n", imaMinHeight, imaMaxHeight );
				for( int h = imaMinHeight; h < imaMaxHeight; h++ )
				{
					long offset1 = ( h - imaMinHeight ) * TEXW * 3;
					long offset2 = h * imaW * 3;
					for(int w=0; w < imaW * 3; w++)
					{
						currentTexture->ima[offset1 + w] = colorIma[offset2 + w];
					}
				}
				
				currentTexture->subIma = colorIma;
				currentTexture->w = TEXW;
				currentTexture->h = TEXH;
				currentTexture->ww = imaW;
				currentTexture->hh = imaMaxHeight - imaMinHeight;
				textureFormat = COLOR;
				initTexture( currentTexture );

				ppmHeightDone = imaMaxHeight;
			}
			if( SMT_DEBUG ) printf("Read texture file %s size %i %i \n", filename, imaW, imaH );
		}
		else if( SMT_DEBUG ) printf("Can't open file %s \n", filename );

		// Important
		currentTexture = firstTexture;
	 }
	 else{
		if( (imaH > TEXH) || (imaW > TEXW) ) {
			readTextureSplit(colorIma,filename);
		}	
		else if( colorIma != NULL ){
			firstTexture = new texture;
			currentTexture = firstTexture;
			currentTexture->nextTexture = NULL;
			glGenTextures(1,&textureID);
			currentTexture->id = textureID;
				currentTexture->ima = new pixel[ TEXW * 3 * TEXH ];
			for( int h = 0; h < imaH; h++ ){
				for(int w=0; w < imaW; w++){
					long offset1 = (h * TEXW + w) * 3;
					long offset2 = (h * imaW + w) * 3;
					currentTexture->ima[offset1] = colorIma[offset2];
					currentTexture->ima[offset1+1] = colorIma[offset2+1];
					currentTexture->ima[offset1+2] = colorIma[offset2+2];
				}
			}
			currentTexture->subIma = colorIma;
			currentTexture->w = TEXW;
			currentTexture->h = TEXH;
			currentTexture->ww = imaW;
			currentTexture->hh = imaH;
			textureFormat = COLOR;
			initTexture( currentTexture );
			if( SMT_DEBUG ) printf("Read texture file %s size %i %i \n", filename, imaW, imaH );
		}
		else if( SMT_DEBUG ) printf("Can't open file %s \n", filename );
	 } // end else
 // no sense in keeping the subIma/colorIma around, we don't actually use it
 delete[] colorIma;
 }
}


void
manuModel::replaceTexture(char *filename)
{
/*
	if( SMT_DEBUG )
		printf("Replacing texture with %s \n", filename );
	glBindTexture( GL_TEXTURE_2D, textureID );
	pixel *colorIma = (pixel *) MBLoadBitmap2(filename,  imaW, imaH);
	if( SMT_DEBUG )
		printf("Size %i %i \n", imaW, imaH );
	glTexSubImage2D(  GL_TEXTURE_2D, 0, 0, 0, imaW, imaH, GL_RGB, GL_UNSIGNED_BYTE, colorIma );
*/
}

void loadMipped(GLenum internalformat, int w, int h, void * data)
{
	// glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h,
						0, GL_RGB, GL_UNSIGNED_BYTE, data );

	//gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, w, h,
	//					GL_RGB, GL_UNSIGNED_BYTE, data );
	errtest(__FILE__,__LINE__,__FUNCTION__);
	GLint size_in_bytes;
	for(int i = 0; i <= 0; i++) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_ARB, &size_in_bytes);
		printf("COMPRESSED? %d\n",size_in_bytes);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size_in_bytes);
		errtest(__FILE__,__LINE__,__FUNCTION__);
		printf("Compressed texture size of level %d: %d\n", i, size_in_bytes);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_INTERNAL_FORMAT, &size_in_bytes);
		printf("Format %d (%d)\n",size_in_bytes,internalformat);
	}

}

void manuModel::initTexture( texture *inTexture )
{
	glBindTexture(GL_TEXTURE_2D, inTexture->id );
	errtest(__FILE__,__LINE__,__FUNCTION__);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	if(textureFormat == COLOR) {
		/*
		if(glexExtensionsSupported("GL_EXT_texture_compression_s3tc")) {
			if( SMT_DEBUG ) printf("Using S3TC texture compression\n"); 
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, (int)inTexture->w, (int)inTexture->h,
			//			0, GL_RGB, GL_UNSIGNED_BYTE, inTexture->ima );
			
			loadMipped(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, (int)inTexture->w, (int)inTexture->h, inTexture->ima);	
		
			GLint errval = glGetError();
			if(errval) {
				// printf("Error: %s\n",gluErrorString(errval));
			}
		}
		else
		*/
		if(GLEE_ARB_texture_compression) {
			if( SMT_DEBUG ) printf("Using generic ARB texture compression\n"); 
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_ARB, (int)inTexture->w, (int)inTexture->h,
			//			0, GL_RGB, GL_UNSIGNED_BYTE, inTexture->ima );
			
			loadMipped(GL_COMPRESSED_RGB_ARB, (int)inTexture->w, (int)inTexture->h, inTexture->ima);	
			
			// glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 3 );
		}
		else 
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)inTexture->w, (int)inTexture->h,
						0, GL_RGB, GL_UNSIGNED_BYTE, inTexture->ima );
		}
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (int)inTexture->w, (int)inTexture->h,
						0, GL_LUMINANCE, GL_UNSIGNED_BYTE, inTexture->ima );
	}
}

void manuModel::BindNextTexture()
{
	texture *next = currentTexture->nextTexture;
	if( next == NULL ) next = firstTexture;
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, currentTexture->id );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

	currentTexture = next;
}

void manuModel::BindArrTexture(int position)
{
	currentTexture = firstTexture;
	for(int i = 0; (i < position) && (currentTexture->nextTexture != NULL); i++) {
		currentTexture = currentTexture->nextTexture;
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, currentTexture->id );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
}

double manuModel::edgeLengthSum()
{
	double sum = 0;
	#define sq(x) ((x)*(x))
	if (YL_UseQuad){

		for(int j=0; j < ySamples-1; j++){
			for(int i=0; i < xSamples-1; i++){
			// v1 ---  v2     d1 = v1+v2
			//                d2 = v2+v3
			//  |      |      d3 = v3+v4
			//  |      |      d4 = v4+v1
			// 
			// v4 ---  v3

			int offset_v1, offset_v2, offset_v3, offset_v4;

			offset_v1 = j*(xSamples)+i;
			offset_v2 = j * (xSamples)+i+1;
			offset_v3 = (j+1) * (xSamples)+i+1;
			offset_v4 = (j+1) * (xSamples)+i;

			double d1=0, d2=0, d3=0, d4=0;

	

			// v1--v2  compute only if at the top
			if (j==0)
				d1 = sqrt( sq( verList[offset_v1].x - verList[ offset_v2 ].x ) +
						   sq( verList[offset_v1].y - verList[ offset_v2 ].y ) +
						   sq( verList[offset_v1].z - verList[ offset_v2 ].z )     );

			
			d2 = sqrt( sq( verList[offset_v3].x - verList[ offset_v2 ].x ) +
					   sq( verList[offset_v3].y - verList[ offset_v2 ].y ) +
					   sq( verList[offset_v3].z - verList[ offset_v2 ].z )     );

			d3 = sqrt( sq( verList[offset_v3].x - verList[ offset_v4 ].x ) +
					   sq( verList[offset_v3].y - verList[ offset_v4 ].y ) +
					   sq( verList[offset_v3].z - verList[ offset_v4 ].z )     );

			// v1--v3 compute only if at the begining
			if (i==0)
				d4 = sqrt( sq( verList[offset_v1].x - verList[ offset_v4 ].x ) +
						   sq( verList[offset_v1].y - verList[ offset_v4 ].y ) +
						   sq( verList[offset_v1].z - verList[ offset_v4 ].z )     );

  
			sum += (d1 + d2 + d3 + d4);
					
			}
		}
	}
	else{
		for(int i=0; i < nTrig; i++){
			Triangle t = trigList[i];
			sum += sqrt( sq(verList[t.idx1].x - verList[t.idx2].x)   + 
					 sq(verList[t.idx1].y - verList[t.idx2].y)   +
					 sq(verList[t.idx1].z - verList[t.idx2].z));
			sum += sqrt( sq(verList[t.idx2].x - verList[t.idx3].x)   + 
					 sq(verList[t.idx2].y - verList[t.idx3].y)   +
					 sq(verList[t.idx2].z - verList[t.idx3].z));
			sum += sqrt( sq(verList[t.idx1].x - verList[t.idx3].x)   + 
					 sq(verList[t.idx1].y - verList[t.idx3].y)   +
					 sq(verList[t.idx1].z - verList[t.idx3].z));
		}
	}

	return sum*scaleFactor;

}

bool IsVertexInEdge(void *vv, void *edge)
{
	long *v = (long *)vv;
	Edge *e = (Edge *)edge;
	
	if ( (e->v1 == *v) || (e->v2 == *v) ) return true;
	else return false;
};
