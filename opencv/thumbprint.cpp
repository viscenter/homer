
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <string>
#include <vector>

#define DEBUG

#define DIRTY_THRESH 105
#define CLEAN_THRESH 110

CvSize camSize;

int findCorners( IplImage * im, CvSize dim, CvPoint2D32f * corners );
int findAllCorners( const char * dir, CvSize dim, CvMat ** corners );
bool loadCalib( const char * file, CvMat * K, CvMat * d );
bool saveCalib( const char * file, const CvMat * K, const CvMat * d );
bool calibrate( const char * dir, CvMat **K, CvMat **d,
		CvSize dim = cvSize(7,7), float size = 25 );
int readCheckers( const char * file, const char * image, CvSize dim,
    CvMat ** checkers, CvMat ** image_checkers );
bool loadExtrinsics( const char * filename, CvMat ** r, CvMat ** t );
bool extrinsics( const char * file, const char * image, CvSize dim,
    const CvMat *K, const CvMat *d, CvMat ** r, CvMat ** t );
bool mapTexture( const char * file, const char * thumb_file, const char * clean_file, CvMat *K, CvMat *d, CvMat *r, CvMat *t );

int main( int argc, char * argv[] )
{
	CvMat *K = NULL, *d = NULL, *r = NULL, *t = NULL;

	if( argc < 7 )
	{
		fprintf( stderr, "%s directory checkers image object thumb nonthumb\n", argv[0] );
		return 1;
	}
	char * calib_dir = argv[1];
	char * checkers  = argv[2];
	char * check_img = argv[3];
	char * obj_file  = argv[4];
	char * thumb_img = argv[5];
	char * clean_img = argv[6];

	if( !calibrate( calib_dir, &K, &d ) )
	{
		printf("Calibration failed\n");
		return -1;
	}
	printf("Intrinsic:\n");
	for( int i=0; i<3; ++i )
		printf("%0.6f %0.6f %0.6f\n",
				K->data.fl[3*i+0], K->data.fl[3*i+1], K->data.fl[3*i+2] );
	printf("Distortion:\n");
	printf("%0.6f %0.6f %0.6f %0.6f\n",
			d->data.fl[0], d->data.fl[1], d->data.fl[2], d->data.fl[3] );
	printf("Calibration successful\n");

	if( !extrinsics( checkers, check_img, cvSize(7,7), K, d, &r, &t ) )
	{
		printf("Extrinsics failed\n");
		return -2;
	}
	printf("R %0.6f %0.6f %0.6f\n", r->data.fl[0], r->data.fl[1], r->data.fl[2] );
	printf("T %0.6f %0.6f %0.6f\n", t->data.fl[0], t->data.fl[1], t->data.fl[2] );
	printf("Found extrinsics\n");

	if( !mapTexture( obj_file, thumb_img, clean_img, K, d, r, t ) )
	{
		printf("Failed to map texture\n");
		return -3;
	}
	printf("Mapped texture\n");

	printf("Success\n");
	return 0;
}

int findCorners( IplImage * im, CvSize dim, CvPoint2D32f * corners )
{
  int count = dim.width*dim.height;
  bool found = cvFindChessboardCorners( im, dim, corners, &count );
  CvSize subpix = cvSize(5,5);
  if( !found && im->width > 100 )
  {
    IplImage * im2 =
      cvCreateImage( cvSize(im->width/2,im->height/2), IPL_DEPTH_8U, 1 );
    cvPyrDown( im, im2 );
    count = findCorners( im2, dim, corners );
    cvReleaseImage( &im2 );
    for( int i=0; i<dim.width*dim.height; ++i )
    {
      corners[i].x *= 2;
      corners[i].y *= 2;
    }
    subpix = cvSize(11,11);
  }

  if( found )
  {
    CvPoint2D32f tmp;
    if( corners[0].x + corners[0].y >
        corners[dim.width*dim.height-1].x + corners[dim.width*dim.height-1].y )
    {
      for( int i=0; i<dim.width*dim.height/2; ++i )
      {
        tmp = corners[i];
        corners[i] = corners[dim.width*dim.height-1-i];
        corners[dim.width*dim.height-1-i] = tmp;
      }
    }
    if( ( corners[dim.width-1].x - corners[0].x <
          corners[dim.width*(dim.height-1)].x - corners[0].x ) ||
        ( corners[dim.width-1].y - corners[0].y >
          corners[dim.width*(dim.height-1)].y - corners[0].y ) )
    {
      for( int x=0; x<dim.width; ++x )
      {
        for( int y=x; y<dim.height; ++y )
        {
          tmp = corners[x*dim.width+y];
          corners[x*dim.width+y] = corners[y*dim.width+x];
          corners[y*dim.width+x] = tmp;
        }
      }
    }
    if( corners[0].x > corners[dim.width-1].x )
    {
      for( int x=0; x<dim.width; ++x )
      {
        for( int y=0; y<dim.height/2; ++y )
        {
          tmp = corners[x*dim.width+y];
          corners[x*dim.width+y] = corners[(x+1)*dim.width-y-1];
          corners[(x+1)*dim.width-y-1] = tmp;
        }
      }
    }
    if( corners[0].y > corners[dim.width*(dim.height-1)].y )
    {
      for( int x=0; x<dim.width; ++x )
      {
        for( int y=0; y<dim.height/2; ++y )
        {
          tmp = corners[y*dim.width+x];
          corners[y*dim.width+x] = corners[(dim.height-1-y)*dim.width+x];
          corners[(dim.height-1-y)*dim.width+x] = tmp;
        }
      }
    }
  }

  cvFindCornerSubPix( im, corners, count, subpix, cvSize(-1,-1),
      cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001) );
  return count;
}

