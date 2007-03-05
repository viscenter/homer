#include "string.h"

#include "cv.h"
#include "cvaux.h"
#include "highgui.h"
#include <math.h>
#include <stdio.h>      
#include <ctype.h>
#include <string>
#include <string.h>
#include <fstream>
#include <stdlib.h>


#define CAM_1 0
#define CAM_2 1
#define PROJ_1 2
#define MAX_CALIB_IMAGES 8
#define PIXEL_RANGE 3

#define ROUND_FLOAT(x)		static_cast<int>(x + 0.5)
#define MAX_NUM( a, b ) (a>b?a:b)
#define MIN_NUM( a, b ) (a<b?a:b)
//////////////////////////////////////////////////////////////////////////////////
#define EPS 1e-9
//inline double abs(double d) { return fabs(d); };



// Compute the determinant of the 3x3 matrix represented by 3 row vectors.
static inline double det(CvPoint3D32f v1, CvPoint3D32f v2, CvPoint3D32f v3)
{
    return v1.x*v2.y*v3.z + v1.z*v2.x*v3.y + v1.y*v2.z*v3.x
           - v1.z*v2.y*v3.x - v1.x*v2.z*v3.y - v1.y*v2.x*v3.z;
};

static CvPoint3D32f operator +(CvPoint3D32f a, CvPoint3D32f b)
{
    return cvPoint3D32f(a.x + b.x, a.y + b.y, a.z + b.z);
}

static CvPoint3D32f operator -(CvPoint3D32f a, CvPoint3D32f b)
{
    return cvPoint3D32f(a.x - b.x, a.y - b.y, a.z - b.z);
}

static CvPoint3D32f operator *(CvPoint3D32f v, double f)
{
    return cvPoint3D32f(f*v.x, f*v.y, f*v.z);
}

static CvPoint3D32f midpoint(const CvPoint3D32f &p1, const CvPoint3D32f &p2)
{
    return cvPoint3D32f((p1.x+p2.x)/2, (p1.y+p2.y)/2, (p1.z+p2.z)/2);
}

static void operator +=(CvPoint3D32f &p1, const CvPoint3D32f &p2)
{
    p1.x += p2.x;
    p1.y += p2.y;
    p1.z += p2.z;
}

static CvPoint3D32f operator /(const CvPoint3D32f &p, int d)
{
    return cvPoint3D32f(p.x/d, p.y/d, p.z/d);
}

/////////////////////////////////////////////////////////////////////////////////
#define createVector( len )  (float*)cvAlloc( (len)*sizeof(float))
#define createMatrix( w, h )  (float*)cvAlloc( (w)*(h)*sizeof(float))

#define releaseMatrix( array) cvFree((void**)array)

#define scaleMatrix( src, dst, w, h, scale ) scaleVector( (src), (dst), (w)*(h), (scale) )

void scaleVector( const float* src, float* dst, int len, double scale )
{
    int i;
    for( i = 0; i < len; i++ )
        dst[i] = (float)(src[i]*scale);
}

void transposeMatrix( const float* src, int w, int h, float* dst )
{
    int i, j;

    for( i = 0; i < w; i++ )
        for( j = 0; j < h; j++ )
            *dst++ = src[j*w + i];
        
   
}

void addVector( const float* src1, const float* src2, float* dst, int len )
{
    int i;
    for( i = 0; i < len; i++ )
        dst[i] = src1[i] + src2[i];

    
}

void subVector( const float* src1, const float* src2, float* dst, int len )
{
    int i;
    for( i = 0; i < len; i++ )
        dst[i] = src1[i] - src2[i];

    
}

void mulVectors( const float* src1, const float* src2, float* dst, int len )
{
    int i;
    for( i = 0; i < len; i++ )
        dst[i] = src1[i] * src2[i];

   
}

void mulMatrix( const float* src1, int w1, int h1, const float* src2, int w2, int h2, float* dst){
    int i, j, k;

    if( w1 != h2 ){
        //assert(0);
        return;
    }

    for( i = 0; i < h1; i++, src1 += w1, dst += w2 ){
        for( j = 0; j < w2; j++ ){
            double s = 0;
            for( k = 0; k < w1; k++ ){
                s += src1[k]*src2[j + k*w2];
			}
            dst[j] = (float)s;
        }
	}

}

void mulMatrixD( const double* src1, int w1, int h1, const double* src2, int w2, int h2, double* dst){
    int i, j, k;

    if( w1 != h2 ){
        //assert(0);
        return;
    }

    for( i = 0; i < h1; i++, src1 += w1, dst += w2 ){
        for( j = 0; j < w2; j++ ){
            double s = 0;
            for( k = 0; k < w1; k++ ){
                s += src1[k]*src2[j + k*w2];
			}
            dst[j] = s;
        }
	}

}


void invMatrix( const float* src, int w, float* dst ){
    CvMat tsrc, tdst;

    cvInitMatHeader( &tsrc, w, w, CV_32FC1, (void*)src );
    cvInitMatHeader( &tdst, w, w, CV_32FC1, dst );
    cvInvert( &tsrc, &tdst );
}

void invMatrixD( const double* src, int w, double* dst ){
    CvMat tsrc, tdst;

    cvInitMatHeader( &tsrc, w, w, CV_64FC1, (void*)src );
    cvInitMatHeader( &tdst, w, w, CV_64FC1, dst );
    cvInvert( &tsrc, &tdst );
}

void printMatrix( const float* src1, int w1, int h1){
    int i, j;

    for( i = 0; i < h1; i++){
        for( j = 0; j < w1; j++ ){
            fprintf(stderr, "%.16f	",(float)src1[j + i*w1]);
		}
		fprintf(stderr, "\n");
	}

}

void printMatrixd( const double* src1, int w1, int h1){
    int i, j;

    for( i = 0; i < h1; i++){
        for( j = 0; j < w1; j++ ){
            fprintf(stderr, "%.32f	",src1[j + i*w1]);
		}
		fprintf(stderr, "\n");
	}

}



void printMatrixD( const double src1[3][3]){
    int i, j;

    for( i = 0; i < 3; i++){
        for( j = 0; j < 3; j++ ){
            fprintf(stderr, "%f	",(float)src1[i][j]);//j + i*w1]);
		}
		fprintf(stderr, "\n");
	}

}



