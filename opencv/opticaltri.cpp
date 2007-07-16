#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

//#define MAX_COUNT 16384
#define MAX_COUNT 131072

struct Triangle {
	CvPoint points[3];
};

bool operator<(const CvPoint &p1, const CvPoint &p2) {
	if((p1.x < p2.x) && (p1.y < p2.y)) {
		return true;
	} // not both less than
	else if(p1.x < p2.x) {
		return true;
	} // not both less than and x is not less than
	else if(p1.y < p2.y) {
		return true;
	}
	return false;
}

bool operator==(const CvPoint &p1, const CvPoint &p2) {
	if((p1.x == p2.x) && (p1.y == p2.y)) {
		return true;
	}
	return false;
}

bool operator==(const Triangle &t1, const Triangle &t2) {
	if( ( (t1.points[0] == t2.points[0]) &&
				(t1.points[1] == t2.points[1]) &&
				(t1.points[2] == t2.points[2]) ) || 
			( (t1.points[0] == t2.points[1]) &&
				(t1.points[1] == t2.points[2]) &&
				(t1.points[2] == t2.points[0]) ) || 
			( (t1.points[0] == t2.points[2]) &&
				(t1.points[1] == t2.points[0]) &&
				(t1.points[2] == t2.points[1]) ) ||
			( (t1.points[0] == t2.points[1]) &&
				(t1.points[1] == t2.points[0]) &&
				(t1.points[2] == t2.points[2]) ) || 
			( (t1.points[0] == t2.points[0]) &&
				(t1.points[1] == t2.points[2]) &&
				(t1.points[2] == t2.points[1]) ) || 
			( (t1.points[0] == t2.points[2]) &&
				(t1.points[1] == t2.points[1]) &&
				(t1.points[2] == t2.points[0]) ) )	{
		return true;
	}
	return false;
}

void draw_subdiv_edge( IplImage* img, CvSubdiv2D* subdiv, CvSubdiv2DEdge edge, CvScalar color, CvPoint2D32f * unwarped_points, CvPoint2D32f * warped_points, int count, char * status )
{
    CvSubdiv2DPoint* org_pt;
    CvSubdiv2DPoint* dst_pt;
    CvPoint2D32f org;
    CvPoint2D32f dst;
    CvPoint iorg, idst;

    org_pt = cvSubdiv2DEdgeOrg(edge);
    dst_pt = cvSubdiv2DEdgeDst(edge);

    if( org_pt && dst_pt )
    {
        org = org_pt->pt;
        dst = dst_pt->pt;

				iorg = cvPoint( cvRound( org.x ), cvRound( org.y ));
        idst = cvPoint( cvRound( dst.x ), cvRound( dst.y ));

				if( count ) {
					CvPoint orgwarp, dstwarp;
					
					bool found_org = false, found_dst = false;
					int found_org_loc, found_dst_loc;
					CvSubdiv2DPoint * found_org_pt;
					CvSubdiv2DPoint * found_dst_pt;
					CvSubdiv2DEdge ignored_edge;
					for(int i = 0; i < count; i++) {
						if(status[i]) {
							if(!found_org) {
								if( ((org_pt->pt).x == unwarped_points[i].x) && ((org_pt->pt).y == unwarped_points[i].y) ) {
									found_org = true;
									found_org_loc = i;
								}
							}
							if(!found_dst) {
								if( ((dst_pt->pt).x == unwarped_points[i].x) && ((dst_pt->pt).y == unwarped_points[i].y) ) {
									found_dst = true;
									found_dst_loc = i;
								}
							}
						}
					}

					if((!found_dst) && (!found_org)) {
						printf("Unfound point correspondence!\n");
					}
					else {
						if(found_org) {
							orgwarp = cvPointFrom32f(warped_points[found_org_loc]);
							iorg = orgwarp;
						}
						if(found_dst) {
							dstwarp = cvPointFrom32f(warped_points[found_dst_loc]);
							idst = dstwarp;
						}
					}
				}

				iorg.x = iorg.x > img->width ? img->width : iorg.x;
				iorg.y = iorg.y > img->height ? img->height : iorg.y;
				idst.x = idst.x > img->width ? img->width : idst.x;
				idst.y = idst.y > img->height ? img->height : idst.y;

				iorg.x = iorg.x < 0 ? 0 : iorg.x;
				iorg.y = iorg.y < 0 ? 0 : iorg.y;
				idst.x = idst.x < 0 ? 0 : idst.x;
				idst.y = idst.y < 0 ? 0 : idst.y;
				
				cvLine( img, iorg, idst, color, 1, CV_AA, 0 );
    }
}