int select( const struct dirent * d )
{
	return strncmp( d->d_name, "Image", 5 ) == 0;
}

int findAllCorners( const char * dir, CvSize dim, CvMat ** points )
{
	printf("Finding all corners on %s\n",dir);
	struct dirent ** files = NULL;
	int c = scandir( dir, &files, select, alphasort );

	printf("c: %d\n",c);
  std::vector<CvPoint2D32f *> point_stack;
  // read directory
  // for each file in directory
	for( int f=0; f<c; ++f )
  {
		std::string file = dir; file += '/'; file += files[f]->d_name;
		free( files[f] );

		printf("Trying to load %s\n",file.c_str());
    IplImage * im = cvLoadImage( file.c_str(), CV_LOAD_IMAGE_GRAYSCALE );
		if( !im ) continue;
		printf("Loading %s\n", file.c_str() );

    camSize = cvSize(im->width,im->height);
    CvPoint2D32f * corners = new CvPoint2D32f[dim.width*dim.height];
    int count = findCorners( im, dim, corners );
    bool found = ( count == dim.width*dim.height );
    cvReleaseImage( &im );
		printf("Found %d corners\n", count);

    if( found )
		{
      point_stack.push_back( corners );
			printf("Found corners in %s\n", file.c_str() );
		}
    else
      delete [] corners;

#ifdef DEBUG
    //cvNamedWindow("calibrate");
    IplImage * tmp1 = cvLoadImage( file.c_str(), CV_LOAD_IMAGE_COLOR );
    IplImage * tmp2 =
      cvCreateImage( cvSize(tmp1->width/2,tmp1->height/2), IPL_DEPTH_8U, 3 );
    cvPyrDown( tmp1, tmp2 );
    cvReleaseImage( &tmp1 );
    IplImage * tmp3 =
      cvCreateImage( cvSize(tmp2->width/2,tmp2->height/2), IPL_DEPTH_8U, 3 );
    cvPyrDown( tmp2, tmp3 );
    cvReleaseImage( &tmp2 );

    CvPoint2D32f * tmp_corners = new CvPoint2D32f[count];
    for( int i=0; i<count; ++i )
    {
      tmp_corners[i].x = corners[i].x/4;
      tmp_corners[i].y = corners[i].y/4;
    }
    cvDrawChessboardCorners( tmp3, dim, tmp_corners, count, found );
    delete [] tmp_corners;
    //cvShowImage( "calibrate", tmp3 );
		cvSaveImage("temp.jpg",tmp3);
    cvReleaseImage( &tmp3 );
    //cvWaitKey( 100 );
#endif
  }
	free(files);
  if( point_stack.empty() ) return 0;

  *points =
    cvCreateMat( point_stack.size()*dim.width*dim.height, 2, CV_32FC1 );
  for( int i=0; i<point_stack.size(); ++i )
  {
    for( int j=0; j<dim.width*dim.height; ++j )
    {
      (*points)->data.fl[2*(i*dim.width*dim.height+j)+0] = point_stack[i][j].x;
      (*points)->data.fl[2*(i*dim.width*dim.height+j)+1] = point_stack[i][j].y;
    }
    delete [] point_stack[i];
  }

  return point_stack.size();
}

bool loadCalib( const char * file, CvMat * K, CvMat * d )
{
  FILE * f = fopen( file, "r" );
  if( f )
  {
    fscanf( f, "%d %d\n", &camSize.width, &camSize.height );
    for( int i=0; i<3; ++i )
      fscanf( f, "%f %f %f\n",
          &K->data.fl[3*i+0], &K->data.fl[3*i+1], &K->data.fl[3*i+2] );
    fscanf( f, "%f %f %f %f\n",
        &d->data.fl[0], &d->data.fl[1], &d->data.fl[2], &d->data.fl[3] );
    fclose(f);

    return true;
  }
  return false;
}

