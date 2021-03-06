#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>
#include <libusb-1.0/libusb.h>


using namespace cv;
using namespace std;

//Message format
typedef struct Vector{
	float angle;
	float magnitude;
}Vector; 

//Enum for return codes due to errors/normal execution
enum ReturnCodes{
	NO_ERROR = 0,
	ERROR_LOAD_CASCADE,
	ERROR_NO_RS_DEVICES,
	ERROR_INIT_USB,
	ERROR_NO_USB_DEVICES,
	ERROR_OPEN_USB,
	ERROR_CLAIM_INTERFACE,
	ERROR_RS_ERROR
};


//Global variables, for now
const int dataSize = 1;
//Data pointer for transfer
unsigned char *data = new unsigned char[dataSize];
libusb_device **devs;
libusb_context *ctx = NULL;
libusb_device_handle *dev_handle;


//Function prototypes
void cleanup(int retCode);


int main(int, char**) try{
	//haarcascade variables
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
 	String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
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
	

	//usb variables

	int retVal;
	ssize_t cnt;
	//Vendor ID of the device, should change per board
	const int vid = 10755;
	//Product ID of the device, should change per board
	const int pid = 67;
	//The interface ID, chosen as this one has 2 endpoints
	const int iid = 1;
	
	
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
		cleanup(ERROR_LOAD_CASCADE);
		return(ERROR_LOAD_CASCADE); 
	}


	//Check to see how many attached devices there are
	if(rsCtx.get_device_count() == 0){
		cleanup(ERROR_NO_RS_DEVICES);
		return(ERROR_NO_RS_DEVICES);
	}

    	// Enable a single device
    	rs::device * cam = rsCtx.get_device(0);

	//Enable the color stream and start camera
	cam->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60);
	cam->start();




	/*******************************************************
			Open USB Connection
	*******************************************************/
	//Initialize and quit if fails
	retVal = libusb_init(&ctx);
	if(retVal < 0){
		cleanup(ERROR_INIT_USB);
		return(ERROR_INIT_USB);
	}

	//Set the libusb library to verbose mode
	libusb_set_debug(ctx, 3);

	//Get a list of length cnt of all the USB devices, or error
	cnt = libusb_get_device_list(ctx, &devs);
	if(cnt < 0){
		cleanup(ERROR_NO_USB_DEVICES);
		return(ERROR_NO_USB_DEVICES);
	}


	//Attempt to open communication
	dev_handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
	if(dev_handle == NULL){
		cleanup(ERROR_OPEN_USB);
		return(ERROR_OPEN_USB);
	}
	

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
		cleanup(ERROR_CLAIM_INTERFACE);
		return(ERROR_CLAIM_INTERFACE);
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


		//Detect faces and place coordinates in faces vector
		face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 
			0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

		//For each face, find the center and draw an arrow from center to face
		//Also print out the matching pixel depth from depth stream
		for( size_t i = 0; i < faces.size(); i++ ){
			faceFound = true;
    			Point faceCenter( faces[i].x + faces[i].width*0.5, 
				faces[i].y + faces[i].height*0.5 );
			arrowedLine(color, center, faceCenter, Scalar( 94, 206, 165), 5);
		}


		//Send the data if there was a face, otherwise nothing
		if(faceFound){
			faceFound = false;

			//Reorient coordinates with 0 at center, not sure why there is 
			//Correction of 80 though
			faces[0].x -= (center.x - 80);
			faces[0].y -= (center.y - 80);

			//Calculate the vector components
			dataStruct.magnitude = sqrtf((faces[0].x * faces[0].x) + 
				(faces[0].y * faces[0].y));
			dataStruct.angle = -((dataStruct.magnitude == 0) ? 0:
				(faces[0].x == 0 && faces[0].y > 0) ? M_PI / 2:
				(faces[0].x == 0 && faces[0].y < 0) ? -M_PI / 2:
				atan2f(faces[0].y, faces[0].x));

			//Cast data pointer to expected type
			structPtr = reinterpret_cast<unsigned char *>(&dataStruct);

			//Write the data
			retVal = libusb_bulk_transfer(dev_handle, (epAddr | LIBUSB_ENDPOINT_OUT), 					structPtr, sizeof(Vector), &bytesWritten, 0);
			if(retVal != 0 || bytesWritten != sizeof(Vector)){
				cout << "Write failed" << endl;
			} 
		}
		
	
		//Show the frame, not actually important
		imshow("Color", color);
		
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

	cleanup(NO_ERROR);
	return(NO_ERROR);
}

catch(const rs::error & e){
	printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), 			e.get_failed_args().c_str());
    	printf("    %s\n", e.what());
    	return ERROR_RS_ERROR;
}





//Function that works off of the return code enum
//Since different variables are not initialized 
//Until later, closing unitialized vairables can cause
//Errors or undefined behavior
void cleanup(int retCode){
	switch(retCode){
		case(NO_ERROR):		
		case(ERROR_CLAIM_INTERFACE):
			libusb_close(dev_handle);
			libusb_free_device_list(devs, 1);
		case(ERROR_OPEN_USB):
		case(ERROR_NO_USB_DEVICES):			
		case(ERROR_INIT_USB):
			libusb_exit(ctx);
		case(ERROR_NO_RS_DEVICES):
		case(ERROR_LOAD_CASCADE):
			delete[] data;
		
			break;
	}
}