void createConvertMatrVect(CvMatr32f rotMatr1, CvMatr32f transVect1, CvMatr32f rotMatr2, CvMatr32f transVect2, CvMatr32f convRotMatr, CvMatr32f convTransVect){
    float invRotMatr2[9];
    float tmpVect[3];

    invMatrix(rotMatr2,3,invRotMatr2);
    /* Test for error */

    mulMatrix(rotMatr1, 3,3, invRotMatr2, 3,3, convRotMatr);

    mulMatrix(convRotMatr, 3,3, transVect2, 1,3, tmpVect);
    
    subVector(transVect1,tmpVect,convTransVect,3);

}


////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////

void loadConfig(char * configFile);

static void print_performance( void );
float get_performance( void );
void loadCalib(int num);
void convertWarpCoordinates(double coeffs[3][3], CvPoint2D32f* cameraPoint, CvPoint2D32f* warpPoint, int direction);
void projectImageToPoint(   CvPoint2D32f* point, CvMatr32f camMatr,CvMatr32f rotMatr,CvVect32f transVect, CvPoint3D32f* projPoint);
void Gauss( int n, float a[][3], int* l);
void solve(int n, float a[][3], int* l, float* b, float* x);
float solveForCenter( int startX, int n, float* vals );
float funct( int i, float val );
bool intersection(CvPoint3D32f o1, CvPoint3D32f p1, CvPoint3D32f o2, CvPoint3D32f p2, CvPoint3D32f &r1, CvPoint3D32f &r2);
CvPoint3D32f ImageCStoWorldCS(CvMatr32f camMat, const float camera_info[4][4], CvPoint2D32f * p);
CvPoint2D32f WorldCStoImageCS(CvMatr32f camMat, const float camera_info[4][4], CvPoint3D32f * worldPoint);
void MultVectorMatrix(float rv[4], const float v[4], const float m[4][4]);
static void MultMatrix(float rm[4][4], const float m1[4][4], const float m2[4][4]);

IplImage* img0 = 0;
IplImage* img = 0;
IplImage* img1 = 0;
IplImage* undistortData[4];

int order[4];		//!< a static array of ints filled by the config file that determines the order of the cameras

int xVal[2][480];
float tempDisp[2];
//CvSize tempSize = cvSize(640,480);
CvStereoCamera* camPairs[4];
//float foundPoint[600][480][4][2];

//float corrPoints[600][480][4][2];

float fDVal[584][480];


char calibName[70];
char scanName[70];
char scanPath[70];
double camZeroInv[3][3][3];
int camOrder[4];
int numberOfCameras=0;
int texCamNum = 1;
int numImages = 0;
int startImages = 0;


CvVect32f distortion[2]; 
CvMatr32f cameraMatrix[2]; 
CvVect32f transVects[2]; 
CvMatr32f rotMatrix[2]; 

	
CvMatr32f cam4;
CvMatr32f tempMatrix; 
CvMatr32f coeffMatrix;
CvMatr32f warpMatrix[2];
CvMatr32f tmat,rmat,smat;
float fBaseline = 0.f;

float inputTrash = 0.f;
float inputTrash1 = 0.f;
float inputTrash2 = 0.f;

//char *inputTrash, *inputTrash1, *inputTrash2;


