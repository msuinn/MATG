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

#define WIDTH 640
#define HEIGHT 480


int main(int, char**) try{
	//haarcascade variables
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
 	CascadeClassifier face_cascade;

	//opencv variables
	const int matSize[2]={HEIGHT,WIDTH};
	const Point center(matSize[1]/2, matSize[0]/2);
	vector<Rect> faces;
	Mat color(2, matSize, CV_8UC3);
	Mat frame_gray;
	rs::context rsCtx;
	namedWindow("Color", 1);
	bool faceFound = false;
	
	//Face position and size, -1 indicates face not found
	int faceX = -1;
	int faceY = -1;
	int faceW = -1;
	int faceH = -1;

	//Top left corner of the submat around last location of face
	int subX = -1;
	int subY = -1;

	//Size of submat
	int subW = -1;
	int subH = -1;
	
	//Opposite corner of submat, for submat near edges of screen
	int subX2 = -1;
	int subY2 = -1;

	//nFace is used to calculate the difference (dFace) between the last
	//frame (face) and the current one (nFace) 
	int nfaceX = 0;
	int nfaceY = 0;
	int dfaceX = 0;
	int dfaceY = 0;

	//FaceC is the center of the fase after scaling
	int faceCX = -1;
	int faceCY = -1;

	//Deflection angles from centerpoint being sent to motor control board
	double angleX = 0.0;
	double angleY = 0.0;

	//Needed to communicate with ball bot, maybe
	double dist = 0.0;

	//loopCount increments with each loop, but is set to zero when a face
	//is found in the submat, when it reaches a threshold, program searches
	//entire mat 
	int loopCount = 0;
	//Number of times that a face has been recognised in the same region
	int loopCount2 = 0;
	//Number of times that a face was found outside of the ROI
	int loopCount3 = 0;

	//The region of interest that is used to create the submat
	Rect roi;
	
	//The submat
	Mat sub;
	
	Vector dataStruct;







	//Point2d pt;
	//bool first = true;
	//Mat test1, test2;





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
		
		/*******************************************************
		    	    	Initial Search, Entire Frame
		*******************************************************/
		//initial detection search whole mat, -1 indicates not found
		if ((subX == -1) && (subY == -1)) {
			face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
			for (size_t i = 0; i < faces.size(); i++)
			{
				//If no face found yet, store the location and size of first found
				if ((faceX == -1) && (faceY == -1)) {
					faceX = faces[i].x;
					faceY = faces[i].y;
					faceW = faces[i].width;
					faceH = faces[i].height;

				}
				//Otherwise, compare the location of the current face to the stored one
				else {
					nfaceX = faces[i].x;
					nfaceY = faces[i].y;
					dfaceX = abs(faceX - nfaceX);
					dfaceY = abs(faceY - nfaceY);
					
					//If the current face is near the stored value
					//modify counters and store current face
					if ((dfaceX < 100) && (dfaceY < 100)) {
						faceX = nfaceX;
						faceY = nfaceY;

						faceW = faces[i].width;
						faceH = faces[i].height;

						//Face has been found so zero iterations since last face
						loopCount = 0;
						//Face has been found near previous one, so zero iterations since last face in sub
						loopCount3 = 0;
						//Face has been found inside submat, so increment number of frames
						loopCount2++;						

					}
					//Otherwise, face was too far away, so out of range faces increments					
					else {
						//If there has been too many out of range faces, clear found counter
						loopCount3++;
						if (loopCount3 >= 5) loopCount2 = 0;
					}

				}

			}
			//If the face has been found locally in 2 or more consecutive frames, setup submat
			if (loopCount2 >= 2) {
				subX = faceX - 50;
				subY = faceY - 50;

				subW = faceW + 100;
				subH = faceH + 100;

				subX2 = subX + subW;
				subY2 = subY + subH;

				//Make sure that the submat is within the frame boundaries
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

				//Draw rectangle around submat
				Rect roi1(subX, subY, subW, subH);
				roi = roi1;
				rectangle(color, roi, Scalar(1, 1, 255), 3);

			}
		}

		/*******************************************************
		    	    	Found Face, Search Submat
		*******************************************************/
		//Case where submat has been initialized
		else {
			//Only look for faces in the submat
			Mat sub1(frame_gray, roi);
			sub = sub1;
			face_cascade.detectMultiScale(sub, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

			//For each face found in submat
			for (size_t i = 0; i < faces.size(); i++) {
				//Store location of new face relative to larger mat
				nfaceX = subX + faces[i].x;
				nfaceY = subY + faces[i].y;
				dfaceX = abs(faceX - nfaceX);
				dfaceY = abs(faceY - nfaceY);
				
				//If the face is relatively near the last, store value and new submat
				if ((dfaceX > 2) && (dfaceY > 2)) {
					//Store face value
					faceX = nfaceX;
					faceY = nfaceY;
					faceW = faces[i].width;
					faceH = faces[i].height;

					printf("FaceX: %d\n",faceX);
					printf("FaceY: %d\n",faceY); 					

					//Stretch submat to account for motion
					int sfaceX = 320 - (faceW/2);
					int sfaceY = 240 - (faceH/2);
 					
					if (faceX > sfaceX){
						sfaceX = (faceX - sfaceX)/3;
											
					}
					else if (faceX < sfaceX){
						sfaceX = (sfaceX - faceX)/3;					
					}
					
					if (faceY > sfaceY){
						sfaceY = (faceY - sfaceY)/3;
											
					}
					else if (faceY < sfaceY){
						sfaceY = (sfaceY - faceY)/3;					
					}					

					//Setup the submat dimensions
					subX = faceX - 50 - sfaceX;
					subY = faceY - 50 - sfaceY;

					subW = faceW + 100 + sfaceX;
					subH = faceH + 100 + sfaceY;

					subX2 = subX + subW;
					subY2 = subY + subH;

					//Make sure that the submat is still within mat
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
				
				//Set counter since last face found to zero
				loopCount = 0;
				
			}
		
			//Create the rectangle around the submat
			Rect roi1(subX, subY, subW, subH);
			roi = roi1;
			rectangle(color, roi, Scalar(1, 255, 1), 3);
			
			//If the face has NOT been found for 40 consecutive frames, restart search
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
		
		//If a face has been found, draw arrow to face
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
		
		//If the face has NOT been found for 40 consecutive frames, restart search again?
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




		
		/*

		if(first){
			cvtColor( color, test1, CV_BGR2GRAY);
			test1.convertTo(test1, CV_32FC1, 1.0/255.0);
			test2 = test1;
			first = false;
		} else{
			cvtColor( color, test1, CV_BGR2GRAY);
			test1.convertTo(test1, CV_32FC1, 1.0/255.0);
			pt = phaseCorrelate(test1, test2);
		}
		
		test2 = test1;
		cout << "POINT: " << 100*pt << endl;
*/



		
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
