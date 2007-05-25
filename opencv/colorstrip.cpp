#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <string>
#include <vector>

#define DEBUG

#define NUM_COLORS 5 // skip yellow, white, dark

#define AREA_MIN 60000
#define AREA_MAX 100000
#define CONTOUR_MATCH_MAX 0.1
// #define AREA_MIN 1000
// #define AREA_MAX 10000

int hdims = 16;
float hranges_arr[] = {0, 180};
float *hranges = hranges_arr;

double avg_area = 0;

CvRect reference_bounds[NUM_COLORS];
CvHistogram *computed_hists[NUM_COLORS];
CvSeq *reference_contour[NUM_COLORS];
CvSeq *matching_contour[NUM_COLORS];

bool read_rois( const char * file );
bool compute_hists( const char * file );
bool back_project( const char * file, bool reference );

// TODO: save out reference histograms and reference contours
// instead of computing them each time

int main( int argc, char * argv[] )
{
	if( argc < 4 ) {
		fprintf( stderr, "%s reference_image color_locations input_image", argv[0] );
		return -1;
	}

	char *reference_filename = argv[1];
	char *roi_filename = argv[2];
	char *input_filename = argv[3];

	printf("Reading color locations\n");
	if( !read_rois( roi_filename ) ) {
		fprintf(stderr, "Unable to read color locations.\n");
		return -1;
	}

	printf("Computing color histograms\n");
	if( !compute_hists( reference_filename ) ) {
		fprintf(stderr, "Unable to compute histograms.\n");
		return -1;
	}

	printf("Back projecting histograms in reference and finding largest connected component\n");
	if( !back_project( reference_filename, true ) ) {
		fprintf(stderr, "Unable to back project histograms.\n");
		return -1;
	}
	
	printf("Back projecting histograms in input and finding best connected component\n");
	if( !back_project( input_filename, false ) ) {
		fprintf(stderr, "Unable to back project histograms.\n");
		return -1;
	}
	
	return 0;
}

bool read_rois( const char * file )
{
	// Read in the ROIs for color squares in the reference image
	FILE *color_loc_file = fopen( file, "r" );
	if(color_loc_file) {
		for( int i = 0; i < NUM_COLORS; i++ ) {
			fscanf( color_loc_file, "%d %d %d %d\n",
							&(reference_bounds[i].x),
							&(reference_bounds[i].y),
							&(reference_bounds[i].width),
							&(reference_bounds[i].height)
					);
		}
		fclose(color_loc_file);
		return true;
	}
	else {
		return false;
	}
}

bool compute_hists( const char * file )
{
	// For each color, read in the sample in the reference image
	// and compute the histogram in HSV space.
	IplImage *reference_image = cvLoadImage( file, CV_LOAD_IMAGE_COLOR );
	IplImage *hsv = cvCreateImage( cvGetSize(reference_image), 8, 3 );
	IplImage *hue = cvCreateImage( cvGetSize(reference_image), 8, 1 );
	cvCvtColor( reference_image, hsv, CV_BGR2HSV );
	cvSplit( hsv, hue, 0, 0, 0 );
	for( int i = 0; i < NUM_COLORS; i++ ) {
		computed_hists[i] = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
		
		cvSetImageROI( hue, reference_bounds[i] );
		cvCalcHist( &hue, computed_hists[i], 0 );
		float max_val = 0;
		cvGetMinMaxHistValue( computed_hists[i], 0, &max_val, 0, 0 );
		cvConvertScale( computed_hists[i]->bins, 
										computed_hists[i]->bins, 
										max_val ? 255. / max_val : 0., 0 );
		cvResetImageROI( hue );
	}
	
	cvReleaseImage( &reference_image );
	cvReleaseImage( &hsv );
	cvReleaseImage( &hue );
	
	return true;
}

