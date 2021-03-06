#include <opencv2/opencv.hpp>

#include <librealsense/rs.hpp>

#include <cstdio>

#include <iostream>

#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;


 /** Global variables */
 String face_cascade_name = "haarcascade_frontalface_alt.xml";
 String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
 CascadeClassifier face_cascade;
 CascadeClassifier eyes_cascade;
 string window_name = "Capture - Face detection";
 RNG rng(12345);
 //Mat dpt;

int main(int, char**) try{

	//-- 1. Load the cascades
   	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
   	if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };	

	//Variables
	const int matSize[2]={480,640};
	//const int matSize2[2]={1080,1920};//rgb res
	const Point center(matSize[1]/2, matSize[0]/2); //low res
	//const Point center(matSize2[1]/2, matSize2[0]/2); // high res
	vector<Rect> faces;
	Mat frame_gray;

	int faceX = 0;
	int faceY = 0;
	double angleX = 0.0;
	double angleY = 0.0;
	double dist = 0.0; 

	//Create the windows
	namedWindow("Color", 1);

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
	dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60); // low res
	//dev->enable_stream(rs::stream::color, 1920, 1080, rs::format::rgb8, 30); // high res
	//dev->enable_stream(rs::stream::infrared, 640, 480, rs::format::y8, 60);

    	dev->start();

	while(true){
        	// This call waits until a new coherent set of frames is available on a device
        	// Calls to get_frame_data(...) and get_frame_timestamp(...) on a device will return stable values until wait_for_frames(...) is called
        	dev->wait_for_frames();
		
		//Depth
		//Retrieve the depth frame from camera and place in new Mat
		Mat depth(2, matSize, CV_16U, (uchar *) dev->get_frame_data(rs::stream::depth));
		Mat dpt(depth);		
		//Compress down to uchar with one channel only for depth
		depth.convertTo(depth, CV_8UC1, 255.0/1000);
		//Apply color map depending on the depth value
		//applyColorMap(depth, depth, COLORMAP_RAINBOW);
		
		//Color
		//Retrieve the color frame from camera and place in new Mat
		Mat color(2, matSize, CV_8UC3, (uchar *) dev->get_frame_data(rs::stream::color)); //low res
		//Mat color(2, matSize2, CV_8UC3, (uchar *) dev->get_frame_data(rs::stream::color)); //high res
		//Convert color encoding so picutre makes sense
		cvtColor(color, color, COLOR_BGR2RGB);



		//Convert to gray for face detection function
		cvtColor( color, frame_gray, CV_BGR2GRAY );
  		equalizeHist( frame_gray, frame_gray );


		//Detect faces and place coordinates in faces vector
		face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 
			0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );


		//For each face, find the center and draw an arrow from center to face
		//Also print out the matching pixel depth from depth stream
		for( size_t i = 0; i < faces.size(); i++ ){
    			Point faceCenter( faces[i].x + faces[i].width*0.5, 
				faces[i].y + faces[i].height*0.5 );
			arrowedLine(color, center, faceCenter, Scalar( 94, 206, 165), 5);
			
			faceX = (faces[i].x + faces[i].width*0.5) - 320;
			faceY = (faces[i].y + faces[i].height*0.5) - 240;			

			//printf("faceX: %d faceY: %d\n", faceX, faceY);

			angleX = faceX * 0.109375;//convert position on X axis to angle of deflection from center point
			angleY = faceY * 0.089583 * -1;//convert position on Y axis to angle of deflection from center point							
			dist = dpt.at<unsigned int16_t>(faceCenter.y+ faces[i].height*0.5, faceCenter.x + faces[i].width*0.5);
			//~820 @ 32"
			
			printf("angleX: %f angleY: %f dist: %f\n", angleX, angleY, dist);

		}

		
		
		//Show image with arrow
		imshow("Color", color);
		
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