int main( int argc, char* argv[]){

	if(argc < 3){
		fprintf(stderr, "usage: %s <scan name> <tex cam>\n", argv[0]);
		exit(1);
	}


	//strcpy(scanName, argv[1]);

	loadConfig(argv[1]);

	texCamNum = atoi(argv[2]);

	char* tempDist = new char[70];
	char* tempNum = new char[70];
//	inputTrash = new char[70];
	//inputTrash1 = new char[70];
	//inputTrash2 = new char[70];



	cvNamedWindow("Texture", CV_WINDOW_AUTOSIZE);
	CvMat* tmpMap, *rectMap;

	CvPoint2D32f * warpP;
	CvPoint2D32f * camP;
	CvPoint3D32f* worldPoint;

	int startLineX	= -1;
	int numLine		= -1;

		
	warpP	=	(CvPoint2D32f *)malloc(sizeof(CvPoint2D32f)); 
	camP	=	(CvPoint2D32f *)malloc(sizeof(CvPoint2D32f)); 
	worldPoint= (CvPoint3D32f *)malloc(sizeof(CvPoint3D32f)); 
	
	
//	img1 = cvCreateImageHeader(tempSize,IPL_DEPTH_8U,1);
//	cvCreateImageData(img1);

	tempMatrix = new float [3*3];
	coeffMatrix = new float [3*3];

	warpMatrix[0] = new float [3*3];
	warpMatrix[1] = new float [3*3];

	tmat = new float [4*4];
	smat = new float [4*4];
	rmat = new float [4*4];

	for(int i = 0 ; i <2; i++){	
		distortion[i] = new float [4]; 
		cameraMatrix[i] = new float [3*3];
		transVects[i] = new float[4 * 3];
		rotMatrix[i] = new float[4 * 9];
	}

	cam4 = new float[4 * 4];

	loadCalib(texCamNum);
	printf("here it is\n");
	
	//cvShowImage("test", img);
	//cvUnDistortInit(img0, undistortData[i], cameraMatrix[i], distortion[i], 1);
	
	CvMatr32f  convRotMatr = createMatrix(3,3);
	CvMatr32f  convTransVect  = createMatrix(1,3);

	float cam4[4][4];
	//	float fTemp;
	//	int x1, x2 ,x3;
	//int texCamNum = 0;

	//	for(int cam = 0; cam < numberOfCameras; cam++){
			//combine the two transformations into one matrix
			//order is important! rotations are not commutative


		float	tmat[4][4] = { { 1.f, 0.f, 0.f, 0.f },
								 { 0.f, 1.f, 0.f, 0.f },
								 { 0.f, 0.f, 1.f, 0.f },
								 { transVects[texCamNum-1][0], transVects[texCamNum-1][1], transVects[texCamNum-1][2], 1.f } };
        
		float rmat[4][4] = { { rotMatrix[texCamNum-1][0], rotMatrix[texCamNum-1][1], rotMatrix[texCamNum-1][2], 0.f },
								 { rotMatrix[texCamNum-1][3], rotMatrix[texCamNum-1][4], rotMatrix[texCamNum-1][5], 0.f },
								 { rotMatrix[texCamNum-1][6], rotMatrix[texCamNum-1][7], rotMatrix[texCamNum-1][8], 0.f },
								 { 0.f, 0.f, 0.f, 1.f } };

			
		MultMatrix(cam4, tmat, rmat);
		printMatrix((float *)rmat, 4,4);

			// change the transformation of the cameras to put them in the world coordinate 
			// system we want to work with.

			// Start with an identity matrix; then fill in the values to accomplish
			// the desired transformation.

		float smat[4][4] = { { 1.f, 0.f, 0.f, 0.f },
							{ 0.f, 1.f, 0.f, 0.f },
								 { 0.f, 0.f, 1.f, 0.f },
								 { 0.f, 0.f, 0.f, 1.f } };

		// First, reflect through the origin by inverting all three axes.
		smat[0][0] = -1.f;
		smat[1][1] = -1.f;
		smat[2][2] = -1.f;
		
		MultMatrix(tmat, cam4, smat);

		// Scale x and y coordinates by the focal length (allowing for non-square pixels
		// and/or non-symmetrical lenses).
		smat[0][0] = 1.0f/ (cameraMatrix[texCamNum-1][0]); 
		smat[1][1] = 1.0f/ (cameraMatrix[texCamNum-1][4]); 
		smat[2][2] = 1.0f;
		MultMatrix(cam4, smat, tmat);





	char* scanBuf = new char[256];


	

	memset(tempDist, '\0', 50);

	sprintf(tempDist, "%s/%s/3D-reconstruction/%s%d%d%d%d-%d-%d.obj", scanPath, scanName, scanName, order[0], order[1], order[2], order[3], startImages, numImages);
	
	fprintf(stderr, "Saving to file: %s\n", tempDist);

	FILE * objfp;
	if((objfp = fopen(tempDist, "wb")) == NULL){
	
		fprintf(stderr, "could not open %s\n", tempDist);

	}

	memset(tempDist, '\0', 50);
				
	sprintf(tempDist, "%s/%s/3D-reconstruction/%s%d%d%d%d-%d-%d.asc", scanPath, scanName, scanName,order[0], order[1], order[2], order[3], startImages, numImages);
	
	fprintf(stderr, "Saving to file: %s\n", tempDist);

	FILE * tempfp;
	if((tempfp = fopen(tempDist, "wb")) == NULL){
	
		fprintf(stderr, "could not open %s\n", tempDist);

	}
	else{
	
		
		
		/*
		FILE *fp;
		if((fp = fopen(tempDist, "rb")) == NULL){

			fprintf(stderr, "could not open %s\n", tempDist);

		}
		else{
			float x,y,z;
			int verts = 0;
			while(!feof(fp)){

				fscanf(fp, "%f	%f	%f\n", &x, &y, &z);
				verts++;
				fprintf(stderr, "%d\n", verts);
			}

			///fprintf(stderr, "%d\n", verts);
			//fprintf(tempfp, "%i 2 3 0 \n", verts);
			fflush(fp);
			fclose(fp);
		}
*/
		
		memset(tempDist, '\0', 50);
		sprintf(tempDist, "%s/%s/3D-reconstruction/%s%d%d%d%d-%d-%d.ls-lmmin-xyz", scanPath, scanName, scanName, order[0], order[1], order[2], order[3], startImages, numImages);
//		%s%d%d%d%d-%d-%d.gfit-xyz", scanPath,scanName, scanName, order[0], order[1], order[2], order[3],

		fprintf(stderr, "Using Data file: %s\n", tempDist);
                 
	//	strcat(tempDist, ".txt");
		FILE *fp;
		if((fp = fopen(tempDist, "r")) == NULL){

			fprintf(stderr, "could not open %s\nPlease verify that it was created by reconstruct_points\n", tempDist);
			exit(1);

		}
		else{
			IplImage* img0 = 0;

			memset(tempDist, '\0', 50);
			sprintf(tempDist, "%s/%s/Texture/Tex_Images-cam%d/%s.jpg", scanPath,scanName, texCamNum, scanName);

			fprintf(stderr, "Opening image %s\n", tempDist);

			if((img0 = cvLoadImage(tempDist,1)) == NULL){
		
				fprintf(stderr, "can't open file %s\nPlease check paramters\n", tempDist);
				exit(1);
			}
			cvShowImage("Texture", img0);

			float x,y,z;
			int verts = 0;
			int iX=0, iY=0;
			int topLeft[3], topRight[3], bottomLeft[3], bottomRight[3];

			
			float tX;
			float tY;

			char line[512];

			fprintf(stderr, "Calculating Texture\n");
			while(!feof(fp)){

				//ADD FIX FOR # COMMENTS


			fgets(line, 512, fp);
				while (line[0] == '#')
				fgets(line, 512, fp);
				
				if(!sscanf(line, "%f %f %f %f %f %f",&x, &y, &z, &inputTrash, &inputTrash1, &inputTrash2)){

				if(!sscanf(line, "v	%f	%f	%f	%s	%s	%s",&x, &y, &z, &inputTrash, &inputTrash, &inputTrash)){
//					fprintf(stderr, "%d\n",fscanf(fp, "v	%f	%f	%f	%s	%s	%s\n",x, y, z, inputTrash, inputTrash, inputTrash));

					
						//fprintf(stderr, "%d\n",fscanf(fp, "%f	%f	%f	%S	%s	%s\n",&x, &y, &z, &inputTrash, &inputTrash, &inputTrash));

						//fscanf(fp, "#%s\n", &inputTrash);
					}
				}

			//	fprintf(stderr, "%f	%f	%f	%f	%f	%f\n", x,y,z,inputTrash,inputTrash1,inputTrash2);
			//	fprintf(stderr, "%d\n",fscanf(fp, "%f	%f	%f	%S	%s	%s\n",&x, &y, &z, &inputTrash, &inputTrash, &inputTrash));

				CvPoint3D32f t;
				t.x = x;
				t.y = y;
				t.z = z;

				CvPoint2D32f r = WorldCStoImageCS(cameraMatrix[texCamNum-1], cam4, &t);

				iX=r.x;
				iY=r.y;

		//		fprintf(stderr, "%f	%f\n", r.x, r.y);
	if(((int)r.x > 0) && ((int)r.x < img0->width-1) && ((int)r.y > 0) && ((int)r.y < img0->height-1)){
				topLeft[0] = ((uchar*)(img0->imageData + img0->widthStep*iY))[iX*3+2];
				topLeft[1] = ((uchar*)(img0->imageData + img0->widthStep*iY))[iX*3+1];
				topLeft[2] = ((uchar*)(img0->imageData + img0->widthStep*iY))[iX*3+0];

				topRight[0] = ((uchar*)(img0->imageData + img0->widthStep*iY))[(iX+1)*3+2];
				topRight[1] = ((uchar*)(img0->imageData + img0->widthStep*iY))[(iX+1)*3+1];
				topRight[2] = ((uchar*)(img0->imageData + img0->widthStep*iY))[(iX+1)*3+0];

				bottomRight[0] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[(iX+1)*3+2];
				bottomRight[1] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[(iX+1)*3+1];
				bottomRight[2] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[(iX+1)*3+0];
				
				bottomLeft[0] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[iX*3+2];
				bottomLeft[1] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[iX*3+1];
				bottomLeft[2] = ((uchar*)(img0->imageData + img0->widthStep*(iY+1)))[iX*3+0];

				tX = r.x - iX;
				tY = r.y - iY;


		

					fprintf(tempfp, "%f %f %f %d %d %d\n", x, y, z, 
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[0])) + ((1-tY)*(tX*topRight[0])) + (tY*((1-tX)*bottomLeft[0])) + (tY*(tX*bottomRight[0]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[1])) + ((1-tY)*(tX*topRight[1])) + (tY*((1-tX)*bottomLeft[1])) + (tY*(tX*bottomRight[1]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[2])) + ((1-tY)*(tX*topRight[2])) + (tY*((1-tX)*bottomLeft[2])) + (tY*(tX*bottomRight[2]))));

					fprintf(objfp, "v %f %f %f %d %d %d\n", x, y, z, 
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[0])) + ((1-tY)*(tX*topRight[0])) + (tY*((1-tX)*bottomLeft[0])) + (tY*(tX*bottomRight[0]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[1])) + ((1-tY)*(tX*topRight[1])) + (tY*((1-tX)*bottomLeft[1])) + (tY*(tX*bottomRight[1]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[2])) + ((1-tY)*(tX*topRight[2])) + (tY*((1-tX)*bottomLeft[2])) + (tY*(tX*bottomRight[2]))));

