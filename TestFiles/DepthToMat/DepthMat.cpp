#include <opencv2/opencv.hpp>

#include <librealsense/rs.hpp>
#include <cstdio>

#include <iostream>

using namespace cv;
using namespace std;

int main(int, char**){
	cout << "DepthMat" << endl;	


	VideoCapture capture(2);
	Mat left;
	Mat right;
	Mat frame;


	namedWindow("Disparity View", 1);

	
		
	








	return(0);

}
