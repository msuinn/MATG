#include <opencv2/opencv.hpp>

#include <librealsense/rs.hpp>

#include <cstdio>

#include <iostream>

#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;

 /** Function Headers */
 void detectAndDisplay( Mat color );
 void detectAndDisplay2( Mat ir );

 /** Global variables */
 String face_cascade_name = "haarcascade_frontalface_alt.xml";
 String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
 CascadeClassifier face_cascade;
 CascadeClassifier eyes_cascade;
 string window_name = "Capture - Face detection";
 RNG rng(12345);

int main(int, char**) try{

	//-- 1. Load the cascades
   	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
   	if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };	

	//OpenCV Mat initialization
	const int matSize[2]={480,640};

	//Create the windows
	namedWindow("Depth", 1);
	namedWindow("Color", 1);
	namedWindow("IR", 1);	

	// Create a context object. This object owns the handles to all connected realsense devices.
    	rs::context ctx;
    	printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    	if(ctx.get_device_count() == 0){
		return EXIT_FAILURE;
	}

    	// Enable a single device
    	rs::device * dev = ctx.get_device(0);
    	printf("\nUsing device 0, an %s\n", dev->get_name());
    	printf("    Serial number: %s\n", dev->get_serial());
    	printf("    Firmware version: %s\n", dev->get_firmware_version());

    	// Configure depth to run at VGA resolution at 30 frames per second
    	dev->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, 60);
	dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60);
	dev->enable_stream(rs::stream::infrared, 640, 480, rs::format::y8, 60);

    	dev->start();

	while(true){
        	// This call waits until a new coherent set of frames is available on a device
        	// Calls to get_frame_data(...) and get_frame_timestamp(...) on a device will return stable values until wait_for_frames(...) is called
        	dev->wait_for_frames();
		
		//Depth
		//Retrieve the depth frame from camera and place in new Mat
		Mat depth(2, matSize, CV_16U, (uchar *) dev->get_frame_data(rs::stream::depth));
		//Compress down to uchar with one channel only for depth
		depth.convertTo(depth, CV_8UC1, 255.0/1000);
		//Apply color map depending on the depth value
		applyColorMap(depth, depth, COLORMAP_RAINBOW);
		
		//Color
		//Retrieve the color frame from camera and place in new Mat
		Mat color(2, matSize, CV_8UC3, (uchar *) dev->get_frame_data(rs::stream::color));
		//Convert color encoding so picutre makes sense
		cvtColor(color, color, COLOR_BGR2RGB);

		//Infrared
		//Retrieve the infrared frame from camera and place in new Mat
		Mat ir(2, matSize, CV_8U, (uchar *) dev->get_frame_data(rs::stream::infrared));

		//-- 3. Apply the classifier to the frame
       		if( !color.empty() )
       		{ detectAndDisplay( color ); }
       		else
       		{ printf(" --(!) No captured frame -- Break!"); break; }

		if( !ir.empty() )
       		{ detectAndDisplay2( ir ); }
       		else
       		{ printf(" --(!) No captured frame -- Break!"); break; }

		//Show both of the views
		imshow("Depth", depth);
		imshow("Color", color);
		imshow("IR", ir);
		
		if(waitKey(30) == 'c'){
			break;
		}	
	}	
	return(0);
}

catch(const rs::error & e)
{
   	// Method calls against librealsense objects may throw exceptions of type rs::error
    	printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    	printf("    %s\n", e.what());
    	return EXIT_FAILURE;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat color )
{
  	std::vector<Rect> faces;
  	Mat frame_gray;

  	cvtColor( color, frame_gray, CV_BGR2GRAY );
  	equalizeHist( frame_gray, frame_gray );

  	//-- Detect faces
  	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

  	for( size_t i = 0; i < faces.size(); i++ )
  	{
    		Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    		ellipse( color, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

    		Mat faceROI = frame_gray( faces[i] );
    		std::vector<Rect> eyes;

    		//-- In each face, detect eyes
    		eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30));

    		for( size_t j = 0; j < eyes.size(); j++ )
     		{
       			Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes	[j].height*0.5 );
       			int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
       			circle( color, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
     		}
  	}
  	//-- Show what you got
  	//imshow( window_name, color );
}
void detectAndDisplay2( Mat ir )
{
  	std::vector<Rect> faces;
  	//Mat frame_gray;

  	//cvtColor( color, frame_gray, CV_BGR2GRAY );
  	//equalizeHist( frame_gray, frame_gray );

  	//-- Detect faces
  	face_cascade.detectMultiScale( ir, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

  	for( size_t i = 0; i < faces.size(); i++ )
  	{
    		Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    		ellipse( ir, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

    		Mat faceROI = ir( faces[i] );
    		std::vector<Rect> eyes;

    		//-- In each face, detect eyes
    		eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30));

    		for( size_t j = 0; j < eyes.size(); j++ )
     		{
       			Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes	[j].height*0.5 );
       			int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
       			circle( ir, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
     		}
  	}
  	//-- Show what you got
  	//imshow( window_name, color );
}


