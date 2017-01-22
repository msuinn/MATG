#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int, char**){
	cout << "Video?" << endl;
	
	VideoCapture capture(0);

	if(!capture.isOpened()){
		cout << "Camera not opened" << endl;
	}

	namedWindow("Video",1);
	while(1){
		Mat frame;
		capture >> frame;
		
		imshow("Video", frame);
		
		if(waitKey(30) == 'c'){
			break;
		}
	}

	return(0);

}