void draw_subdiv( IplImage* img, CvSubdiv2D* subdiv, CvPoint2D32f * unwarped_points, CvPoint2D32f * warped_points, int count, char * status )
{
    CvSeqReader  reader;
    int i, total = subdiv->edges->total;
    int elem_size = subdiv->edges->elem_size;

    cvStartReadSeq( (CvSeq*)(subdiv->edges), &reader, 0 );

    for( i = 0; i < total; i++ )
    {
        CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

        if( CV_IS_SET_ELEM( edge ))
        {
            // draw_subdiv_edge( img, (CvSubdiv2DEdge)edge + 1, voronoi_color );
            draw_subdiv_edge( img, subdiv, (CvSubdiv2DEdge)edge, CV_RGB(0,255,0), unwarped_points, warped_points, count, status );
        }

        CV_NEXT_SEQ_ELEM( elem_size, reader );
		}
}

int main(int argc, char * argv[])
{
	if(argc < 2) {
		fprintf(stderr, "%s image1 image2\n", argv[0]);
		return 1;
	}

	char * im1fname = argv[1];
	char * im2fname = argv[2];

	IplImage * image1 = cvLoadImage(im1fname, CV_LOAD_IMAGE_GRAYSCALE);

	IplImage * eigenvalues = cvCreateImage(cvGetSize(image1), 32, 1);
	IplImage * temp = cvCreateImage(cvGetSize(image1), 32, 1);

	int count = MAX_COUNT;
	double quality = 0.5;
	// double min_distance = 2;
	double min_distance = 50;
	int block_size = 7;
	int use_harris = 0;
	int win_size = 10;
	int flags = 0;

	CvPoint2D32f * source_points = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
	CvPoint2D32f * dest_points = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));
	CvPoint2D32f * delaunay_points = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(CvPoint2D32f));

	cvGoodFeaturesToTrack( image1, eigenvalues, temp, source_points, &count,
			quality, min_distance, 0, block_size, use_harris, 0.04 );

	printf("%d features\n",count);

	setbuf(stdout, NULL);

	printf("Finding corner subpix...");
	cvFindCornerSubPix( image1, source_points, count,
			cvSize(win_size,win_size), cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03));
	printf("done.\n");

	cvReleaseImage(&eigenvalues);
	cvReleaseImage(&temp);

	IplImage * image2 = cvLoadImage(im2fname, CV_LOAD_IMAGE_GRAYSCALE);

	char * status = (char*)cvAlloc(sizeof(char)*MAX_COUNT);

	IplImage * pyramid = cvCreateImage( cvGetSize(image1), IPL_DEPTH_8U, 1 );
	IplImage * second_pyramid = cvCreateImage( cvGetSize(image2), IPL_DEPTH_8U, 1 );

	printf("Computing optical flow...");	
	cvCalcOpticalFlowPyrLK(image1, image2, pyramid, second_pyramid, source_points,
		dest_points, count, cvSize(win_size,win_size), 4, status, 0,
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03),
		flags);
	printf("done.\n");

	int num_matches = 0;
	int num_out_matches = 0;
	int max_dist = 30;
	int offset = 200;
	
	CvMemStorage * storage = cvCreateMemStorage(0);
	CvSubdiv2D * delaunay = cvCreateSubdivDelaunay2D( cvRect(0,0,image1->width,image1->height), storage);

	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	
	image1 = cvLoadImage(im1fname, CV_LOAD_IMAGE_COLOR);
	image2 = cvLoadImage(im2fname, CV_LOAD_IMAGE_COLOR);

	cvSet( image1, cvScalarAll(255) );

	std::map<CvPoint, CvPoint> point_lookup_map;
	std::vector<std::pair<CvPoint, CvPoint> > point_lookup;

	// put corners in the point lookup as going to themselves
	point_lookup_map[cvPoint(0,0)] = cvPoint(0,0);
	point_lookup_map[cvPoint(0,image1->height-1)] = cvPoint(0,image1->height-1);
	point_lookup_map[cvPoint(image1->width-1,0)] = cvPoint(image1->width-1,0);
	point_lookup_map[cvPoint(image1->width-1,image1->height-1)] = cvPoint(image1->width-1,image1->height-1);

	point_lookup.push_back(std::pair<CvPoint,CvPoint>(cvPoint(0,0), cvPoint(0,0)));
	point_lookup.push_back(std::pair<CvPoint,CvPoint>(cvPoint(0,image1->height-1), cvPoint(0,image1->height-1)));
	point_lookup.push_back(std::pair<CvPoint,CvPoint>(cvPoint(image1->width-1,0), cvPoint(image1->width-1,0)));
	point_lookup.push_back(std::pair<CvPoint,CvPoint>(cvPoint(image1->width-1,image1->height-1), cvPoint(image1->width-1,image1->height-1)));

	printf("Inserting corners...");
	// put corners in the Delaunay subdivision
	for(unsigned int i = 0; i < point_lookup.size(); i++) {
		cvSubdivDelaunay2DInsert( delaunay, cvPointTo32f(point_lookup[i].first) );
	}
	printf("done.\n");

	CvSubdiv2DEdge proxy_edge;
	for(int i = 0; i < count; i++) {
		if(status[i]) {
			CvPoint source = cvPointFrom32f(source_points[i]);
			CvPoint dest = cvPointFrom32f(dest_points[i]);
	
			if((((int)fabs((double)(source.x - dest.x))) > max_dist) ||
				 (((int)fabs((double)(source.y - dest.y))) > max_dist)) {	
				num_out_matches++;
			}
			else if((dest.x >= 0) && (dest.y >= 0) && (dest.x < (image1->width)) && (dest.y < (image1->height))) {
				if(point_lookup_map.find(source) == point_lookup_map.end()) {
					num_matches++;
				
					point_lookup_map[source] = dest;
					point_lookup.push_back(std::pair<CvPoint,CvPoint>(source,dest));
					delaunay_points[i] = (cvSubdivDelaunay2DInsert( delaunay, cvPointTo32f(source) ))->pt;
					cvSetImageROI( image1, cvRect(source.x-8,source.y-8,8*2,8*2) );
					cvResetImageROI( image2 );
					cvGetRectSubPix( image2, image1, dest_points[i] );
				}
				/*
				cvSet2D( image1, source.y, source.x, cvGet2D( image2, dest.y, dest.x ) );
				cvSet2D( image1, source.y, source.x+1, cvGet2D( image2, dest.y, dest.x+1 ) );
				cvSet2D( image1, source.y, source.x-1, cvGet2D( image2, dest.y, dest.x-1 ) );
				cvSet2D( image1, source.y+1, source.x, cvGet2D( image2, dest.y+1, dest.x ) );
				cvSet2D( image1, source.y-1, source.x, cvGet2D( image2, dest.y-1, dest.x ) );
				cvSet2D( image1, source.y+1, source.x+1, cvGet2D( image2, dest.y+1, dest.x+1 ) );
				cvSet2D( image1, source.y-1, source.x-1, cvGet2D( image2, dest.y-1, dest.x-1 ) );
				cvSet2D( image1, source.y+1, source.x-1, cvGet2D( image2, dest.y+1, dest.x-1 ) );
				cvSet2D( image1, source.y-1, source.x+1, cvGet2D( image2, dest.y-1, dest.x+1 ) );
				*/

				// cvCircle( image1, source, 4, CV_RGB(255,0,0), 2, CV_AA );
				// cvCircle( image2, dest, 4, CV_RGB(255,0,0), 2, CV_AA );
			}

			/*
			cvSetImageROI( image1, cvRect(source.x-offset,source.y-offset,offset*2,offset*2) );
			cvSetImageROI( image2, cvRect(dest.x-offset,dest.y-offset,offset*2,offset*2) );
			cvNamedWindow("image1",0);
			cvNamedWindow("image2",0);
			cvShowImage("image1",image1);
			cvShowImage("image2",image2);
			printf("%d,%d -> %d,%d\n",source.x,source.y,dest.x,dest.y);
			cvWaitKey(0);
			cvDestroyAllWindows();
			*/
		}
	}
	printf("%d %d\n",num_matches,num_out_matches);
	printf("%d lookups\n",point_lookup_map.size());

	cvResetImageROI( image1 );

	cvSaveImage("sparse.jpg", image1);

	cvReleaseImage(&image1);
	image1 = cvLoadImage(im1fname, CV_LOAD_IMAGE_COLOR);
	cvSet( image1, cvScalarAll(255) );
	printf("Warping image...");

	CvSeqReader  reader;
	int total = delaunay->edges->total;
	int elem_size = delaunay->edges->elem_size;

	cvStartReadSeq( (CvSeq*)(delaunay->edges), &reader, 0 );

	std::vector<Triangle> trivec;
	std::vector<CvMat *> baryinvvec;

	for( int i = 0; i < total; i++ ) {
		CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

		if( CV_IS_SET_ELEM( edge ))	{
			CvSubdiv2DEdge curedge = (CvSubdiv2DEdge)edge;
			CvSubdiv2DEdge t = curedge;
			Triangle temptri;
			int count = 0;
			
			// construct a triangle from this edge
			do {
				CvSubdiv2DPoint* pt = cvSubdiv2DEdgeOrg( t );
				if(count < 3) {
					pt->pt.x = pt->pt.x >= image1->width ? image1->width-1 : pt->pt.x;
					pt->pt.y = pt->pt.y >= image1->height ? image1->height-1 : pt->pt.y;
					pt->pt.x = pt->pt.x < 0 ? 0 : pt->pt.x;
					pt->pt.y = pt->pt.y < 0 ? 0 : pt->pt.y;

					temptri.points[count] = cvPointFrom32f( pt->pt );
				}
				else {
					printf("More than 3 edges\n");
				}
				count++;
				t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
			} while( t != curedge );
			
			// check that triangle is not already in
			if( std::find(trivec.begin(), trivec.end(), temptri) == trivec.end() ) {
				// push triangle in and draw
				trivec.push_back(temptri);
				cvLine( image1, temptri.points[0], temptri.points[1], CV_RGB(255,0,0), 1, CV_AA, 0 );
				cvLine( image1, temptri.points[1], temptri.points[2], CV_RGB(255,0,0), 1, CV_AA, 0 );
				cvLine( image1, temptri.points[2], temptri.points[0], CV_RGB(255,0,0), 1, CV_AA, 0 );

				// compute barycentric computation vector for this triangle
				CvMat * barycen = cvCreateMat( 3, 3, CV_32FC1 );
				CvMat * baryceninv = cvCreateMat( 3, 3, CV_32FC1 );

				barycen->data.fl[3*0+0] = temptri.points[0].x;
				barycen->data.fl[3*0+1] = temptri.points[1].x;
				barycen->data.fl[3*0+2] = temptri.points[2].x;
				barycen->data.fl[3*1+0] = temptri.points[0].y;
				barycen->data.fl[3*1+1] = temptri.points[1].y;
				barycen->data.fl[3*1+2] = temptri.points[2].y;
				barycen->data.fl[3*2+0] = 1;
				barycen->data.fl[3*2+1] = 1;
				barycen->data.fl[3*2+2] = 1;

				cvInvert( barycen, baryceninv, CV_LU );
				baryinvvec.push_back(baryceninv);

				cvReleaseMat( &barycen );
			}
		}

		CV_NEXT_SEQ_ELEM( elem_size, reader );
	}
	printf("%d triangles...", trivec.size());
	cvSaveImage("triangles.jpg", image1);
	
	cvSet( image1, cvScalarAll(255) );
	IplImage * clean_nonthresh = cvLoadImage( "conhull-clean.jpg", CV_LOAD_IMAGE_COLOR );

	// for each triangle
	for(unsigned int i = 0; i < trivec.size(); i++) {
		Triangle curtri = trivec[i];
		CvMat * curpoints = cvCreateMat( 1, 3, CV_32SC2 );
		Triangle target;
		std::map<CvPoint,CvPoint>::iterator piter[3];
		
		printf("Triangle %d / %d\n",i,trivec.size());
		bool is_corner = false;
		for(int j = 0; j < 3; j++) {
			/*
			curpoints->data.i[2*j+0] = curtri.points[j].x;
			curpoints->data.i[2*j+1] = curtri.points[j].y;
			*/
			CV_MAT_ELEM( *curpoints, CvPoint, 0, j ) = curtri.points[j];
			printf("%d,%d\n",curtri.points[j].x,curtri.points[j].y);
	
			/*	
			if((curtri.points[j] == cvPoint(0,0)) ||  (curtri.points[j] == cvPoint(0,image1->height)) ||(curtri.points[j] == cvPoint(image1->width,0)) ||(curtri.points[j] == cvPoint(image1->width,image1->height))) {
				is_corner = true;
				break;
			}
			*/

			for(unsigned int k = 0; k < point_lookup.size(); k++) {
				std::pair<CvPoint,CvPoint> thispair = point_lookup[k];
				if(thispair.first == curtri.points[j]) {
					target.points[j] = thispair.second;
					break;
				}
			}

			/*
			piter[j] = point_lookup_map.find(curtri.points[j]);
			if(piter[j] != point_lookup_map.end() ) {
				target.points[j] = piter[j]->second;
			}
			*/
		}
			
		// if((piter[0] != point_lookup_map.end()) && (piter[1] != point_lookup_map.end()) && (piter[2] != point_lookup_map.end())) {
		if(!is_corner) {
			CvMat * newcorners = cvCreateMat( 3, 3, CV_32FC1 );
			newcorners->data.fl[3*0+0] = target.points[0].x;
			newcorners->data.fl[3*0+1] = target.points[1].x;
			newcorners->data.fl[3*0+2] = target.points[2].x;
			newcorners->data.fl[3*1+0] = target.points[0].y;
			newcorners->data.fl[3*1+1] = target.points[1].y;
			newcorners->data.fl[3*1+2] = target.points[2].y;
			newcorners->data.fl[3*2+0] = 1;
			newcorners->data.fl[3*2+1] = 1;
			newcorners->data.fl[3*2+2] = 1;

			CvContour hdr;
			CvSeqBlock blk;
			CvRect trianglebound = cvBoundingRect( cvPointSeqFromMat(CV_SEQ_KIND_CURVE+CV_SEQ_FLAG_CLOSED, curpoints, &hdr, &blk), 1 );
			printf("Bounding box: %d,%d,%d,%d\n",trianglebound.x,trianglebound.y,trianglebound.width,trianglebound.height);
			for(int y = trianglebound.y; (y < (trianglebound.y + trianglebound.height)) && ( y < image1->height); y++) {
				for(int x = trianglebound.x; (x < (trianglebound.x + trianglebound.width)) && (x < image1->width); x++) {
					// check to see if we're inside this triangle
					/*
					CvPoint v0 = cvPoint( curtri.points[2].x - curtri.points[0].x, curtri.points[2].y - curtri.points[0].y );
					CvPoint v1 = cvPoint( curtri.points[1].x - curtri.points[0].x, curtri.points[1].y - curtri.points[0].y );
					CvPoint v2 = cvPoint( x - curtri.points[0].x, y - curtri.points[0].y );
					
					int dot00 = v0.x * v0.x + v0.y * v0. y;
					int dot01 = v0.x * v1.x + v0.y * v1. y;
					int dot02 = v0.x * v2.x + v0.y * v2. y;
					int dot11 = v1.x * v1.x + v1.y * v1. y;
					int dot12 = v1.x * v2.x + v1.y * v2. y;

					double invDenom = 1.0 / (double)(dot00 * dot11 - dot01 * dot01);
					double u = (double)(dot11 * dot02 - dot01 * dot12) * invDenom;
					double v = (double)(dot00 * dot12 - dot01 * dot02) * invDenom;
					*/

					CvMat * curp = cvCreateMat(3, 1, CV_32FC1);
					CvMat * result = cvCreateMat(3, 1, CV_32FC1);
					curp->data.fl[0] = x;
					curp->data.fl[1] = y;
					curp->data.fl[2] = 1;
					cvMatMul( baryinvvec[i], curp, result );
					// double u = result->data.fl[0]/result->data.fl[2];
					// double v = result->data.fl[1]/result->data.fl[2];
			

					if( (result->data.fl[0] > 0) && (result->data.fl[1] > 0) && (fabs(1.0 - (result->data.fl[0]+result->data.fl[1]+result->data.fl[2])) <= 0.01) ) {
					// if((u > 0) || (v > 0) /*&& ((u +v) < 1)*/ ) {
						// printf("Barycentric: %f %f %f\n", result->data.fl[0], result->data.fl[1], result->data.fl[2]);
						// this point is inside this triangle
						// printf("Point %d,%d inside %d,%d %d,%d %d,%d\n",x,y,trivec[i].points[0].x,trivec[i].points[0].y,
						//	trivec[i].points[1].x,trivec[i].points[1].y,trivec[i].points[2].x,trivec[i].points[2].y);

						CvMat * sourcepoint = cvCreateMat(3, 1, CV_32FC1);
						cvMatMul( newcorners, result, sourcepoint );
						double sourcex = sourcepoint->data.fl[0]/*/sourcepoint->data.fl[2]*/;
						double sourcey = sourcepoint->data.fl[1]/*/sourcepoint->data.fl[2]*/;
						if((sourcex >= 0) && (sourcey >= 0) && (sourcex < (image1->width)) && (sourcey < (image1->height))) {
							// printf("%d,%d %d,%d\n",x,y,(int)sourcex,(int)sourcey);
							cvSet2D( image1, y, x, cvGet2D( clean_nonthresh, (int)sourcey, (int)sourcex ) );
						}
	
						/*
						if((i == 143) && (y == 3577) && (x > 2055) && (x < 2087)) {
							printf("%d: %f, %f, %f\t%f, %f, %f\n",x,result->data.fl[0],result->data.fl[1],result->data.fl[2],
									sourcepoint->data.fl[0],sourcepoint->data.fl[1],sourcepoint->data.fl[2]);
						}
						*/
	
						cvReleaseMat( &sourcepoint );
						
						// printf("Point %d,%d inside %d,%d %d,%d %d,%d\n",x,y,trivec[i].points[0].x,trivec[i].points[0].y,
						//		trivec[i].points[1].x,trivec[i].points[1].y,trivec[i].points[2].x,trivec[i].points[2].y);

					}
					cvReleaseMat( &result );
					cvReleaseMat( &curp );
				}
			}
			cvReleaseMat( &newcorners );
		}
		cvReleaseMat( &curpoints );
	}
	/*
	for(int y = 0; y < image1->height; y++) {
		for(int x = 0; x < image1->width; x++) {
			CvMat * curp = cvCreateMat(3, 1, CV_32FC1);
			CvMat * result = cvCreateMat(3, 1, CV_32FC1);
			curp->data.fl[0] = x;
			curp->data.fl[1] = y;
			curp->data.fl[2] = 1;
			for(unsigned int i = 0; i < baryinvvec.size(); i++) {
				cvMatMul( baryinvvec[i], curp, result );
				double u = result->data.fl[0]/result->data.fl[2];
				double v = result->data.fl[1]/result->data.fl[2];
				if((u > 0) && (v > 0) && (u + v < 1)) {
					// printf("Point %d,%d inside %d,%d %d,%d %d,%d\n",x,y,trivec[i].points[0].x,trivec[i].points[0].y,
					//		trivec[i].points[1].x,trivec[i].points[1].y,trivec[i].points[2].x,trivec[i].points[2].y);

					break;
				}
			}
			cvReleaseMat( &result );
			cvReleaseMat( &curp );
		}
	}
	*/

	cvReleaseImage( &clean_nonthresh );