bool saveCalib( const char * file, const CvMat * K, const CvMat * d )
{
  FILE * f = fopen( file, "w" );
  if( f )
  {
    fprintf( f, "%d %d\n", camSize.width, camSize.height );
    for( int i=0; i<3; ++i )
      fprintf( f, "%f %f %f\n",
          K->data.fl[3*i+0], K->data.fl[3*i+1], K->data.fl[3*i+2] );
    fprintf( f, "%f %f %f %f\n",
        d->data.fl[0], d->data.fl[1], d->data.fl[2], d->data.fl[3] );
    fclose(f);

    return true;
  }
  return false;
}

bool calibrate( const char * dir, CvMat **K, CvMat **d, CvSize dim, float size )
{
  if( !*K ) *K = cvCreateMat( 3, 3, CV_32FC1 );
  if( !*d ) *d = cvCreateMat( 4, 1, CV_32FC1 );

  std::string calibfile = dir; calibfile += "/calib.dat";
  if( loadCalib( calibfile.c_str(), *K, *d ) )
    return true;

  CvMat * points;
  int count = findAllCorners( dir, dim, &points );
  if( count )
  {
    CvMat * world = cvCreateMat( count*dim.width*dim.height, 3, CV_32FC1 );
    for( int i=0; i<count; ++i )
    {
      for( int x=0; x<dim.width; ++x )
      {
        for( int y=0; y<dim.height; ++y )
        {
          world->data.fl[3*(i*dim.width*dim.height+y*dim.height+x)+0] = x*size;
          world->data.fl[3*(i*dim.width*dim.height+y*dim.height+x)+1] = y*size;
          world->data.fl[3*(i*dim.width*dim.height+y*dim.height+x)+2] = 0;
        }
      }
    }

    CvMat * counts = cvCreateMat( 1, count, CV_32S );
    cvSet( counts, cvScalar(dim.width*dim.height) );

    cvCalibrateCamera2( world, points, counts, camSize, *K, *d );
    saveCalib( calibfile.c_str(), *K, *d );
    return true;
  }

  cvReleaseMat( K );
  cvReleaseMat( d );
  return false;
}

int readCheckers( const char * file, const char * image, CvSize dim,
    CvMat ** checkers, CvMat ** image_checkers )
{
  std::vector<CvPoint3D32f> check;

  FILE * f = fopen( file, "r" );
  char buf[1024];
  CvPoint3D32f last = cvPoint3D32f( 0, 0, 0 );
  bool reversed = false, transposed = false, mirrored = false;
	while( f && !feof(f) )
  {
    fgets( buf, sizeof(buf), f );
    if( *buf != '#' )
    {
      if( *buf == 'v' )
      {
        CvPoint3D32f c;
        sscanf( buf, "v %f %f %f\n", &c.x, &c.y, &c.z );
				printf( "%f %f %f\n",c.x,c.y,c.z);

        // If within 2 mm of last point, assume duplicate
        if( (last.x-c.x)*(last.x-c.x)+
            (last.y-c.y)*(last.y-c.y)+
            (last.z-c.z)*(last.z-c.z) < 4. )
        {
          // To properly orient the Faro coordinates with the image,
          // scan the second point from the left on the top row,
          // as seen from the camera, twice.  We then determine the
          // appropriate transformation to line up the two point sets.
          switch( check.size() )
          {
            case 2:
              reversed = false; transposed = false; mirrored = false; break;
            case 3:
              reversed = false; transposed = false; mirrored = true; break;
            case 5:
              reversed = false; transposed = true; mirrored = false; break;
            case 8:
              reversed = false; transposed = true; mirrored = true; break;
            case 9:
              reversed = true; transposed = true; mirrored = true; break;
            case 12:
              reversed = true; transposed = true; mirrored = false; break;
            case 14:
              reversed = true; transposed = false; mirrored = true; break;
            case 15:
              reversed = true; transposed = false; mirrored = false; break;
            default:
              printf("ignoring duplicate point\n");
          }
        }
        // Add to list
        else
        {
          check.push_back( c );
        }
        last = c;
      }
    }
  }
  fclose(f);

  float size = sqrt(check.size());
  if( size && size == (int)size )
  {
		printf("Size %f\n",size);
    if( reversed )
    {
      printf("Reversed\n");
      for( size_t i=0; i<check.size()/2; ++i )
      {
        CvPoint3D32f p = check[i];
        check[i] = check[check.size()-i-1];
        check[check.size()-i-1] = p;
      }
    }
    if( mirrored )
    {
      printf("Mirrored\n");
      for( size_t i=0; i<size; ++i )
      {
        for( size_t j=0; j<size/2; ++j )
        {
          CvPoint3D32f p = check[(size_t)(i*size+j)];
          check[(size_t)(i*size+j)] = check[(size_t)((i+1)*size-j-1)];
          check[(size_t)((i+1)*size-j-1)] = p;
        }
      }
    }
    if( transposed )
    {
      printf("Transposed\n");
      for( int i=0; i<size; ++i )
      {
        for( int j=i; j<size; ++j )
        {
          CvPoint3D32f p = check[(size_t)(i*size+j)];
          check[(size_t)(i*size+j)] = check[(size_t)(j*size+i)];
          check[(size_t)(j*size+i)] = p;
        }
      }
    }

    int dist = ((int)size-1)*(int)sqrt((double)check.size());
    CvPoint3D32f vect = cvPoint3D32f(
        ( check[check.size()-1].x - check[0].x ) /dist,
        ( check[check.size()-1].y - check[0].y ) /dist,
        ( check[check.size()-1].z - check[0].z ) /dist );
    for( int i=0; i<check.size(); ++i )
    {
      check[i].x += vect.x;
      check[i].y += vect.y+1;
      check[i].z += vect.z;
    }

    IplImage * im = cvLoadImage( image, CV_LOAD_IMAGE_GRAYSCALE );
    CvPoint2D32f * corners = new CvPoint2D32f[dim.width*dim.height];
    int count = findCorners( im, dim, corners );
		printf("Found %d corners\n",count);
		cvReleaseImage( &im );
		if( sqrt(count) == size*2-1 )
    {
      if( !*checkers ) *checkers = cvCreateMat( check.size(), 3, CV_32FC1 );
      if( !*image_checkers )
        *image_checkers = cvCreateMat( check.size(), 2, CV_32FC1 );

			for( int i=0; i<check.size(); ++i )
			{
				(*checkers)->data.fl[3*i+0] = check[i].x;
				(*checkers)->data.fl[3*i+1] = check[i].y;
				(*checkers)->data.fl[3*i+2] = check[i].z;
			}
			int i=0;
      for( int y=0; y<dim.height; y+=2 )
      {
        for( int x=0; x<dim.width; x+=2 )
        {
					(*image_checkers)->data.fl[2*i+0] = corners[y*dim.width+x].x;
					(*image_checkers)->data.fl[2*i+1] = corners[y*dim.width+x].y;
					++i;
        }
      }
    }
		else
		{
			check.clear();
		}
		delete [] corners;
  }
	else
	{
		printf("not square\n");
		check.clear();
	}

  return check.size();
}

