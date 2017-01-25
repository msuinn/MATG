#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int, char**){
	//Camera 0 is the green one, 1 is possibly depth, 2 is normal vision
	//Camera 3+ is invalid
	VideoCapture capture(2);
	Mat frame;
	Mat normal;
	Mat depth;

	//Check to see if the camera was succesfully opened
	if(!capture.isOpened()){
		cout << "Camera not opened" << endl;
		return(-1);
	}

	//Create a window named "Video" that isn't resizeable
	namedWindow("Video", 1);
	while(1){
		
		//Grab a frame
		capture.grab();
		
		//Retrieve the BGR image and put it in normal
		//Depth map goes to depth
		capture.retrieve(normal, CV_CAP_OPENNI_BGR_IMAGE);
		capture.retrieve(depth, CV_CAP_OPENNI_DEPTH_MAP);

		//Inefficiently combine the two captures and concat
		Mat combine[2] = {normal, depth};
		hconcat(combine, 2, frame);
		
		//Actually show the image
		imshow("Video", frame);
		
		//Not really working, but should end on 'c' key
		if(waitKey(30) == 'c'){
			break;
		}
	}

	return(0);

}
