#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sift.h"
#include "imgfeatures.h"
#include "kdtree.h"
#include "utils.h"
#include "xform.h"

#include <vector>
#include <algorithm>

#include "opticaltri.h"
#include "siftgrid.h"

/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200

/* threshold on squared ratio of distances between NN and 2nd NN */
#define NN_SQ_DIST_RATIO_THR 0.35

#define BOX_WIDTH 1000
#define BOX_HEIGHT 1000

int findsiftpoints( char * img1fname, char * img2fname, CvPoint2D32f * &source_points, CvPoint2D32f * &dest_points, char * &status )
{
	IplImage * img1, * img2, * img1orig, * img2orig;
	int count, n1, n2, k, i, m;
	struct feature * feat1, * feat2, * feat;
	struct feature** nbrs;
	struct kd_node* kd_root;
	double d0, d1;
	
	std::vector<std::pair<CvPoint, CvPoint> > point_correspondences;
	
	CvRect current_box = cvRect( 0, 0, BOX_WIDTH, BOX_HEIGHT );

	img1orig = cvLoadImage( img1fname, CV_LOAD_IMAGE_COLOR );
	int image_width = img1orig->width;
	int image_height = img1orig->height;
	cvReleaseImage( &img1orig );

	int xstep = BOX_WIDTH - 50;
	int ystep = BOX_HEIGHT - 50;

	for( int y = 0; y < image_height; y += ystep ) {
		current_box.y = y;
		current_box.height = (current_box.y + BOX_HEIGHT) < image_height ? BOX_HEIGHT : image_height - current_box.y;
		for( int x = 0; x < image_width; x += xstep ) {
			current_box.x = x;
			current_box.width = (current_box.x + BOX_WIDTH) < image_width ? BOX_WIDTH : image_width - current_box.x;
			
			img1orig = cvLoadImage( img1fname, CV_LOAD_IMAGE_COLOR );
			cvSetImageROI( img1orig, current_box );
			img1 = cvCreateImage( cvSize( current_box.width, current_box.height ), img1orig->depth, img1orig->nChannels );
			cvCopy( img1orig, img1 );
			cvReleaseImage( &img1orig );

			img2orig = cvLoadImage( img2fname, CV_LOAD_IMAGE_COLOR );
			cvSetImageROI( img2orig, current_box );
			img2 = cvCreateImage( cvSize( current_box.width, current_box.height ), img2orig->depth, img2orig->nChannels );
			cvCopy( img2orig, img2 );
			cvReleaseImage( &img2orig );

			fprintf( stdout, "Finding features in %s (%d,%d,%d,%d)...\n", img1fname, current_box.x, current_box.y, current_box.width, current_box.height );
			n1 = sift_features( img1, &feat1 );
			fprintf( stdout, "Finding features in %s (%d,%d,%d,%d)...\n", img2fname, current_box.x, current_box.y, current_box.width, current_box.height );
			n2 = sift_features( img2, &feat2 );

			cvReleaseImage( &img1 );
			cvReleaseImage( &img2 );

			if(n2) {
				m = 0;
				kd_root = kdtree_build( feat2, n2 );
				if(kd_root) {
					for( i = 0; i < n1; i++ ) {
						feat = feat1 + i;
						k = kdtree_bbf_knn( kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
						if( k == 2 ) {
							d0 = descr_dist_sq( feat, nbrs[0] );
							d1 = descr_dist_sq( feat, nbrs[1] );
							if( d0 < d1 * NN_SQ_DIST_RATIO_THR ) {
								m++;
								feat1[i].fwd_match = nbrs[0];
								std::pair<CvPoint, CvPoint> curmatch = std::pair<CvPoint, CvPoint>(cvPoint((int)(feat1[i].img_pt.x+current_box.x),(int)(feat1[i].img_pt.y+current_box.y)),cvPoint((int)(nbrs[0]->img_pt.x+current_box.x),(int)(nbrs[0]->img_pt.y+current_box.y)));
								if(std::find(point_correspondences.begin(), point_correspondences.end(), curmatch) == point_correspondences.end()) {
									point_correspondences.push_back(curmatch);
								}
							}
						}
						free( nbrs );
					}
					kdtree_release( kd_root );
				}
			}

			free( feat1 );
			free( feat2 );

			printf("%d features so far\n",point_correspondences.size());
		}
	}

	count = point_correspondences.size();

	source_points = (CvPoint2D32f*)cvAlloc(count*sizeof(CvPoint2D32f));
	dest_points = (CvPoint2D32f*)cvAlloc(count*sizeof(CvPoint2D32f));
	
	status = (char*)cvAlloc(sizeof(char)*count);

	for( i = 0; i < count; i++ ) {
		status[i] = 1;
		source_points[i] = cvPointTo32f(point_correspondences[i].first);
		dest_points[i] = cvPointTo32f(point_correspondences[i].second);
	}

	return count;
}