//					fprintf(tempfp, "%i 2 3 0 \n", verts);
/*
					fprintf(stderr, "v %f %f %f %d %d %d\n", x, y, z, 
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[0])) + ((1-tY)*(tX*topRight[0])) + (tY*((1-tX)*bottomLeft[0])) + (tY*(tX*bottomRight[0]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[1])) + ((1-tY)*(tX*topRight[1])) + (tY*((1-tX)*bottomLeft[1])) + (tY*(tX*bottomRight[1]))),
					ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[2])) + ((1-tY)*(tX*topRight[2])) + (tY*((1-tX)*bottomLeft[2])) + (tY*(tX*bottomRight[2]))));
*/
													/*
													ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[0])) + ((1-tY)*(tX*topRight[0])) + (tY*((1-tX)*bottomLeft[0])) + (tY*(tX*bottomRight[0]))),
													ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[1])) + ((1-tY)*(tX*topRight[1])) + (tY*((1-tX)*bottomLeft[1])) + (tY*(tX*bottomRight[1]))),
													ROUND_FLOAT(((1-tY)*((1-tX)*topLeft[2])) + ((1-tY)*(tX*topRight[2])) + (tY*((1-tX)*bottomLeft[2])) + (tY*(tX*bottomRight[2])))*/

					/*
					fprintf(tempfp, "v %f %f %f %d %d %d\n", x, y, z, 
					ROUND_FLOAT((tY*(tX*topLeft[0])) + (tY*((1-tX)*topRight[0])) + ((1-tY)*(tX*bottomLeft[0])) + ((1-tY)*((1-tX)*bottomRight[0]))),
					ROUND_FLOAT((tY*(tX*topLeft[1])) + (tY*((1-tX)*topRight[1])) + ((1-tY)*(tX*bottomLeft[1])) + ((1-tY)*((1-tX)*bottomRight[1]))),
					ROUND_FLOAT((tY*(tX*topLeft[2])) + (tY*((1-tX)*topRight[2])) + ((1-tY)*(tX*bottomLeft[2])) + ((1-tY)*((1-tX)*bottomRight[2]))));*/
/*
			fprintf(stderr, "v %f %f %f %d %d %d\n", x, y, z, 
					ROUND_FLOAT((tY*(tX*topLeft[0])) + (tY*((1-tX)*topRight[0])) + ((1-tY)*(tX*bottomLeft[0])) + ((1-tY)*((1-tX)*bottomRight[0]))),
					ROUND_FLOAT((tY*(tX*topLeft[1])) + (tY*((1-tX)*topRight[1])) + ((1-tY)*(tX*bottomLeft[1])) + ((1-tY)*((1-tX)*bottomRight[1]))),
					ROUND_FLOAT((tY*(tX*topLeft[2])) + (tY*((1-tX)*topRight[2])) + ((1-tY)*(tX*bottomLeft[2])) + ((1-tY)*((1-tX)*bottomRight[2]))));

*/				
					verts++;
				}

	else{
		fprintf(tempfp, "%f %f %f %d %d %d\n", x, y, z, 0,0,255);

					fprintf(objfp, "v %f %f %f %d %d %d\n", x, y, z, 0,0,255);
	//}

	}

			}

		}
		fclose(fp);
		//fflush(tempfp);
		fclose(tempfp);				



	
}

	return 0;

}

