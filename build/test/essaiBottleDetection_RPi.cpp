#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <raspicam/raspicam_cv.h>

using namespace cv;
using namespace std;

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




int main(){

    raspicam::RaspiCam_Cv Camera;

    Mat cameraFeed, hsvImage;
    Mat morphImage, morphImage1,morphImage2;
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


 

    while(1){

	cout<<"------begin-------"<<endl;
	t2=(double)getTickCount();
	t1=(double)getTickCount();
	Camera.grab();
        Camera.retrieve(cameraFeed);
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"grab pic "<<t<<endl;
	
        ///cut hupper part of image
	t1=(double)getTickCount();
	//cvtColor(cameraFeed,cameraFeed,COLOR_BGR2GRAY);
        Mat cutImage = cameraFeed(Range(FRAME_CUT,FRAME_HEIGHT),Range::all());
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"cutImage "<<t<<endl;
	//imshow("cutGray",cutImage);

        ///Morphology (opening and closing) processing
	//t1=(double)getTickCount();
	//morphologyEx( cutImage, morphImage, MORPH_CLOSE, element );
        //morphologyEx( morphImage, morphImage1, MORPH_OPEN, element1 );
        //morphologyEx( morphImage1, morphImage2, MORPH_CLOSE, element2 );
	//t=((double)getTickCount()-t1)/getTickFrequency();
	//cout<<"morphology processing "<<t<<endl;

	t1=(double)getTickCount();
	//MORPHING
	dilate(cutImage,morphImage2,element);
	erode(morphImage2,morphImage2,element1);
	dilate(morphImage2,morphImage2,element2);
	erode(morphImage2,morphImage2,element3);
	t=((double)getTickCount()-t1)/getTickFrequency();
		cout<<"dilate+erode "<<t<<endl;

        ///Threshold on lighter part
	t1=(double)getTickCount();
        //cvtColor(morphImage2,morphImage2,COLOR_BGR2GRAY);
        inRange(morphImage2,RANGEMIN,RANGEMAX,thresholdImage);
	t=((double)getTickCount()-t1)/getTickFrequency();
	cout<<"threshold "<<t<<endl;

//        imshow("morph",morphImage2);
//        imshow("thres",thresholdImage);

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
	cout<<"total time while"<<t3<<endl;
	cout<<"--------end-----------"<<endl;

	imshow("cut",cutImage);


        waitKey(10);
    }

	cout<<"Stop camera..."<<endl;
    Camera.release();
    return 0;
}