bool loadExtrinsics( const char * filename, CvMat ** r, CvMat ** t )
{
	FILE * extrinsics = fopen(
			filename, "r" );

	if( extrinsics ) {
		fclose(extrinsics);
		*r = (CvMat*)cvLoad( filename, NULL, "R", NULL);
		*t = (CvMat*)cvLoad( filename, NULL, "T", NULL);
		return true;
	}

	return false;
}

bool extrinsics( const char * file, const char * image, CvSize dim,
    const CvMat *K, const CvMat *d, CvMat ** r, CvMat ** t )
{
	CvMat * checkers = NULL;
	CvMat * image_checkers = NULL;

	char * image_nodir = strrchr(image,'/');
	if(image_nodir == NULL) {
		image_nodir = (char *)image;
	}
	else {
		image_nodir += 1; // get rid of beginning slash
	}

	std::string extrinsicsname = (std::string(file).substr(0,strlen(file)-4)+"-"+
			std::string(image_nodir).substr(0,strlen(image_nodir)-3)+"extrinsics");

	printf("Extrinsics file: %s\n", extrinsicsname.c_str());

	if( loadExtrinsics( extrinsicsname.c_str(), r, t ) ) {
		printf("Loaded extrinsics from file\n");
		return true;
	}

	int size = readCheckers( file, image, dim, &checkers, &image_checkers );
	printf("Read %d checkers\n",size);
	if( size )
	{
		if( !*r ) *r = cvCreateMat( 3, 1, CV_32FC1 );
		if( !*t ) *t = cvCreateMat( 3, 1, CV_32FC1 );
		for( int i=0; i<size; ++i )
			printf("%f %f %f\n",checkers->data.fl[3*i],checkers->data.fl[3*i+1],checkers->data.fl[3*i+2] );
		for( int i=0; i<size; ++i )
			printf("%f %f\n",image_checkers->data.fl[2*i], image_checkers->data.fl[2*i+1] );
		cvFindExtrinsicCameraParams2( checkers, image_checkers, K, d, *r, *t );

#ifdef DEBUG
		IplImage * im = cvLoadImage( image, CV_LOAD_IMAGE_COLOR );
		//cvNamedWindow("calibrate");
		CvMat * img_check = cvCreateMat( size, 2, CV_32FC1 );
		cvProjectPoints2( checkers, *r, *t, K, d, img_check );
		cvDrawChessboardCorners( im, cvSize((int)sqrt(size),(int)sqrt(size)),
				(CvPoint2D32f*)img_check->data.fl, size, true );
		cvReleaseMat( &img_check );
		//cvShowImage( "calibrate", im );
		cvSaveImage("temp.jpg",im);
		cvReleaseImage( &im );
		//cvWaitKey(100);
#endif

		cvReleaseMat( &checkers );
		cvReleaseMat( &image_checkers );

		CvFileStorage * fs = cvOpenFileStorage( extrinsicsname.c_str(), 0, CV_STORAGE_WRITE );
		cvWrite( fs, "R", *r );
		cvWrite( fs, "T", *t );
		cvReleaseFileStorage( &fs );

		return true;
	}
	return false;
}