void loadCalib(int num){
	char* tempDist = new char[50];
	char* scanBuf = new char[256];
	
	memset(tempDist, '\0', 50);
	texCamNum = num;
	sprintf(tempDist, "%s/%s/Texture/Tex_Images-cam%d/%s-tex.mat", scanPath,scanName, texCamNum, scanName);
	//strcpy(tempDist, "C:/Documents and Settings/root/Desktop/temp_zip/");
	                 
//	strcat(tempDist, "-tex.mat");
	FILE *fp;
	if((fp = fopen(tempDist, "rb")) == NULL){

		fprintf(stderr, "could not open %s\nPlease check reconstruct_config.txt\n", tempDist);
		exit(1);

	}
	else{

		fprintf(stderr, "starting to read %s\n", tempDist);
	
		
		fscanf(fp,"%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f\n", 
		&cameraMatrix[texCamNum-1][0],&cameraMatrix[texCamNum-1][1],&cameraMatrix[texCamNum-1][2], &rotMatrix[texCamNum-1][0], &rotMatrix[texCamNum-1][1], &rotMatrix[texCamNum-1][2], &transVects[texCamNum-1][0],
		&cameraMatrix[texCamNum-1][3],&cameraMatrix[texCamNum-1][4],&cameraMatrix[texCamNum-1][5], &rotMatrix[texCamNum-1][3], &rotMatrix[texCamNum-1][4], &rotMatrix[texCamNum-1][5], &transVects[texCamNum-1][1],
		&cameraMatrix[texCamNum-1][6],&cameraMatrix[texCamNum-1][7],&cameraMatrix[texCamNum-1][8], &rotMatrix[texCamNum-1][6], &rotMatrix[texCamNum-1][7], &rotMatrix[texCamNum-1][8], &transVects[texCamNum-1][2],
		&distortion[texCamNum-1][0],&distortion[texCamNum-1][1],&distortion[texCamNum-1][2],&distortion[texCamNum-1][3]);

		float rmat[3][3] = { { rotMatrix[texCamNum-1][0], rotMatrix[texCamNum-1][1], rotMatrix[texCamNum-1][2]},
							{ rotMatrix[texCamNum-1][3], rotMatrix[texCamNum-1][4], rotMatrix[texCamNum-1][5]},
							{ rotMatrix[texCamNum-1][6], rotMatrix[texCamNum-1][7], rotMatrix[texCamNum-1][8]}};

		float tmat[3][1] = {{transVects[texCamNum-1][0]},
							{transVects[texCamNum-1][1]},
							{transVects[texCamNum-1][2]}};

		float theta = -45.f;


		float fixRmat[3][3] ={ { 1,0,0},//{ { -1,0,0},
								{ 0,1,0},
							{ 0,0,1}};
	
		//y axis
		float fixRmatY[3][3] = { {cos(theta), 0, sin(theta)},
								{ 0,1,0},
							{ -1*sin(theta), 0, cos(theta)}};

		//z axis
		float fixRmatZ[3][3] = { {cos(theta), -1*sin(theta), 0},
							{ sin(theta), cos(theta), 0},
							{ 0,0,1}};
							

			printMatrix(*fixRmat,3,3);
		fprintf(stderr ,"\n*\n");
		//printMatrix(*rmat,3,3);
		printMatrix(*tmat,3,1);
		fprintf(stderr ,"\n=\n");

	//	mulMatrix(*rmat, 3, 3, *fixRmatZ, 3, 3, rotMatrix[texCamNum-1]);
	//	mulMatrix(*fixRmat, 3, 3, *tmat, 1, 3, transVects[texCamNum-1]);
		//printMatrix(rotMatrix[texCamNum-1],3,3);
		printMatrix(transVects[texCamNum-1],3,1);




	
		fflush(fp);
		fclose(fp);

			fprintf(stderr,"%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f\n", 
		cameraMatrix[texCamNum-1][0],cameraMatrix[texCamNum-1][1],cameraMatrix[texCamNum-1][2], rotMatrix[texCamNum-1][0], rotMatrix[texCamNum-1][1], rotMatrix[texCamNum-1][2], transVects[texCamNum-1][0],
		cameraMatrix[texCamNum-1][3],cameraMatrix[texCamNum-1][4],cameraMatrix[texCamNum-1][5], rotMatrix[texCamNum-1][3], rotMatrix[texCamNum-1][4], rotMatrix[texCamNum-1][5], transVects[texCamNum-1][1],
		cameraMatrix[texCamNum-1][6],cameraMatrix[texCamNum-1][7],cameraMatrix[texCamNum-1][8], rotMatrix[texCamNum-1][6], rotMatrix[texCamNum-1][7], rotMatrix[texCamNum-1][8], transVects[texCamNum-1][2],
		distortion[texCamNum-1][0],distortion[texCamNum-1][1],distortion[texCamNum-1][2],distortion[texCamNum-1][3]);
/*

		float camSize[2];
		camSize[0] = 1536;
		camSize[1] = 1024;
		*/
	}

	//texCamNum = 2;
	//	sprintf(tempDist, "C:/Rocks/%s/Texture/Tex_Images-cam%d/%s-tex.mat", scanName, texCamNum, scanName);
	//strcpy(tempDist, "C:/Documents and Settings/root/Desktop/temp_zip/");
	                 
//	strcat(tempDist, "-tex.mat");
//	FILE *fp;

		/*
	if((fp = fopen(tempDist, "rb")) == NULL){

		fprintf(stderr, "could not open %s\n", tempDist);

	}
	else{

		fprintf(stderr, "starting to read %s\n", tempDist);
	
		
		fscanf(fp,"%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f\n", 
		&cameraMatrix[texCamNum-1][0],&cameraMatrix[texCamNum-1][1],&cameraMatrix[texCamNum-1][2], &rotMatrix[texCamNum-1][0], &rotMatrix[texCamNum-1][1], &rotMatrix[texCamNum-1][2], &transVects[texCamNum-1][0],
		&cameraMatrix[texCamNum-1][3],&cameraMatrix[texCamNum-1][4],&cameraMatrix[texCamNum-1][5], &rotMatrix[texCamNum-1][3], &rotMatrix[texCamNum-1][4], &rotMatrix[texCamNum-1][5], &transVects[texCamNum-1][1],
		&cameraMatrix[texCamNum-1][6],&cameraMatrix[texCamNum-1][7],&cameraMatrix[texCamNum-1][8], &rotMatrix[texCamNum-1][6], &rotMatrix[texCamNum-1][7], &rotMatrix[texCamNum-1][8], &transVects[texCamNum-1][2],
		&distortion[texCamNum-1][0],&distortion[texCamNum-1][1],&distortion[texCamNum-1][2],&distortion[texCamNum-1][3]);
	
		fflush(fp);
		fclose(fp);

			fprintf(stderr,"%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f	%f	%f	%f\n%f	%f	%f	%f\n", 
		cameraMatrix[texCamNum-1][0],cameraMatrix[texCamNum-1][1],cameraMatrix[texCamNum-1][2], rotMatrix[texCamNum-1][0], rotMatrix[texCamNum-1][1], rotMatrix[texCamNum-1][2], transVects[texCamNum-1][0],
		cameraMatrix[texCamNum-1][3],cameraMatrix[texCamNum-1][4],cameraMatrix[texCamNum-1][5], rotMatrix[texCamNum-1][3], rotMatrix[texCamNum-1][4], rotMatrix[texCamNum-1][5], transVects[texCamNum-1][1],
		cameraMatrix[texCamNum-1][6],cameraMatrix[texCamNum-1][7],cameraMatrix[texCamNum-1][8], rotMatrix[texCamNum-1][6], rotMatrix[texCamNum-1][7], rotMatrix[texCamNum-1][8], transVects[texCamNum-1][2],
		distortion[texCamNum-1][0],distortion[texCamNum-1][1],distortion[texCamNum-1][2],distortion[texCamNum-1][3]);



	}
*/

	
}

