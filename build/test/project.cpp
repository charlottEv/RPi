//include system librarys
#include <stdio.h> //for printf
#include <stdint.h> //uint8_t definitions
#include <stdlib.h> //for exit(int);
#include <string.h> //for errno
#include <errno.h> //error output
#include <iostream>
#include <sstream>
#include <string>

//opencv librarys
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <raspicam/raspicam_cv.h>
 
//wiring Pi
#include <wiringPi.h>
#include <wiringSerial.h>

using namespace std;
using namespace cv;


/*******constant STATE*******/
const char BOTTLE_DETECTION = 'a';
const char GO_HOME = 'b';
const char SHUTDOWN_RPi = 'c';

/*******constant bottle detection*******/
const int FRAME_WIDTH = 320;
const int FRAME_HEIGHT = 240;

const int FRAME_CUT = FRAME_HEIGHT/3; //80

const int HUEVALUE = 0;
const int SATURATIONVALUE = 1;
const int BRIGHTVALUE = 2;

const int RANGEMIN = 200;
const int RANGEMAX = 255;

const int MAX_OBJ = 30;
const int MIN_OBJECT_AREA = 10*10;
const int MAX_OBJECT_AREA = 1000;

raspicam::RaspiCam_Cv Camera;

/*******constant LED detection*******/
const int FRAME_WIDTH = 320;
const int FRAME_HEIGHT = 240;

const int WINDOWWIDTH = 320;
const int WINDOWHEIGHT = 30;
const int ONELINEHEIGHT = 1;

const int HUEVALUE = 0;
const int SATURATIONVALUE = 1;
const int BRIGHTVALUE = 2;

const int NBPIX = 5; //IMPAIR

const int MINTHRES_Y = 10;
const int MAXTHRES_Y = 30;

const int MINTHRES_G = 60;
const int MAXTHRES_G = 85;

const int MINTHRES_B = 85;
const int MAXTHRES_B = 130;

const int BIGTHRES_R = 130;//95
const int SMALLTHRES_R = 10;

/*******constant Communication Arduino-RPi******/
char state;



/*********************** PROGRAMME *********************/
int bottleDetection(){

    Mat cameraFeed, hsvImage;
    Mat morphImage2;
    Mat thresholdImage;

    double t = 0;
    double t1 = 0;
    double t2 =0;
    double t3 = 0;

    int erosion_size = 10;
    int erosion_size1 = 6;
    int erosion_size2 = 9;
    int erosion_size3 = 7;


    Mat element = getStructuringElement(MORPH_ELLIPSE,Size( 2*erosion_size + 1, 2*erosion_size+1 ),Point( erosion_size, erosion_size ) );
    Mat element1 = getStructuringElement(MORPH_ELLIPSE,Size( 2*erosion_size1 + 1, 2*erosion_size1+1 ),Point( erosion_size1, erosion_size1 ) );
    Mat element2 = getStructuringElement(MORPH_ELLIPSE,Size( 2*erosion_size2 + 1, 2*erosion_size2+1 ),Point( erosion_size2, erosion_size2 ) );
    Mat element3 = getStructuringElement(MORPH_ELLIPSE,Size(2*erosion_size3 +1,2*erosion_size3+1),Point(erosion_size3,erosion_size3));

	
	cout<<"------begin-------"<<endl;
	t2=(double)getTickCount();
	t1=(double)getTickCount();
	Camera.grab();
    Camera.retrieve(cameraFeed);
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"grab pic "<<t<<endl;
	
	
	///IMAGE PROCESSING:
    ///cut hupper part of image
	t1=(double)getTickCount();
    Mat cutImage = cameraFeed(Range(FRAME_CUT,FRAME_HEIGHT),Range::all());
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"cutImage "<<t<<endl;

	///Morphing image processing
	t1=(double)getTickCount();
	dilate(cutImage,morphImage2,element);
	erode(morphImage2,morphImage2,element1);
	dilate(morphImage2,morphImage2,element2);
	erode(morphImage2,morphImage2,element3);
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"dilate+erode "<<t<<endl;

    ///Threshold on lighter part
	t1=(double)getTickCount();
    inRange(morphImage2,RANGEMIN,RANGEMAX,thresholdImage);
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"threshold "<<t<<endl;

	///find objects and point(x,y)
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(thresholdImage,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	int position_objects[MAX_OBJ][2];

	t1=(double)getTickCount();
	if (hierarchy.size() > 0) {
	   int numObjects = hierarchy.size();
	   if(numObjects<MAX_OBJ){
		   for (int index = 0; index >= 0; index = hierarchy[index][0]) {
			   Moments moment = moments((cv::Mat)contours[index]);
			   double area = moment.m00;

			   if(area>(MIN_OBJECT_AREA) && area < (MAX_OBJECT_AREA)){
				   position_objects[index][0] = moment.m10/area;
				   position_objects[index][1] = moment.m01/area;
			   }
			circle(cutImage,Point(position_objects[index][0],position_objects[index][1]),7,Scalar(0,255,0),2);
			cout<<"x,y: " <<position_objects[index][0]<<","<<position_objects[index][1]<<endl;
		   }
	   }
	}
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"point centre mass "<<t<<endl;

	t3=((double)getTickCount()-t2)/getTickFrequency();
	cout<<"total time while "<<t3<<endl;
	cout<<"--------end-----------"<<endl;

	imshow("cut",cutImage);

    waitKey(10);
    return 0;
}

int init_camera(){
		
	///setting camera
    Camera.set(CV_CAP_PROP_FORMAT, CV_8UC1);
	Camera.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    Camera.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	Camera.set(CV_CAP_PROP_EXPOSURE,0.2);
	Camera.set(CV_CAP_PROP_GAIN,0.1);

    cout<<"Opening Camera..."<<endl;
    if (!Camera.open()) {
		cerr<<"Error opening the camera"<<endl;
		return -1;
	}
	return 0;

}


/*******MAIN*********/
int main(){
	char oldState = 'b';
	
	state = 'a';
	
	if(init_camera() == 0)
		cout<<"camera init"<<endl;
	
	while(1){
		//read state Arduino
		
		//if(state != oldState){
			switch(state){
				case BOTTLE_DETECTION: 
					cout<< "state bottle"<<endl; 
					oldState = state; 
					bottleDetection();
					break;
				case GO_HOME: 
					cout<<"state led"<<endl; 
					oldState = state; 
					break;
				case SHUTDOWN_RPi: 	
					cout<<"state shutdown"<<endl;
					oldState = state; 
					break;
				default: 
					cout<<"default case"<<endl;
					break;
			}
		//}
	}
 
 	cout<<"Stop camera..."<<endl;
    Camera.release();
	return 0;
}