#ifdef OLD_BUSTED
	for(int y = 0; y < image1->height; y++) {
		for(int x = 0; x < image1->width; x++) {
			CvSubdiv2DPointLocation locate_result;
			CvSubdiv2DEdge on_edge;
			CvSubdiv2DPoint * on_vertex;
			CvPoint curpoint = cvPoint( x, y );
			locate_result = cvSubdiv2DLocate( delaunay, cvPointTo32f( curpoint ),
				&on_edge, &on_vertex );
			if( (locate_result != CV_PTLOC_OUTSIDE_RECT) && (locate_result != CV_PTLOC_ERROR) ) {
				if( locate_result == CV_PTLOC_VERTEX ) { // this point is on a vertex
					for(int i = 0; i < count; i++) {
						if(((on_vertex->pt).x == delaunay_points[i].x) && ((on_vertex->pt).y == delaunay_points[i].y)) {
							cvSet2D( image1, y, x, cvGet2D( image2, cvPointFrom32f(dest_points[i]).y, cvPointFrom32f(dest_points[i]).x ) );
							break;
						}
					}
				}
				else if( locate_result == CV_PTLOC_ON_EDGE ) { // this point is on an edge
					CvSubdiv2DPoint* org_pt;
					CvSubdiv2DPoint* dst_pt;
					CvPoint org_pt_warp;
					CvPoint dst_pt_warp;
						
					org_pt = cvSubdiv2DEdgeOrg(on_edge);
					dst_pt = cvSubdiv2DEdgeDst(on_edge);

					for(int i = 0; i < count; i++) {
						if(((org_pt->pt).x == delaunay_points[i].x) && ((org_pt->pt).y == delaunay_points[i].y)) {
							org_pt_warp = cvPointFrom32f(dest_points[i]);
						}
						if(((dst_pt->pt).x == delaunay_points[i].x) && ((dst_pt->pt).y == delaunay_points[i].y)) {
							dst_pt_warp = cvPointFrom32f(dest_points[i]);
						}
					}

					// compute vector length of original edge and current point
					double original_length;
					double cur_length; 
					if( (int)((org_pt->pt).x) == curpoint.x ) { // vertical line
						original_length = fabs((org_pt->pt).y - (dst_pt->pt).y);
						cur_length = fabs((org_pt->pt).y - curpoint.y);
					}
					else if( (int)((org_pt->pt).y) == curpoint.y ) { // horizontal line
						original_length = fabs((org_pt->pt).x - (dst_pt->pt).x);
						cur_length = fabs((org_pt->pt).x - curpoint.x);
					}
					else { // sloped line
				 		original_length = sqrt(pow((org_pt->pt).x - (dst_pt->pt).x, 2.0) + pow((org_pt->pt).y - (dst_pt->pt).y, 2.0));
						cur_length = sqrt(pow((org_pt->pt).x - curpoint.x, 2.0) + pow((org_pt->pt).y - curpoint.y, 2.0));
					}
					// compute ratio of this point on the edge
					double ratio = cur_length / original_length;
					// copy this point from the destination edge
					CvPoint point_in_original;
					int warped_x = (int)(org_pt_warp.x - dst_pt_warp.x);
					int warped_y = (int)(org_pt_warp.y - dst_pt_warp.y);
					if( org_pt_warp.x == curpoint.x ) { // vertical line
						point_in_original.y = (int)(org_pt_warp.y + (ratio * (org_pt_warp.y - dst_pt_warp.y)));
						point_in_original.x = org_pt_warp.x;
					}
					else if(org_pt_warp.y == curpoint.y) { // horizontal line
						point_in_original.x = (int)(org_pt_warp.x + (ratio * (org_pt_warp.x - dst_pt_warp.x)));
						point_in_original.y = org_pt_warp.y;
					}
					else { // sloped line
						double destination_length = sqrt(pow((org_pt_warp).x - (dst_pt_warp).x, 2.0) + pow((org_pt_warp).y - (dst_pt_warp).y, 2.0));
						double scaled_length = ratio * destination_length;
						double dest_angle = atan(fabs( (double)warped_y / (double)warped_x ));
						double xdist = scaled_length * cos(dest_angle);
						double ydist = scaled_length * sin(dest_angle);
						xdist = warped_x > 0 ? xdist : xdist * -1;
						ydist = warped_y > 0 ? ydist : ydist * -1;
						point_in_original.x = (int)( org_pt_warp.x + xdist);
						point_in_original.y = (int)( org_pt_warp.y + ydist);
					}
					
					if((point_in_original.x >= 0) && (point_in_original.y >= 0) && (point_in_original.x < (image1->width)) && (point_in_original.y < (image1->height))) {
						cvSet2D( image1, y, x, cvGet2D( image2, point_in_original.y, point_in_original.x ) );
					}
					else {
						printf("Edge point outside image\n");
					}
					// cvSet2D( image1, y, x, cvGet2D( image2, (int)(org_pt_warp.x + (ratio * (org_pt_warp.x - dst_pt_warp.x))), 
					//			(int)(org_pt_warp.y + (ratio * (org_pt_warp.y - dst_pt_warp.y))) ) );
				}
				else if( locate_result == CV_PTLOC_INSIDE ) { // this point is inside a facet (triangle)
					/*
					printf("Point inside facet: %d, %d\n",curpoint.x,curpoint.y);
					int count = 0;
					CvPoint * origins = (CvPoint*)malloc(sizeof(CvPoint)*3);
					CvSubdiv2DEdge t = on_edge;
					// count number of edges
					do {
						CvSubdiv2DPoint* pt = cvSubdiv2DEdgeOrg( t );
						if(count < 3) {
							origins[count] = cvPoint( cvRound(pt->pt.x), cvRound(pt->pt.y));
							printf("%d,%d\t",origins[count].x,origins[count].y);
						}
						count++;
						t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
					} while(t != on_edge);
					printf("\n");

					free(origins);
					*/
				}
			}
		}
	}