/* Convert function for stereo warping */
void convertWarpCoordinates(double coeffs[3][3], CvPoint2D32f* cameraPoint, CvPoint2D32f* warpPoint, int direction){
    double x,y;
    double det; 
    if( direction == 0 )
    {/* convert from warped image to camera image coordinates */
        x = warpPoint->x;
        y = warpPoint->y;
        
        det = (coeffs[2][0] * x + coeffs[2][1] * y + coeffs[2][2]);
        if( fabs(det) > 1e-8 )
        {
            cameraPoint->x = (float)((coeffs[0][0] * x + coeffs[0][1] * y + coeffs[0][2]) / det);
            cameraPoint->y = (float)((coeffs[1][0] * x + coeffs[1][1] * y + coeffs[1][2]) / det);
            return;
        }
    }
    else if( direction == 1 )
    {/* convert from camera image to warped image coordinates */
        x = cameraPoint->x;
        y = cameraPoint->y;

        det = (coeffs[2][0]*x-coeffs[0][0])*(coeffs[2][1]*y-coeffs[1][1])-(coeffs[2][1]*x-coeffs[0][1])*(coeffs[2][0]*y-coeffs[1][0]);

        if( fabs(det) > 1e-8 )
        {
            warpPoint->x = (float)(((coeffs[0][2]-coeffs[2][2]*x)*(coeffs[2][1]*y-coeffs[1][1])-(coeffs[2][1]*x-coeffs[0][1])*(coeffs[1][2]-coeffs[2][2]*y))/det);
            warpPoint->y = (float)(((coeffs[2][0]*x-coeffs[0][0])*(coeffs[1][2]-coeffs[2][2]*y)-(coeffs[0][2]-coeffs[2][2]*x)*(coeffs[2][0]*y-coeffs[1][0]))/det);
            return;
        }
    }
    
    return;
}

void projectImageToPoint(   CvPoint2D32f* point, CvMatr32f camMatr,CvMatr32f rotMatr,CvVect32f transVect, CvPoint3D32f* projPoint)
{

    float tmpVect1[3];
    float tmpVect2[3];
	float tmpVect3[3];

	tmpVect1[0] = point->x;
	tmpVect1[1] = point->y;
	tmpVect1[2] = 1.f;

	CvMatr32f invCamMatr = createMatrix(3,3);
	CvMatr32f invRotMatr = createMatrix(3,3);

	invMatrix(camMatr, 3, invCamMatr);
    
    mulMatrix(invCamMatr, 3, 3, tmpVect1, 1, 3, tmpVect2);

	subVector(tmpVect2, transVect, tmpVect3, 3);

	invMatrix(rotMatr, 3, invRotMatr);


    mulMatrix(invRotMatr, 3, 3, tmpVect3, 1, 3, tmpVect1);

    projPoint->x = tmpVect1[0];
    projPoint->y = tmpVect1[1];
   
    return;
}


float solveForCenter( int startX, int n, float* vals )
{
	float *A;
	float* y;
	float * invA;
  
	A = createMatrix(3,3);//matrix(3,3);
	invA = createMatrix(3,3);
	y = createVector(3);//vector(3);

	float ans;
	int p,q,z;

	for( p=0; p<3; ++p){
		for( q=0; q<3; ++q){
			A[p*3 + q] = 0.0;
		}
	}

	for( p=0; p<3; ++p){
		for( q=0; q<3; ++q){
			for( z = 0; z<n; ++z){
					A[p*3 + q] += funct(p, (float)(startX+z)) * funct(q, (float)(startX+z));
			}
		}
	}

	for( p=0; p<3; ++p ){
		y[p] = 0.0;
	}
	for( p=0; p<3; ++p ){
		for( z=0; z<n; ++z ){
			y[p] += vals[z] * funct(p, (float)(startX+z));
		}
	}
	float* x = createVector(3);
	for( p=0; p<3; ++p ){
		x[p] = 0.0;
	}
	/*
    int l[3];
		for( p=0; p<3; ++p ){
		l[p] = 0.0;
	}

	Gauss( 3, A, l);
	solve(3, A, l, y, x);
*/

//	printMatrix(A,3,3);
//	printMatrix(y,1,3);
invMatrix(A, 3, invA);
//printMatrix(invA,3,3);
mulMatrix(invA, 3, 3, y, 1, 3, x);

//	fprintf(stderr, "*************************c1 %f	c2 %f	c3 %f\n", x[0], x[1], x[2]);
//	ans = -x[1] / (2.f*x[0]);  CHANGED BECAUSE I THINK ITS WRONG
	ans = -x[1] / (2.f*x[2]);
	//releaseMatrix(&A);
//	releaseVector(&y);
//	releaseVector(&x);

	return ans;
}

void Gauss( int n, float a[][3], int* l){
       /****************************************************
           a is a n x n matrix and l is an int array of length n
           l is used as an index array that will determine the order of
           elimination of coefficients
           All array indexes are assumed to start at 0
       ******************************************************/
        double* s = new double[n];   //scaling factor
        int i, j=0, k;
        double r, rmax, smax, xmult;
        for(i=0; i<n; ++i){
            l[i] = i;
            smax = 0;
            for(j=0; j<n; ++j)
                smax = MAX_NUM(smax, fabs(a[i][j]));
            s[i] = smax;
        }

        i=n-1;
        for(k=0; k<(n-1); ++k){
        --j;
            rmax = 0;
            for(i=k; i<n; ++i){
                r = fabs(a[l[i]][k] / s[l[i]]);
				if( r > rmax){
					rmax = r;
					j=i;
				}
			}
		
			int temp = l[j];
			l[j] = l[k];
			l[k] = temp;
			for( i=k+1; i<n; ++i){
				xmult = a[l[i]][k] / a[l[k]][k];
				a[l[i]][k] = xmult;
				for(j=k+1; j<n; ++j){
				a[l[i]][j] = a[l[i]][j] - xmult*a[l[k]][j];
				}
			}
		}
}

void solve(int n, float a[][3], int* l, float* b, float* x){
      /*********************************************************
         a and l have previously been passed to Gauss() b is the product of
         a and x. x is the 1x3 matrix of coefficients to solve for
      *************************************************************/
      int i, k;
      double sum;
      for(k=0; k < (n-1); ++k){
          for( i=k+1; i<n; ++i){
              b[l[i]] -= a[l[i]][k] * b[l[k]];
          }
      }
      x[n-1] = b[l[n-1]] / a[l[n-1]][n-1];

      for( i=n-2; i>= 0; --i){
          sum = b[l[i]];
          for(int j=i+1; j<n; ++j){
             sum = sum - a[l[i]][j] * x[j];
          }
          x[i] = sum / a[l[i]][i];
      }
}

