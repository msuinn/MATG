#include <opencv2/opencv.hpp>

#include <librealsense/rs.hpp>

#include <cstdio>

#include <iostream>

#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;

int main(int, char**) try{

	//OpenCV Mat initialization
	const int matSize[2]={480,640};

	Mat d = Mat::zeros(2,matSize,CV_64F);

	Mat c = Mat::zeros(2,matSize,CV_64FC3);

	namedWindow("depth",1);
	namedWindow("Test", 1);	

	Mat adjMap;

	//double min = 0, max = 0;

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
	dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 60);

    	dev->start();


	while(true){
        // This call waits until a new coherent set of frames is available on a device
        // Calls to get_frame_data(...) and get_frame_timestamp(...) on a device will return stable 		values until wait_for_frames(...) is called
        	dev->wait_for_frames();

        	// Retrieve depth data, which was previously configured as a 640 x 480 image of 
		// 16-bit depth values
        	const uint16_t * depth_frame = reinterpret_cast<const uint16_t *>
			(dev->get_frame_data(rs::stream::depth));

		const uint16_t * color_frame = reinterpret_cast<const uint16_t *>
			(dev->get_frame_data(rs::stream::color));

		
		Mat test(2, matSize, CV_16U, (uchar *) dev->get_frame_data(rs::stream::depth));
		Mat depth8u = test;
		depth8u.convertTo(depth8u, CV_8UC1, 255.0/1000);
		applyColorMap(depth8u, adjMap, COLORMAP_RAINBOW);
		imshow("Test", adjMap);

		
		for(int y=0; y<480; ++y){
            		for(int x=0; x<640; ++x){		                
				int depth = *(depth_frame++);
				d.at<double>(y,x) = depth;

				int color = *color_frame++; //Color error?
				c.at<double>(y,x) = color;
           		}
        	}
		
    
		/////////////////////////commented code for heat mapping	
		//minMaxIdx(d, &min,&max);
		//convertScaleAbs(d,adjMap,255/max);
		//d.convertTo(adjMap,CV_8UC1);	
	
		//cout << "this is the depth" << d.at<double>(200,200) << endl;	
		//Mat fCM;
		//applyColorMap(adjMap, fCM, COLORMAP_RAINBOW);
	
		//imshow("depth", adjMap);
		
		//imshow("colors", fCM);
		/////////////////////////////////////////////////////////////	

		//imshow("depth", d);
		//imshow("color", c);

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