#endif // OLD_BUSTED
	printf("done.\n");

	cvSaveImage("fullwarp.jpg", image1);

	printf("Drawing subdivisions on warped image...");
	draw_subdiv( image1, delaunay, NULL, NULL, 0, NULL );
	// draw_subdiv( image1, delaunay, delaunay_points, source_points, count, status );
	printf("done.\n");
	
	cvSaveImage("edgeswarp.jpg", image1);

	cvReleaseImage(&image2);

	image2 = cvLoadImage(im2fname, CV_LOAD_IMAGE_COLOR);
	// cvCreateImage( cvGetSize(image2), IPL_DEPTH_8U, 3 );

	// cvCalcSubdivVoronoi2D( delaunay );
	printf("Drawing subdivisions on unwarped image...");
	draw_subdiv( image2, delaunay, delaunay_points, dest_points, count, status );
	// draw_subdiv( image2, delaunay, NULL, NULL, 0, NULL );
	printf("done.\n");

	cvSaveImage("edges.jpg",image2);

	cvReleaseImage(&image1);
	cvFree(&source_points);
	cvFree(&dest_points);
	cvFree(&status);
	cvReleaseMemStorage(&storage);
	cvFree(&delaunay_points);

	cvReleaseImage(&image2);

	return 0;
}