bool back_project( const char * file, bool reference )
{
	IplImage *input_image = cvLoadImage( file, CV_LOAD_IMAGE_COLOR );

	IplImage *dst = cvCreateImage( cvGetSize(input_image), 8, 3 );
	IplImage *input_hue = cvCreateImage( cvGetSize(input_image), 8, 1 );
	IplImage *backproject = cvCreateImage( cvGetSize(input_image), 8, 1 );

	cvCvtColor( input_image, input_image, CV_BGR2HSV );
	cvSplit( input_image, input_hue, 0, 0, 0 );

	// Use the back projection of each histogram to find the
	// corresponding color square in the input image.
	for( int i = 0; i < NUM_COLORS; i++ ) {
		cvCalcBackProject( &input_hue, backproject, computed_hists[i] );
		cvThreshold( backproject, backproject, 254, 255, CV_THRESH_BINARY );

		// cvSaveImage( "backproject.jpg", backproject );

		CvMemStorage *storage = cvCreateMemStorage( 0 );
		CvSeq *best_contour = 0;
		CvSeq *contour = 0;

		cvFindContours( backproject, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		double max_area = 0;
		double best_match = 0;
		double max_metric = 0;
		for( ; contour != 0; contour = contour->h_next )
		{
			double cur_area = fabs( cvContourArea( contour ) );
			if( !reference ) {
				double match1 = cvMatchShapes( reference_contour[i], contour, CV_CONTOURS_MATCH_I1 );
				// double match2 = cvMatchShapes( reference_contour[i], contour, CV_CONTOURS_MATCH_I2 );
				// double match3 = cvMatchShapes( reference_contour[i], contour, CV_CONTOURS_MATCH_I3 );
				double chosen_match = match1;
				// Use a combination of Hu contour similarity and area to determine
				// the best matching contour
				double cur_metric = (1.0 / chosen_match);
			 	cur_metric *=	avg_area > cur_area ? cur_area / avg_area : avg_area / cur_area;
				if((chosen_match > 0) && (cur_area > 0) && (cur_metric > max_metric)) {
					best_match = chosen_match;
					best_contour = contour;
					max_metric = cur_metric;
					// printf("Metric (%.8f,%.8f) %.8f\n",chosen_match,cur_area,cur_metric);
				}
			}
			else if( cur_area > max_area ) {
				best_contour = contour;
				max_area = cur_area;
			}
		}
		if( reference ) {
			printf( "Color %d largest area: %.2f", i+1, max_area );
			if( (max_area > AREA_MIN) && (max_area < AREA_MAX) ) {
				avg_area += max_area;
				reference_contour[i] = best_contour;
				// matching_contour[i] = best_contour;
				printf("\n");
			}
			else {
				reference_contour[i] = 0;
				// matching_contour[i] = 0;
				printf( "\t(rejecting)\n" );
				// missing a correct rect in the reference image
				// TODO: take out hardcoded max/min areas
				return false;
			}
		}
		else {
			printf( "Color %d best match: %.6f\t(Area: %.2f)\n", i+1, best_match, fabs( cvContourArea( best_contour ) ) );
			matching_contour[i] = best_contour;
		}

		if(reference) {
			avg_area = avg_area / (double) NUM_COLORS;
		}
	}
	
	if( !reference ) {
		// write out an image showing the selected color squares
		char *nodir = strrchr(file,'/') + 1;
		std::string output_filename = std::string(nodir).substr(0,strlen(nodir)-4)+"-strip.jpg";

		cvZero( dst );

		for( int i = 0; i < NUM_COLORS; i++ ) {
			if(matching_contour[i]) {
				double scale = ((double)(i+1)/(double)NUM_COLORS)*(double)255;
				CvScalar color = CV_RGB( scale, scale, scale );
				cvDrawContours( dst, matching_contour[i], color, color, -1, CV_FILLED, 8 );
				// CvRect bound = cvBoundingRect( matching_contour[i] );
				// cvRectangle( dst, cvPoint(bound.x, bound.y), cvPoint(bound.x + bound.width, bound.y + bound.height), color, CV_FILLED );
			}
		}

		printf("filename: %s\n", output_filename.c_str());
		cvSaveImage( output_filename.c_str(), dst );
	}

	return true;
}