bool mapTexture( const char * file, const char * thumb_file, const char * clean_file, CvMat *K, CvMat *d, CvMat *r, CvMat *t )
{
	FILE * input = fopen( file, "r" );
	FILE * output = fopen(
			(std::string(file).substr(0,strlen(file)-3)+"surf").c_str(), "w" );

	IplImage * dirty = cvLoadImage( thumb_file, CV_LOAD_IMAGE_GRAYSCALE );
	IplImage * clean = cvLoadImage( clean_file, CV_LOAD_IMAGE_GRAYSCALE );

	if( input && output && dirty && clean)
	{
		int verts, tris;
		char buf[1024];
		fgets( buf, sizeof(buf), input );
		fgets( buf, sizeof(buf), input );
		fgets( buf, sizeof(buf), input );
		sscanf( buf, "# Number of geometric vertices: %d\n", &verts );

		CvMat * points = cvCreateMat( verts, 3, CV_32FC1 );
		CvMat * texture = cvCreateMat( verts, 2, CV_32FC1 );
		for( int i=0; !feof(input) && i<verts; )
		{
			fgets( buf, sizeof(buf), input );
			if( *buf == 'v' )
			{
				sscanf( buf, "v %f %f %f",
						&points->data.fl[3*i+0],
						&points->data.fl[3*i+1],
						&points->data.fl[3*i+2] );
				++i;
			}
		}
		printf("projecting points\n");
		cvProjectPoints2( points, r, t, K, d, texture );
		printf("size %d %d\n", camSize.width, camSize.height);
		CvMat * rot = cvCreateMat( 3, 3, CV_32FC1 );
		CvMat * rod = cvCreateMat( 3, 1, CV_32FC1 );
		rod->data.fl[0] = 0; rod->data.fl[1] = 0; rod->data.fl[2] = -.5;
		cvRodrigues2( rod, rot );
		cvMatMul( points, rot, points );
		cvReleaseMat( &rod );
		cvReleaseMat( &rot );

		CvMat * blank_intrinsics = cvCreateMat( 3, 3, CV_32FC1 );
		cvSetIdentity( blank_intrinsics );
		CvMat * second_r = cvCreateMat( 3, 1, CV_32FC1 );
		CvMat * second_t = cvCreateMat( 3, 1, CV_32FC1 );
		
		CvRect original_bound = cvRect( dirty->width, dirty->height, 0, 0); // bounding box of original point set
		for( int i = 0; i < verts; i++ ) {
			int cx =(int)(texture->data.fl[2*i+0]);
			int cy =(int)(texture->data.fl[2*i+1]);

			original_bound.x = original_bound.x < cx ? original_bound.x : cx;
			original_bound.y = original_bound.y < cy ? original_bound.y : cy;
			original_bound.width = original_bound.width > cx ? original_bound.width : cx;
			original_bound.height = original_bound.height > cy ? original_bound.height : cy;
		}
		original_bound.width -= original_bound.x;
		original_bound.height -= original_bound.y;
		CvRect unmodified_bound = original_bound;
		printf( "Bound: %d, %d, %d, %d\n", original_bound.x, original_bound.y, original_bound.width, original_bound.height );
		// cvReleaseMat( &texture_i );
		/*
		for( int i=0; i<size; ++i )
			printf("%f %f %f\n",checkers->data.fl[3*i],checkers->data.fl[3*i+1],checkers->data.fl[3*i+2] );
		for( int i=0; i<size; ++i )
		 	printf("%f %f\n",image_checkers->data.fl[2*i], image_checkers->data.fl[2*i+1] );
		*/


		// thresh the images
		cvThreshold( dirty, dirty, DIRTY_THRESH, 255, CV_THRESH_BINARY);
		cvThreshold( clean, clean, CLEAN_THRESH, 255, CV_THRESH_BINARY);
		
		cvResetImageROI( dirty );
		IplImage * dirty_orig = cvCreateImage( cvSize(dirty->width, dirty->height), dirty->depth, dirty->nChannels );
		cvCopy( dirty, dirty_orig );

		// do connected component analysis first
		int bind_offset = 200;
		original_bound.x += bind_offset;
		original_bound.y += bind_offset;
		original_bound.width -= bind_offset;
		original_bound.height -= (bind_offset*3);
		cvSaveImage("threshedunbound.jpg", dirty);
		cvSetImageROI( dirty, original_bound );
		cvSaveImage("threshedbound.jpg", dirty);
		cvSaveImage("threshedclean.jpg", clean);
		CvMemStorage * storage = cvCreateMemStorage(0);
		CvSeq * contours;
		int numcontours = cvFindContours( dirty, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint( original_bound.x, original_bound.y ) );
		cvReleaseImage( &dirty );
	
		// we'll have up to one point correspondence for each contour
		CvMat * dirty_points_i = cvCreateMat( numcontours, 3, CV_32FC1 );
    CvMat * clean_points_i = cvCreateMat( numcontours, 2, CV_32FC1 );

		double curmin, curmax;
		CvPoint minpoint, maxpoint;
		double curarea;
		int curcontour = 0;
		int offset = 200;
		
		while( contours ) {
			curarea = fabs(cvContourArea(contours));
			if((curarea > 5000.0) && (curarea < 10000.0)) {
				// CvSeq * approxp = cvApproxPoly( contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter( contours ) * 0.001, 0 );
				CvPoint firstp = *((CvPoint*)cvGetSeqElem( contours, 0 ));
				// firstp = ((CvChain*)contours)->origin;
				// printf("%d,%d:\t%d\n",firstp.x,firstp.y,(int)curarea);
				cvResetImageROI( dirty_orig );
				if( ((int)cvGetReal2D( dirty_orig, firstp.y+1, firstp.x+1 )) == 0 /*true*/ ) {
					printf("Area: %f\n",curarea);
					// CvSeq * result = 
					CvRect bounding_rect = cvBoundingRect(contours,0);
					//CvRect bounding_rect = ((CvContour*)contours)->rect;
					// printf("%d,%d,%d,%d\n",bounding_rect.x,bounding_rect.y,bounding_rect.width,bounding_rect.height);
				
					CvRect cursize = cvRect(bounding_rect.x - offset, bounding_rect.y - offset, bounding_rect.width + (offset * 2), bounding_rect.height + (offset * 2) );
					IplImage * result = cvCreateImage( cvSize( cursize.width - bounding_rect.width + 1, cursize.height - bounding_rect.height + 1), IPL_DEPTH_32F, 1 );	

					cvSetImageROI( dirty_orig, bounding_rect );
					cvSetImageROI( clean, cursize );

					cvMatchTemplate( clean, dirty_orig, result, CV_TM_SQDIFF_NORMED );

					/*
					cvNamedWindow("Dirty",0);
					cvNamedWindow("Clean",0);
					cvNamedWindow("Result",0);
					cvShowImage("Dirty",dirty_orig);
					cvShowImage("Clean",clean);
					cvShowImage("Result",result);
					*/

					cvMinMaxLoc( result, &curmin, &curmax, &minpoint, &maxpoint );

					// printf("%d,%d\n", minpoint.x, minpoint.y);

					// cvWaitKey(0);
					// cvDestroyAllWindows();
					
					dirty_points_i->data.fl[3*curcontour+0] = bounding_rect.x + (bounding_rect.width / 2);
					dirty_points_i->data.fl[3*curcontour+1] = bounding_rect.y + (bounding_rect.height / 2);
					dirty_points_i->data.fl[3*curcontour+2] = 0;
					clean_points_i->data.fl[2*curcontour+0] = cursize.x + minpoint.x + (bounding_rect.width / 2);
					clean_points_i->data.fl[2*curcontour+1] = cursize.y + minpoint.y + (bounding_rect.height / 2);
					
					printf("%d,%d\t->\t%d,%d\n",(int)dirty_points_i->data.fl[3*curcontour+0],(int)dirty_points_i->data.fl[3*curcontour+1],
						(int)clean_points_i->data.fl[2*curcontour+0],(int)clean_points_i->data.fl[2*curcontour+1]);

					cvReleaseImage( &result );
					curcontour++;
				}
			}
			contours = contours->h_next;
		}
		cvReleaseMemStorage( &storage );

		printf( "%d correspondences\n", curcontour );
		// copy out only the point correspondences we generated
		CvMat * dirty_points = cvCreateMat( curcontour, 3, CV_32FC1 );
    CvMat * clean_points = cvCreateMat( curcontour, 2, CV_32FC1 );

		for(int i = 0; i < curcontour; i++) {
			dirty_points->data.fl[3*i+0] = dirty_points_i->data.fl[3*i+0];
			dirty_points->data.fl[3*i+1] = dirty_points_i->data.fl[3*i+1];
			dirty_points->data.fl[3*i+2] = dirty_points_i->data.fl[3*i+2];
			clean_points->data.fl[2*i+0] = clean_points_i->data.fl[2*i+0];
			clean_points->data.fl[2*i+1] = clean_points_i->data.fl[2*i+1];
		}

		cvReleaseMat( &dirty_points_i );
		cvReleaseMat( &clean_points_i );

		cvFindExtrinsicCameraParams2( dirty_points, clean_points, blank_intrinsics, NULL, second_r, second_t );
		// cvFindExtrinsicCameraParams2( dirty_points, clean_points, K, d, second_r, second_t );
	
		printf("R %0.6f %0.6f %0.6f\n", second_r->data.fl[0], second_r->data.fl[1], second_r->data.fl[2] );
		printf("T %0.6f %0.6f %0.6f\n", second_t->data.fl[0], second_t->data.fl[1], second_t->data.fl[2] );

		cvReleaseMat( &dirty_points );
		cvReleaseMat( &clean_points );

		CvMat * texture_3d = cvCreateMat( verts, 3, CV_32FC1 );
		for( int i=0; i<verts; ++i ) {
			texture_3d->data.fl[3*i+0] = texture->data.fl[2*i+0];
			texture_3d->data.fl[3*i+1] = texture->data.fl[2*i+1];
			texture_3d->data.fl[3*i+2] = 0;
		}
		CvMat * clean_texture = cvCreateMat( verts, 2, CV_32FC1 );
		cvProjectPoints2( texture_3d, second_r, second_t, blank_intrinsics, NULL, clean_texture );
		// cvProjectPoints2( texture_3d, second_r, second_t, K, d, clean_texture );

		cvReleaseMat( &texture_3d );
		
		cvResetImageROI( dirty_orig );
		cvSaveImage( "dirty.jpg", dirty_orig );

		cvReleaseImage( &dirty_orig );
		cvReleaseImage( &clean );

		IplImage * clean_remapped = cvLoadImage( thumb_file, CV_LOAD_IMAGE_COLOR );
		clean = cvLoadImage( clean_file, CV_LOAD_IMAGE_COLOR );
		
		cvResetImageROI( clean );

		CvMat * all_points; 
		CvMat * all_points_proj;
		
		int thisx = 0;
		int thisy = 0;
		int tx = 0;
		int ty = 0;

		printf("Projecting all points...\n");
	
		int maxsize = 8192*4;
	
		bool project_points = true;
		int passes = 0;	
		while(project_points) {	
			// printf("Pass %d\n",++passes);
			all_points = cvCreateMat( maxsize, 3, CV_32FC1 );

			for( int i = 0; i < maxsize; i++ ) {
					if(project_points) {
						all_points->data.fl[3*i+0] = thisx;
						all_points->data.fl[3*i+1] = thisy;
					}
					else {
						all_points->data.fl[3*i+0] = 0;
						all_points->data.fl[3*i+1] = 0;
					}	
					all_points->data.fl[3*i+2] = 0;
					
					thisx++;
					if(thisx >= clean_remapped->width) {
						thisx = 0;
						thisy++;
						if(thisy >= clean_remapped->height) {
							project_points = false;
						}
					}
			}
			
			all_points_proj	= cvCreateMat( maxsize, 2, CV_32FC1 );
			
			cvProjectPoints2( all_points, second_r, second_t, blank_intrinsics, NULL, all_points_proj );
			
			for( int i = 0; i < maxsize; i++ ) {
				tx = (int)all_points->data.fl[3*i+0];
				ty = (int)all_points->data.fl[3*i+1];
 
				int thatx = (int)all_points_proj->data.fl[2*i+0];
				int thaty = (int)all_points_proj->data.fl[2*i+1];
				if( (thatx > 0) && (thatx < clean->width) && (thaty > 0) && (thaty < clean->height)) {
					cvSet2D( clean_remapped, ty, tx, cvGet2D( clean, thaty, thatx ));
				}
			}
			cvReleaseMat( &all_points );
			cvReleaseMat( &all_points_proj );
		}
		
		cvSaveImage( "remapped.jpg", clean_remapped );

		cvReleaseImage( &clean );

		storage = cvCreateMemStorage();
		CvPoint * texture_conv_stor = (CvPoint*)malloc(verts*sizeof(CvPoint));
		CvPoint * hullmat_stor = (CvPoint*)malloc(verts*sizeof(CvPoint));
		CvMat texture_conv = cvMat( 1, verts, CV_32SC2, texture_conv_stor );
		for(int i = 0; i < verts; i++) {
			 texture_conv_stor[i] = cvPoint((int)texture->data.fl[2*i+0],(int)texture->data.fl[2*i+1]);
		}
		/*
		CvSeq * ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage );
		for(int i = 0; i < verts; i++) {
			CvPoint temppoint;
			temppoint.x = (int)texture->data.fl[2*i+0];
			temppoint.y = (int)texture->data.fl[2*i+1];

			cvSeqPush( ptseq, &temppoint );
		}
		CvSeq * hullseq = cvConvexHull2( texture_conv, 0, CV_CLOCKWISE, 0);
		*/
		CvMat hullmat = cvMat( 1, verts, CV_32SC2, hullmat_stor );
		cvConvexHull2( &texture_conv, &hullmat, CV_CLOCKWISE, 1);

		dirty = cvLoadImage( thumb_file, CV_LOAD_IMAGE_COLOR );
		IplImage * dirty_conhull = cvCreateImage(cvSize(unmodified_bound.width,unmodified_bound.height), IPL_DEPTH_8U, 3);
		
		cvSet( dirty_conhull, cvScalarAll(255) );

		IplImage * clean_remapped_conhull = cvCreateImage(cvSize(unmodified_bound.width,unmodified_bound.height), IPL_DEPTH_8U, 3);
		cvSet( clean_remapped_conhull, cvScalarAll(255) );
		
		// now for each pixel in the bounding box, check if it's in the convex hull
		for(int y = 0; y < unmodified_bound.height; y++) {
			int origy = y + unmodified_bound.y;
			for(int x = 0; x < unmodified_bound.width; x++) {
				int origx = x + unmodified_bound.x;

				if(cvPointPolygonTest( &hullmat, cvPointTo32f( cvPoint( origx, origy ) ), 0 ) == 100) {
					cvSet2D( dirty_conhull, y, x, cvGet2D( dirty, origy, origx ) );
					cvSet2D( clean_remapped_conhull, y, x, cvGet2D( clean_remapped, origy, origx ) );
				}
			}
		}
	 	cvReleaseMemStorage( &storage );

		cvSaveImage( "conhull-clean.jpg", clean_remapped_conhull );	
		cvSaveImage( "conhull-dirty.jpg", dirty_conhull );	

		free( hullmat_stor );
		free( texture_conv_stor );
		cvReleaseImage( &dirty );
		cvReleaseImage( &clean_remapped );
		
		IplImage * clean_thresh_conhull = cvCreateImage(cvGetSize(clean_remapped_conhull), IPL_DEPTH_8U, 1);
		cvCvtColor( clean_remapped_conhull, clean_thresh_conhull, CV_BGR2GRAY );
		
		cvThreshold( clean_thresh_conhull, clean_thresh_conhull, CLEAN_THRESH, 255, CV_THRESH_BINARY );
		cvSaveImage( "conhull-clean-thresh.jpg", clean_thresh_conhull );	

		cvReleaseImage( &clean_remapped_conhull );
		
		IplImage * dirty_thresh_conhull = cvCreateImage(cvSize(unmodified_bound.width,unmodified_bound.height), IPL_DEPTH_8U, 1);
		cvCvtColor( dirty_conhull, dirty_thresh_conhull, CV_BGR2GRAY );
		cvThreshold( dirty_thresh_conhull, dirty_thresh_conhull, DIRTY_THRESH, 255, CV_THRESH_BINARY );
		
		cvSaveImage( "conhull-dirty-thresh.jpg", dirty_thresh_conhull );	
		
		cvReleaseImage( &clean_thresh_conhull );
		cvReleaseImage( &dirty_thresh_conhull );

		cvReleaseMat( &blank_intrinsics );

		fprintf( output, "Vertices %d\n", verts );
		for( int i=0; i<verts; ++i )
		{
			fprintf( output, "%.6f %.6f %.6f %.6f %.6f\n",
					points->data.fl[3*i+0],
					points->data.fl[3*i+2],
					points->data.fl[3*i+1],
					//(texture->data.fl[2*i+0]+73.0)/camSize.width,
					//(texture->data.fl[2*i+1]+1.0)/camSize.height );
					(clean_texture->data.fl[2*i+0])/camSize.width,
					(clean_texture->data.fl[2*i+1])/camSize.height );
		}
		cvReleaseMat( &points );
		cvReleaseMat( &texture );
		cvReleaseMat( &clean_texture );
		fgets( buf, sizeof(buf), input );
		fgets( buf, sizeof(buf), input );
		fgets( buf, sizeof(buf), input );
		sscanf( buf, "# Number of triangles: %d\n", &tris );

		fprintf( output, "Triangles %d\n", tris );
		int tri[3];
		for( int i=0; !feof(input) && i<tris; )
		{
			fgets( buf, sizeof(buf), input );
			if( *buf == 'f' )
			{
				sscanf( buf, "f %d %d %d", &tri[0], &tri[1], &tri[2] );
				fprintf( output, "%d %d %d\n", tri[0], tri[1], tri[2] );
				++i;
			}
		}

		fclose( input );
		fclose( output );

		return true;
	}

	return false;
}

