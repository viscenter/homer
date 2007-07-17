/*
  Detects SIFT features in two images and finds matches between them.

  Copyright (C) 2006  Rob Hess <hess@eecs.oregonstate.edu>

  @version 1.1.1-20070119
*/

#include "sift.h"
#include "imgfeatures.h"
#include "kdtree.h"
#include "utils.h"
#include "xform.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include <stdio.h>

#include "match.h"

/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200

/* threshold on squared ratio of distances between NN and 2nd NN */
#define NN_SQ_DIST_RATIO_THR 0.49


int match( const char * img1fname, const char * img2fname, CvMat **H )
{
	double scale = 10.0;

	IplImage* img1, * img2, * img1orig, * img2orig;

	struct feature* feat1, * feat2, * feat;
	struct feature** nbrs;
	struct kd_node* kd_root;
	CvPoint pt1, pt2;
	double d0, d1;
	int n1, n2, k, i, m = 0;

	img1orig = cvLoadImage( img1fname, CV_LOAD_IMAGE_COLOR );
	if( ! img1orig )
		fatal_error( "unable to load image from %s", img1fname );
  img1 = cvCreateImage( cvSize( (int)(img1orig->width/scale), (int)(img1orig->height/scale) ), img1orig->depth, img1orig->nChannels );
	cvResize( img1orig, img1, CV_INTER_AREA );
	cvReleaseImage( &img1orig );
  
	img2orig = cvLoadImage( img2fname, CV_LOAD_IMAGE_COLOR );
	if( ! img2orig )
		fatal_error( "unable to load image from %s", img2fname );
  img2 = cvCreateImage( cvSize( (int)(img2orig->width/scale), (int)(img2orig->height/scale) ), img2orig->depth, img2orig->nChannels );
	cvResize( img2orig, img2, CV_INTER_AREA );
	cvReleaseImage( &img2orig );

	fprintf( stdout, "Finding features in %s...\n", img1fname );
  n1 = sift_features( img1, &feat1 );
  fprintf( stdout, "Finding features in %s...\n", img2fname );
  n2 = sift_features( img2, &feat2 );

	cvReleaseImage( &img1 );
	cvReleaseImage( &img2 );

	kd_root = kdtree_build( feat2, n2 );
  for( i = 0; i < n1; i++ ) {
		feat = feat1 + i;
		k = kdtree_bbf_knn( kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
		if( k == 2 ) {
			d0 = descr_dist_sq( feat, nbrs[0] );
			d1 = descr_dist_sq( feat, nbrs[1] );
			if( d0 < d1 * NN_SQ_DIST_RATIO_THR ) {
				/*
				pt1 = cvPoint( cvRound( feat->x ), cvRound( feat->y ) );
				pt2 = cvPoint( cvRound( nbrs[0]->x ), cvRound( nbrs[0]->y ) );
				pt2.y += img1->height;
				*/
				m++;
				feat1[i].fwd_match = nbrs[0];
			}
		}
		free( nbrs );
	}

	
	// scale feature coordinates back in to original image size
	for( i = 0; i < n2; i++ ) {
		feat2[i].img_pt.x *= scale;
		feat2[i].img_pt.y *= scale;
	}
	for( i = 0; i < n1; i++ ) {
		feat1[i].img_pt.x *= scale;
		feat1[i].img_pt.y *= scale;
	}
  
	fprintf( stdout, "Found %d total matches\n", m );
  
	{
    // CvMat* H;
    *H = ransac_xform( feat1, n1, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01,
		      homog_xfer_err, 3.0, NULL, NULL );
		printf("Perspective transform:\n");
		for( i = 0; i < 3; i++ ) {
			printf("%0.6f %0.6f %0.6f\n", (*H)->data.db[3*i+0], (*H)->data.db[3*i+1], (*H)->data.db[3*i+2]);
		}
		/*
		if( H ) {
			IplImage* xformed;
			img2orig = cvLoadImage( img2fname, CV_LOAD_IMAGE_COLOR );
			xformed = cvCreateImage( cvGetSize( img2orig ), IPL_DEPTH_8U, 3 );
			cvReleaseImage( &img2orig );
			img1orig = cvLoadImage( img1fname, CV_LOAD_IMAGE_COLOR);
			cvWarpPerspective( img1orig, xformed, H, 
						CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS,
						cvScalarAll( 0 ) );
			cvSaveImage( "xformed.jpg", xformed );
			cvReleaseImage( &img1orig );
			cvReleaseImage( &xformed );
			// cvReleaseMat( &H );
		}
		*/
	}

	kdtree_release( kd_root );
	free( feat1 );
	free( feat2 );
	return 0;
}