float funct( int i, float val ){
	switch( i ){
	case 2:  //  changed from 0
           return val*val;
	case 1:  // stays the same
		   return val;
	case 0:  // changed from 2
		return 1.f;
	}
	return -1;
}

// Find the intersection of two lines, or if they don't intersect,
// the points of closest approach.
// The lines are defined by (o1,p1) and (o2, p2).
// If they intersect, r1 and r2 will be the same.
// Returns false on error.






/*
typedef struct
{
    CvBool valid;
    float mat[4][4];              
    CvPoint2D32f principal_point; 
                                  
} Cv3dTrackerCameraInfo;

typedef struct
{
    CvPoint2D32f principal_point;
    float focal_length[2];
    float distortion[4];
} Cv3dTrackerCameraIntrinsics;

*/



bool intersection(CvPoint3D32f o1, CvPoint3D32f p1, CvPoint3D32f o2, CvPoint3D32f p2, CvPoint3D32f &r1, CvPoint3D32f &r2){
    CvPoint3D32f x = o2 - o1;
    CvPoint3D32f d1 = p1 - o1;
    CvPoint3D32f d2 = p2 - o2;

	
    CvPoint3D32f cross = cvPoint3D32f(d1.y*d2.z - d1.z*d2.y, d1.z*d2.x - d1.x*d2.z, d1.x*d2.y - d1.y*d2.x);

    double den = cross.x*cross.x + cross.y*cross.y + cross.z*cross.z;

    if (den < EPS)
        return false;

    double t1 = det(x, d2, cross) / den;
    double t2 = det(x, d1, cross) / den;

    r1 = o1 + d1 * t1;
    r2 = o2 + d2 * t2;

    return true;
}

// Convert from image to camera space by transforming point p in
// the image plane by the camera matrix.
CvPoint3D32f ImageCStoWorldCS(CvMatr32f camMat, const float camera_info[4][4], CvPoint2D32f * p){
    float tp[4];
    tp[0] = (float)p->x - camMat[2];//.principal_point.x;
    tp[1] = (float)p->y - camMat[5];//.principal_point.y;
    tp[2] = 1.f;
    tp[3] = 1.f;

    float tr[4];
    //multiply tp by mat to get tr
    MultVectorMatrix(tr, tp, camera_info);

    return cvPoint3D32f(tr[0]/tr[3], tr[1]/tr[3], tr[2]/tr[3]);

}

// Convert from camera to image space by transforming point p in
// the image plane by the camera matrix.
CvPoint2D32f WorldCStoImageCS(CvMatr32f camMat, const float camera_info[4][4], CvPoint3D32f * worldPoint){

					// FIND back project to find external
				int t;
				CvPoint2D32f x;
				float pointM[3];
				float tempPointM[3];
//				float pointCamera[3] = {1,1,1};


				CvMat cRotMatr = cvMat(3,3,CV_MAT32F,rotMatrix[texCamNum-1]);
				

	
				CvMat cPoint   = cvMat(3,1,CV_MAT32F,pointM);
				CvMat tempPoint   = cvMat(3,1,CV_MAT32F,tempPointM);
				CvMat cTransVect = cvMat(3,1,CV_MAT32F,transVects[texCamNum-1]);
		
				int currPoint;
				//x = (CvPoint2D32f *) createVector( 2 );
				
				//for( currPoint = 0; currPoint < 63; currPoint++ ){

				


					double invZ;
					CvMat point = cvMat(3,1,CV_MAT32F,worldPoint);
					cvMatMulAdd(&cRotMatr,&point,&cTransVect, &cPoint);
	//				mulMatrix(cRotMatr.data.fl, 3,3, point.data.fl, 1, 3, tempPoint.data.fl);
	//				addVector(point.data.fl, cTransVect.data.fl, cPoint.data.fl, 3);

				//	printMatrix((float *)cRotMatr.data.fl,3,3);

			//		fprintf(stderr, "^%f	%f	%f\n", point.data.fl[0], point.data.fl[1], point.data.fl[2]);
				//	printMatrix((float *)cTransVect.data.fl, 1,3);
		//			fprintf(stderr, "^%f	%f	%f\n", cPoint.data.fl[0], cPoint.data.fl[1], cPoint.data.fl[2]);



		float theta = 0.f;


		float fixRmat[3][3] ={ { -1,0,0},
								{ 0,1,0},
							{ 0,0,1}};
	
		//y axis
		float fixRmatY[3][3] = { {cos(theta), 0, sin(theta)},
								{ 0,1,0},
							{ -1*sin(theta), 0, cos(theta)}};

		//z axis
		float fixRmatZ[3][3] = { {cos(theta), -1*sin(theta), 0},
							{ sin(theta), cos(theta), 0},
							{ 0,0,1}};
							

		//	printMatrix(*fixRmatZ,3,3);
		//fprintf(stderr ,"\n*\n");
		//printMatrix(*rmat,3,3);
//		printMatrix(*tmat,3,1);
		//fprintf(stderr ,"\n=\n");

		float imgPt[3];

		/*
		 cPoint.data.fl[0] = cPoint.data.fl[0]/cPoint.data.fl[2];
		 cPoint.data.fl[1] = cPoint.data.fl[1]/cPoint.data.fl[2];
		 cPoint.data.fl[2] = cPoint.data.fl[2]/cPoint.data.fl[2];

		mulMatrix(*fixRmatZ, 3, 3, cPoint.data.fl, 1, 3, imgPt);
		*/

		imgPt[0] = cPoint.data.fl[0];
		 imgPt[1] = cPoint.data.fl[1];
		 imgPt[2] = cPoint.data.fl[2];

/*

					float theta = 90.f;

					cPoint.data.fl[0] = cPoint.data.fl[0] * cos(theta) + cPoint.data.fl[1] * -1.f* sin(theta);
					cPoint.data.fl[1] = cPoint.data.fl[0] * sin(theta) + cPoint.data.fl[1] * cos(theta);
					*/
					invZ  = 1.0 / imgPt[2];
					x.x = imgPt[0] * invZ;
					x.y = imgPt[1] * invZ;

/*
					float theta = -90.f;

					x.x = x.x * cos(theta) + x.y * -1.f* sin(theta);
					x.y = x.x * sin(theta) + x.y * cos(theta);

					*/



					

					
			//	fprintf(stderr, "%f	%f*****\n", (x.x * camMat[0]) - camMat[2], (x.y * camMat[4]) - camMat[5]);
				//}

				double tmp;

				CvVect32f r2 = createVector( 1);
				CvVect32f r4 = createVector( 1);


				//for( t = 0; t < 63; t++ ){
					tmp = (x.x * x.x) + (x.y * x.y);
					r2[0] = tmp;
					r4[0] = tmp * tmp;
				//}

				double radial;
				double tangX;
				double tangY;

//				fprintf(stderr, "%f	%f\n", x.x,x.y);
				//for(t = 0; t < 63; t++ ){

				// this is the good part!!!!
					radial = ((distortion[texCamNum-1][0] * r2[0]) + (distortion[texCamNum-1][1] * r4[0]));
					tangX = (2 * distortion[texCamNum-1][2] * x.x * x.y) + (distortion[texCamNum-1][3] * ( r2[0] + (2 *(x.x *x.x))));
					tangY = (2 * distortion[texCamNum-1][2] * x.x * x.y) + (distortion[texCamNum-1][3] * ( r2[0] + (2 *(x.y *x.y))));

					//fprintf(stderr, "%f	%f\n",((x.x + (x.x * radial) + tangX) * camMat[0]) + camMat[2], ((x.y + (x.y * radial) + tangY) * camMat[4]) + camMat[5]);  
					return cvPoint2D32f(((x.x + (x.x * radial) + tangX) * camMat[0]) + camMat[2], ((x.y + (x.y * radial) + tangY) * camMat[4]) + camMat[5]);


					/*float newX = ((x.x + (x.x * radial) + tangX) * camMat[0]) + camMat[2];
					float newY = ((x.y + (x.y * radial) + tangY) * camMat[4]) + camMat[5];

					float theta = 90.f;

					return cvPoint2D32f(newX * cos(theta) + (newY * -1.f* sin(theta)),newX * sin(theta) + newY * cos(theta));
				*/
						  



					//		return cvPoint2D32f(((x.x) * camMat[0]) + camMat[2], ((x.y) * camMat[4]) + camMat[5]);




					//imgPointBuffer[0].x = (x[0].x + (x[0].x * radial) + tangX) * camMat[0] + camMat[2];// + principal.x;
					//imgPointBuffer[t].y = (x[0].y + (x[0].y * radial) + tangY) * camMat[4] + camMat[5];// + principal.y;
				//}
				//END FIND

	/*
    float tp[4];
    tp[0] = (float)worldPoint->x;// - transVects[0];
    tp[1] = (float)worldPoint->y;// - transVects[1];
    tp[2] = (float)worldPoint->z;// - transVects[2];
    tp[3] = 1.f;

    float tr[4];
    //multiply tp by mat to get tr

	float inv_camera_info[4][4];
	invMatrix((float*)camera_info, 4, (float*)inv_camera_info);
	//invert camera_info ??
    MultVectorMatrix(tr, tp, inv_camera_info);
//	printMatrix((float *)camera_info,4,4);
//	printMatrix((float *)inv_camera_info,4,4);

	fprintf(stderr, "%f	%f	%f	%f\n", tr[0], tr[1], tr[2], tr[3]);

    return cvPoint2D32f(((tr[0]/tr[2]) *camMat[0])+ camMat[2], ((tr[1]/tr[2]) * camMat[4])+ camMat[5]);
	*/
				
}


