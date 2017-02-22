#include <opencv2/opencv.hpp>

#include <librealsense/rs.hpp>

#include <cstdio>

#include <iostream>

#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;


 /** Global variables */
 String face_cascade_name = "haarcascade_frontalface_alt.xml";
 CascadeClassifier face_cascade;
 string window_name = "Capture - Face detection";
 RNG rng(12345);
 //Mat dpt;

int main(int, char**) try{

	//-- 1. Load the cascades
   	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	//Variables
	const int matSize[2]={480,640};
	//const int matSize2[2]={1080,1920};//rgb res
	const Point center(matSize[1]/2, matSize[0]/2); //low res
	//const Point center(matSize2[1]/2, matSize2[0]/2); // high res
	vector<Rect> faces;
	Mat frame_gray;

	int faceX = -1;
	int faceY = -1;
	double angleX = 0.0;
	double angleY = 0.0;
	double dist = 0.0; 
	int loopCount = 0;
	int loopCount2 = 0;

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
		
		//Color
		//Retrieve the color frame from camera and place in new Mat
		Mat color(2, matSize, CV_8UC3, (uchar *) dev->get_frame_data(rs::stream::color)); //low res
		
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
			if ((faceX == -1) && (faceY == -1)) {
				faceX = faces[i].x + faces[i].width*0.5;
				faceY = faces[i].y + faces[i].height*0.5;
			}
			else {
				int nfaceX = faces[i].x + faces[i].width*0.5;
				int nfaceY = faces[i].y + faces[i].height*0.5;
				int dfaceX = abs(faceX - nfaceX);
				int dfaceY = abs(faceY - nfaceY);
				if ((dfaceX < 100) && (dfaceY < 100)) { //adjust the ability to follow a face in motion
					faceX = nfaceX;
					faceY = nfaceY;
					loopCount = 0;
					loopCount2++;
				}

			}
			
		}
		if ((faceX != -1) && (faceY != -1) && (loopCount2 > 2)){//adjust how long to recognize a new face
			Point faceCenter( faceX, faceY );
			arrowedLine(color, center, faceCenter, Scalar( 85, 155, 215), 5);
			angleX = (faceX - 320) * 0.109375;//convert position on X axis to angle of deflection from center point
			angleY = (faceY - 240) * 0.089583 * -1;//convert position on Y axis to angle of deflection from center point							
			dist = dpt.at<unsigned int16_t>(faceY, faceX);
			
			printf("angleX: %f angleY: %f dist: %f\n", angleX, angleY, dist);
		}
		
		if (loopCount >= 10) { //adjust how long the arrow stays without detecting a face
			faceX = -1;
			faceY = -1;
			printf("LoopCount: %d RESET\n", loopCount);
			loopCount = 0;
			loopCount2 = 0;
		}	
		
		//Show image with arrow
		imshow("Color", color);
		
		loopCount++;
		
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




