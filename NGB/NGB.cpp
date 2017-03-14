#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>
#include <libusb-1.0/libusb.h>


using namespace cv;
using namespace std;


typedef struct Vector{
	int angleX;
	int angleY;
	int magnitude;
}Vector; 


int main(int, char**) try{
	//haarcascade variables
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
 	CascadeClassifier face_cascade;

	//opencv variables
	const int matSize[2]={480,640};
	const Point center(matSize[1]/2, matSize[0]/2);
	vector<Rect> faces;
	Mat color(2, matSize, CV_8UC3);
	Mat frame_gray;
	rs::context rsCtx;
	namedWindow("Color", 1);
	bool faceFound = false;
	
	//added these variables
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
	
	Vector dataStruct;

	/*******************************************************
		Initialize Camera/Face Recognition
	*******************************************************/
	//Load the cascades
   	if( !face_cascade.load( face_cascade_name )){ 
		printf("--(!)Error loading\n"); 
		return -1; 
	}


	//Check to see how many attached devices there are
	if(rsCtx.get_device_count() == 0){
		return EXIT_FAILURE;
	}

    	// Enable a single device
    	rs::device * cam = rsCtx.get_device(0);

	//Enable the color stream and start camera
	cam->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, 60);
	cam->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60);
	cam->start();


	/*******************************************************
			Open USB Connection
	*******************************************************/
    	FILE *file;

	/*******************************************************
		    	    Main Loop
	*******************************************************/
	while(true){
		//Setting up the frames to analyze
		cam->wait_for_frames();
		color.data = (unsigned char*)cam->get_frame_data(rs::stream::color);
		cvtColor( color, color, CV_BGR2RGB);
		cvtColor( color, frame_gray, CV_BGR2GRAY);
		equalizeHist( frame_gray, frame_gray );

		//new
		Mat depth(2, matSize, CV_16U, (uchar *) cam->get_frame_data(rs::stream::depth));
		sub = frame_gray;

		//new face detection REMEMBER faceFound	
		
		//initial detection search whole mat
		if ((subX == -1) && (subY == -1)) {
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

			Mat sub1(frame_gray, roi);
			sub = sub1;
			face_cascade.detectMultiScale(sub, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

			for (size_t i = 0; i < faces.size(); i++) {
				
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

			dist = depth.at<unsigned int16_t>(faceY, faceX);
		
			faceFound = true;
			//printf("angleX: %f angleY: %f\n", angleX, angleY);
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
			
		}			
		
		//Send the data if there was a face, otherwise nothing
		if(faceFound){
			faceFound = false;

			//Calculate the vector components
			dataStruct.magnitude = (int)dist;
			dataStruct.angleX = (int)angleX;
			dataStruct.angleY = (int)angleY;
			
			file = fopen("/dev/ttyACM0","w");  //Opening device file
			
			fprintf(file,"%d",dataStruct.angleX); //Writing to the file			
			fprintf(file,"%c",'.'); //To separate digits

			fprintf(file,"%d",dataStruct.angleY); //Writing to the file			
			fprintf(file,"%c",','); //To separate digits

			fclose(file);
			
		}

		//Show the frame, not actually important
		imshow("Color", color);
		
		//increment the loop counter
		loopCount++;
		
		if(waitKey(30) == 'c'){
			break;
		}
	}
	

	fclose(file);

	return(0);
}

catch(const rs::error & e){
	printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), 			e.get_failed_args().c_str());
    	printf("    %s\n", e.what());
    	return EXIT_FAILURE;
}