// Multiply affine transformation m1 by the affine transformation m2 and
// return the result in rm.
static void MultMatrix(float rm[4][4], const float m1[4][4], const float m2[4][4]){
    for (int i=0; i<=3; i++)
        for (int j=0; j<=3; j++){
            rm[i][j]= 0.0;
            for (int k=0; k <= 3; k++)
                rm[i][j] += m1[i][k]*m2[k][j];
        }
}

// Multiply the vector v by the affine transformation matrix m and return the
// result in rv.
void MultVectorMatrix(float rv[4], const float v[4], const float m[4][4]){
    for (int i=0; i<=3; i++){
        rv[i] = 0.f;
        for (int j=0;j<=3;j++)
            rv[i] += v[j] * m[j][i];
    }
}



void loadConfig(char * configFile){

	int junk;

	FILE *fp;

	if((fp = fopen(configFile, "rb")) == NULL){
		fprintf(stderr, "could not open %s\nPlease check parameter\n", configFile);
		exit(1);
	}

	else{
		
		//while(!feof(fp)){

			fscanf(fp, "Scan Path: %s\n", &scanPath);
			fscanf(fp, "Scan Name: %s\n", &scanName);
			fscanf(fp, "Start Image Number: %d\n", &startImages);
			fscanf(fp, "Finish Image Number: %d\n", &numImages);


			fscanf(fp, "1 Camera Number: %d\n", &order[0]);
			fscanf(fp, "Top Left Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Bottom Right Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Line Threshold: %d\n", &junk);
			fscanf(fp, "Line Contrast: %d\n", &junk);
			fscanf(fp, "Strip Threshold: %d\n", &junk);
			fscanf(fp, "Strip Contrast: %d\n", &junk);

			fscanf(fp, "2 Camera Number: %d\n", &order[1]);
			fscanf(fp, "Top Left Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Bottom Right Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Line Threshold: %d\n", &junk);
			fscanf(fp, "Line Contrast: %d\n", &junk);
			fscanf(fp, "Strip Threshold: %d\n", &junk);
			fscanf(fp, "Strip Contrast: %d\n", &junk);

			fscanf(fp, "3 Camera Number: %d\n", &order[2]);
			fscanf(fp, "Top Left Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Bottom Right Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Line Threshold: %d\n", &junk);
			fscanf(fp, "Line Contrast: %d\n", &junk);
			fscanf(fp, "Strip Threshold: %d\n", &junk);
			fscanf(fp, "Strip Contrast: %d\n", &junk);

			fscanf(fp, "4 Camera Number: %d\n", &order[3]);
			fscanf(fp, "Top Left Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Bottom Right Corner: %d	%d\n", &junk, &junk);
			fscanf(fp, "Line Threshold: %d\n", &junk);
			fscanf(fp, "Line Contrast: %d\n", &junk);
			fscanf(fp, "Strip Threshold: %d\n", &junk);
			fscanf(fp, "Strip Contrast: %d\n", &junk);

		//}	
			
		

		fflush(fp);
		fclose(fp);
	}

	int i = 0;
	numberOfCameras = 0;
	while((order[i] > -1) && (i < 4)){
		numberOfCameras++;
		i++;
	}
}



