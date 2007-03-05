#ifndef SWAP
#define SWAP(a,b) { pixel t; t = a; a = b; b = t; }
#endif

#ifndef pixel
typedef unsigned char pixel;
#endif

struct RGB
{
	pixel a,b,c;
};


typedef struct tagBITMAPINFOHEADER
{
	unsigned long biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

bool  MBSaveBitmap(char *name, int w, int h, int depth, pixel *ima);
pixel *MBLoadBitmap(char *name, int &w, int &h);

pixel *MBLoadBitmap2(char *name, int &w, int &h);
