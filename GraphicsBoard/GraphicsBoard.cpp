#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>
#include <libusb-1.0/libusb.h>


using namespace cv;
using namespace std;


typedef struct Vector{
	float angleX;
	float angleY;
	float magnitude;
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
	

	//usb variables
	libusb_device **devs;
	libusb_context *ctx = NULL;
	libusb_device_handle *dev_handle;
	int retVal;
	ssize_t cnt;
	//Vendor ID of the device, should change per board
	const int vid = 10755;
	//Product ID of the device, should change per board
	const int pid = 67;
	//The interface ID, chosen as this one has 2 endpoints
	const int iid = 1;
	//Max size of read data transfer
	const int dataSize = 1;
	//Data pointer for transfer
	unsigned char *data = new unsigned char[dataSize];
	//Which interface endpoint to use, found in EP Address field
	const int epAddr = 4;
	//The address of the read endpoint
	const int inEp = 131;
	//How many bytes are written in bulk transfer
	int bytesWritten = 0;
	//Data to send
	Vector dataStruct;
	//Pointer to data to send
	unsigned char *structPtr;



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
	//Initialize and quit if fails
	retVal = libusb_init(&ctx);
	if(retVal < 0){
		cout << "Init Error " << retVal << endl;
		delete[] data;
		return(1);
	}

	//Set the libusb library to verbose mode
	libusb_set_debug(ctx, 3);

	//Get a list of length cnt of all the USB devices, or error
	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0){
		cout << "Get Device Error" << endl;
	}


	//Attempt to open communication
	dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
	if(dev_handle == NULL){
		cout << "Cannot open communication with board" << endl;
	}

	//Free the device list, it is no longer needed
	libusb_free_device_list(devs, 1);
	

	//Check for a kernal driver attached to device
	//If a driver is attached, interface cannot be claimed and no IO
	if(libusb_kernel_driver_active(dev_handle, iid) == 1){
		cout << "Kernal driver present" << endl;
		if(libusb_detach_kernel_driver(dev_handle, iid) == 0){
					cout << "Kernal driver detached" << endl;
		}	
	}

	//Claim the device interface for IO, boards tend to have just 1
	retVal = libusb_claim_interface(dev_handle, iid);
	if(retVal < 0){
		cout << "Cannot claim interface" << endl;
	}




	/*******************************************************
		    Wait for Motor Control Board
	*******************************************************/
	//Sometimes boards start up in odd order, so wait for 's'
	//from motor board before starting
	data[0] = 'a';
	while(data[0] != 's'){
		retVal = libusb_bulk_transfer(dev_handle, (inEp | LIBUSB_ENDPOINT_IN), data, 1, 				&bytesWritten, 0);
		if(retVal != 0)	{
			cout << "Read failed" << endl;
			cout << libusb_error_name(retVal) << endl;
		}
		cout << "Read: " << data[0] << endl;
	}
	delete[] data;



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
			
			//printf("LoopCount: %d RESET\n", loopCount);
		}	
		
		
		
		
		
		//Send the data if there was a face, otherwise nothing
		if(faceFound){
			faceFound = false;

			//Calculate the vector components
			dataStruct.magnitude = dist;
			dataStruct.angleX = angleX;
			dataStruct.angleY = angleY;

			//Cast data pointer to expected type
			structPtr = reinterpret_cast<unsigned char *>(&dataStruct);

			//Write the data
			retVal = libusb_bulk_transfer(dev_handle, (epAddr | LIBUSB_ENDPOINT_OUT), structPtr, sizeof(Vector), &bytesWritten, 0);
			if(retVal != 0 || bytesWritten != sizeof(Vector)){
				cout << "Write failed" << endl;
			} 
		}
		
	
		//Show the frame, not actually important
		imshow("Color", color);
		
		//increment the loop counter
		loopCount++;
		
		if(waitKey(30) == 'c'){
			break;
		}
	}
	

	/*******************************************************
		    	    Clean Up
	*******************************************************/
	//For whatever reasone, this is the only clean up that can fail
	retVal = libusb_release_interface(dev_handle, 1);
	if(retVal!=0) {
		cout << "Cannot release interface" << endl;
		return(1);
	}
	
	//All these guys are void
	libusb_close(dev_handle);
	libusb_exit(ctx);


	return(0);
}

catch(const rs::error & e){
	printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), 			e.get_failed_args().c_str());
    	printf("    %s\n", e.what());
    	return EXIT_FAILURE;
}
