#ifndef __MANUMODEL_H__
#define __MANUMODEL_H__

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include "MBlist.h"

typedef unsigned char pixel;

struct Point {
	float x,y,z;
	float u1, v1;
	int color;
};

struct Quad {
	long v1, v2, v3, v4;
};

struct Edge
{
   long v1, v2;
   bool visted;
};

typedef unsigned char pixel;

struct texture
{
	pixel *ima;
	pixel *subIma;
	float w, h, d, ww, hh, dd;
	int id;
	texture *nextTexture;
};

struct Triangle
{
	int idx1, idx2, idx3;
	float textureVertex1[2], textureVertex2[2], textureVertex3[2];
};

#define GRAY  1
#define COLOR 3
#define GRAYLEVEL

// #define TEXW 2048
// #define TEXH 2048

// #define TEXW 4096
// #define TEXH 4096
// #define TEXD 128

extern GLint TEXW;
extern GLint TEXH;

class manuModel
{
	public:
		texture *firstTexture, *currentTexture;
		bool texArray;
		int numberOfTextures, numberOfTrianglesInATexture;
		int tileW, tileH, border;

		pixel *image;
		long nVer, nQuad, nEdges, nTrig;
		int xSamples, ySamples;
		
		Point *verList, *originalList;
		Quad  *quadList;
		Edge  **edgeList;
		Triangle *trigList;
		bool firstRun;
		int TRIGTEXRES, NUMTRIGPERROW;
		
		int textureFormat;
		GLuint textureID, modelDL, testID;
		double cz, cy, cx;
		int imaW, imaH;
		double edgeSum;
		double scaleFactor; 
		MBlist **verEdgeList;
		bool SMT_DEBUG;
	
		bool YL_UseQuad;
		bool YL_UseTriangularTextureMap;
		char *textureFile;
		char *meshFile;
		float minx, miny, minz, maxx, maxy, maxz;
		
		manuModel();
		~manuModel();
		bool readMesh(char *filename);
		bool readOBJ(char *filename);
		void readTexture(char *filename);
		void readTextureSplit(pixel *colorIma);
		
		void replaceTexture(char *filename);
		void initTexture( texture *inTexture );
		void BindNextTexture();
		void BindArrTexture(int position);
		double edgeLengthSum();
		void makeImage();
		int readPPM( char* filename, unsigned char* &image, int &width, int &height );
		int readImage( char* filename, unsigned char* &image, int &width, int &height );
};

#endif // __MANUMODEL_H__
