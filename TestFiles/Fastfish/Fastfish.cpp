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
	int faceW = -1;
	int faceH = -1;

	int subX = -1;
	int subY = -1;

	int subW = -1;
	int subH = -1;
	
	int subX2 = -1;
	int subY2 = -1;

	int nfaceX = 0;
	int nfaceY = 0;
	int dfaceX = 0;
	int dfaceY = 0;

	int faceCX = -1;
	int faceCY = -1;
	double angleX = 0.0;
	double angleY = 0.0;
	double dist = 0.0;
	int loopCount = 0;
	int loopCount2 = 0;
	int loopCount3 = 0;

	Rect roi;

	Mat sub;

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

		sub = frame_gray;


		//Detect faces and place coordinates in faces vector
		//face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );


		//For each face, find the center and draw an arrow from center to face
		//Also print out the matching pixel depth from depth stream
		

		//initial detection search whole mat
		if ((subX == -1) && (subY == -1)) {
			//printf("looking in 1\n");
			face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
			for (size_t i = 0; i < faces.size(); i++)
			{

				if ((faceX == -1) && (faceY == -1)) {
					faceX = faces[i].x;
					faceY = faces[i].y;
					faceW = faces[i].width;
					faceH = faces[i].height;

				}
				else {
					nfaceX = faces[i].x;
					nfaceY = faces[i].y;
					dfaceX = abs(faceX - nfaceX);
					dfaceY = abs(faceY - nfaceY);

					if ((dfaceX < 100) && (dfaceY < 100)) {
						faceX = nfaceX;
						faceY = nfaceY;

						faceW = faces[i].width;
						faceH = faces[i].height;

						//printf("Detected in first!!!!\n");

						loopCount = 0;

						loopCount3 = 0;

						loopCount2++;						

					}					
					else {
						loopCount3++;
						if (loopCount3 >= 5) loopCount2 = 0;
					}

				}

			}
			if (loopCount2 >= 2) {
				subX = faceX - 50;
				subY = faceY - 50;

				subW = faceW + 100;
				subH = faceH + 100;

				subX2 = subX + subW;
				subY2 = subY + subH;

				if (subX < 0) subX = 0;
				if (subY < 0) subY = 0;
				if (subX2 >= 640) {
					subX2 = 639;
					subW = subX2 - subX;
				}
				if (subY2 >= 480) {
					subY2 = 479;
					subH = subY2 - subY;
				}

				Rect roi1(subX, subY, subW, subH);
				roi = roi1;
				rectangle(color, roi, Scalar(1, 1, 255), 3);
			}
		}
		
		
		else {
			//printf("looking in 2\n");
			Mat sub1(frame_gray, roi);
			sub = sub1;
			face_cascade.detectMultiScale(sub, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

			for (size_t i = 0; i < faces.size(); i++)
			{
				
				nfaceX = subX + faces[i].x;
				nfaceY = subY + faces[i].y;
				dfaceX = abs(faceX - nfaceX);
				dfaceY = abs(faceY - nfaceY);
				
				if ((dfaceX > 2) && (dfaceY > 2)) {
					faceX = nfaceX;
					faceY = nfaceY;
					faceW = faces[i].width;
					faceH = faces[i].height;

					subX = faceX - 50;
					subY = faceY - 50;

					subW = faceW + 100;
					subH = faceH + 100;

					subX2 = subX + subW;
					subY2 = subY + subH;

					if (subX < 0) subX = 0;
					if (subY < 0) subY = 0;
					if (subX2 >= 640) {
						subX2 = 639;
						subW = subX2 - subX;
					}
					if (subY2 >= 480) {
						subY2 = 479;
						subH = subY2 - subY;
					}
 
				}
				
				loopCount = 0;
				
			}
		
			Rect roi1(subX, subY, subW, subH);
			roi = roi1;
			rectangle(color, roi, Scalar(1, 255, 1), 3);
			
			if (loopCount >= 40) {
				subX = -1;
				subY = -1;
				subW = -1;
				subH = -1;
				subX2 = -1;
				subY2 = -1;
				faceX = -1;
				faceY = -1;
				faceW = -1;
				faceH = -1;
				faceCX = -1;
				faceCY = -1;
				loopCount = 0;
				loopCount2 = 0;
				loopCount3 = 0;
			}

			loopCount++;
		
		}
		
		if ((subX != -1) && (subY != -1)) {
			
			faceCX = faceX + faceW*0.5;
			faceCY = faceY + faceH*0.5;

			Point faceCenter(faceCX, faceCY);
			arrowedLine(color, center, faceCenter, Scalar(94, 206, 165), 5);

			circle(color, faceCenter, 10, Scalar(1, 255, 1), 2);

			angleX = (faceCX - 320) * 0.109375;//convert position on X axis to angle of deflection from center point
			angleY = (faceCY - 240) * 0.089583 * -1;//convert position on Y axis to angle of deflection from center point					
		
			printf("angleX: %f angleY: %f\n", angleX, angleY);
		}

		if (loopCount >= 40) { //adjust how long the arrow stays without detecting a face
			subX = -1;
			subY = -1;
			subW = -1;
			subH = -1;
			subX2 = -1;
			subY2 = -1;
			faceX = -1;
			faceY = -1;
			faceW = -1;
			faceH = -1;
			faceCX = -1;
			faceCY = -1;
			loopCount = 0;
			loopCount2 = 0;
			loopCount3 = 0;
			
			printf("LoopCount: %d RESET\n", loopCount);
		}	
		
		//Show image with arrow
		imshow("Color", color);
		
		//imshow("ROI", roi);
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




