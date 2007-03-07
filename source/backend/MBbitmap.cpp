#include <stdlib.h>
#include <stdio.h>
#include "MBbitmap.h"

bool MBSaveBitmap(char *name, int w, int h, int depth, pixel *ima)
{
	BITMAPINFOHEADER bihead;
	BITMAPFILEHEADER fhead;

	FILE *fp;
	fp=fopen(name,"wb");
	if (fp==NULL) return false;

	fhead.bfType = 19778;
	fhead.bfSize = w * h * depth + 54;
	fhead.bfReserved1 = 0;
	fhead.bfReserved2 = 0;
	fhead.bfOffBits   = 54;

	bihead.biBitCount = depth * 8;
	bihead.biClrImportant = 0;
	bihead.biClrUsed = 0;
	bihead.biCompression = 0;  // no compression
	bihead.biHeight = h;
	bihead.biPlanes = 1;
	bihead.biSize = 40;       // size of this structure
	bihead.biSizeImage = w * h * depth;
	bihead.biXPelsPerMeter = 2925; // emperically found
	bihead.biYPelsPerMeter = 2925; // "
	bihead.biWidth = w;

	fwrite(&fhead, sizeof(BITMAPFILEHEADER),
			1, fp);
	fwrite(&bihead, sizeof(BITMAPINFOHEADER),
			1, fp);

	int n = fwrite(ima, sizeof(pixel), w * h * depth, fp);
	printf("SAVEBITMAP %i %i \n", n, w * h * depth );
	fclose(fp);
	return true;
}

//#define DEBUG 1
/* - requires MIL.h header - which is a commerical library ( methinks )
pixel *MBLoadBitmap2(char *name, int &w, int &h)
{
	// major hack
	// i couldn't get my load bitmap code to read in the bitmap
	// so I use my code to determine the size (w,h)
	// then I allocate a milBuffer of that size and use the mil BMP reader
	// to fill in the buffer
	int ww, hh;
	pixel *ima =  MBLoadBitmap(name, ww, hh );

	w = ww; h = hh;
//	w = 1996; h = 1450;

	MIL_ID milApp, milSys, milBuf;

	MappAlloc( M_DEFAULT, &milApp );
	MsysAlloc( M_SYSTEM_HOST, M_DEFAULT, M_COMPLETE, &milSys );

	MbufAllocColor( milSys, 3, w, h, 8 + M_UNSIGNED,
						M_IMAGE, 
						&milBuf );
	//MbufAlloc( milSys, 
	MbufImport( name, M_DEFAULT, M_LOAD, milSys, &milBuf);

	//MbufExport( "e:\\temp\\milOut.bmp", M_BMP, milBuf );

	MbufInquire( milBuf, M_SIZE_X, &w );
	MbufInquire( milBuf, M_SIZE_Y, &h );
	MbufInquire( milBuf, M_HOST_ADDRESS, &ima );
	ima = new pixel[ w * h * 3 ];

	MbufGetColor(milBuf, M_RGB24+M_PACKED, M_ALL_BAND, ima );

//	RGB *cBuffer = (RGB *) ima;
//	for(int i=0; i < w * h; i++)
//		SWAP( cBuffer[i].a, cBuffer[i].c );

	MbufFree( milBuf );
	MsysFree( milSys );
	MappFree( milApp );

	return ima;

}
*/

pixel *MBLoadBitmap(char *name, int &w, int &h)
{

    BITMAPINFOHEADER bihead;
    BITMAPFILEHEADER fhead;
    FILE *fp; 
	int linebytes,clrnum;

	fp=fopen(name,"rb");
	if (fp==NULL) return NULL;

    fread(&fhead,1,sizeof(BITMAPFILEHEADER),fp);
	
#ifdef DEBUG
	printf("TYPE %i \n"
			"SIZE %i \n"
			"R1   %i \n"
			"R2   %i \n"
			"BOS  %i \n",
		fhead.bfType, fhead.bfSize,
		fhead.bfReserved1, fhead.bfReserved2,
		fhead.bfOffBits );
#endif

    fread(&bihead,1,sizeof(BITMAPINFOHEADER),fp);

#ifdef DEBUG
	printf( "biBitCount %i     \n"
			"biClrImportant  %i \n"
			"biClrUsed       %i \n"
			"biCompression   %i \n"
			"biHeight        %i \n"
			"biPlanes        %i \n"
			"biSize			 %i \n"
			"biSizeImage     %i \n"
			"biWidth         %i \n"
			"biXPelsPerMeter %i \n"
			"biYPelsPerMeter %i \n"
		,bihead.biBitCount,bihead.biClrImportant,
		bihead.biClrUsed, bihead.biCompression,
		bihead.biHeight,
		bihead.biPlanes, bihead.biSize, bihead.biSizeImage,
		bihead.biWidth, bihead.biXPelsPerMeter,
		bihead.biYPelsPerMeter );

	switch (bihead.biCompression)
	{
		case BI_RGB		  : printf("uncompressed \n"); break;
		case BI_RLE8	  : printf("rle 8	     \n"); break;
		case BI_RLE4      : printf("rle 4        \n"); break;
		case BI_BITFIELDS : printf("bitfields    \n"); break;
	}
#endif
 
	linebytes=bihead.biWidth * (bihead.biBitCount/8);

    linebytes=(linebytes+3)/4*4;
    clrnum=1<<bihead.biBitCount;

	//color=(RGBQUAD*) new pixel[ sizeof(RGBQUAD))*(clrnum) ];

    //color=(RGBQUAD*) malloc((sizeof(RGBQUAD))*(clrnum));
    //if (color==NULL) return NULL;

	bihead.biSizeImage = bihead.biWidth * bihead.biHeight * 3;
   
	pixel *buffer = new pixel[bihead.biSizeImage];
    int n = fread(buffer,1, bihead.biSizeImage,fp);
//	printf("Reading %i -- read %i \n", bihead.biSizeImage, n );

//	pixel *buffer = new pixel[bihead.biSizeImage];
	w     = bihead.biWidth;
	h     = bihead.biHeight;

	/*
	RGB *cBuffer = (RGB *) buffer;

	for(int i=0; i < w * h; i++)
	{
		SWAP( cBuffer[i].a, cBuffer[i].c );
	}
	*/
	fclose(fp);

	return buffer;

}
