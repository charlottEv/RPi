//include system librarys
#include <stdio.h> //for printf
#include <stdint.h> //uint8_t definitions
#include <stdlib.h> //for exit(int);
#include <string.h> //for errno
#include <errno.h> //error output

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

int main(){
 
	char device[]= "/dev/ttyACM0";
	int fd; //filedescriptor
	unsigned long baud = 9600;
	
	unsigned int compteur = 0;
	unsigned int compteurWhile = 0;	
	char state;
	int valueAvailable = 0;
	char oldState = 'a';
	char xcharvalue[] = "851";
	
	

	if((fd = serialOpen(device,baud))<0){
		cout<<"trouble opening"<<endl;
	}
	else cout<<"serialOpen OK"<<endl;


	if(wiringPiSetup() == -1){
		cout<<"unable to start wiringPi"<<endl;
	}
	else cout<<"wiringPiSetup OK"<<endl;
	
	cout<<"while : "<<endl;
	while(1){
		//if(compteur >= 0){
			serialPuts(fd,"x");
			//serialPuts(fd,xcharvalue);
			//serialPutchar(fd,120);//'x'
			//serialPutchar(fd,49);
			//serialPrintf(fd,"salut imstepf Je suis le RPi\n");
			//serialFlush(fd);
			//cout<<"send data "<<compteur<<endl;
			//compteur = 0;
		//}
		//compteur++;


		if(valueAvailable = serialDataAvail(fd)){
			cout<<"value of available: "<< valueAvailable<<endl;
			//waitKey(100);
			state = serialGetchar (fd);
			cout<<"received: "<<state<<endl;
		}
		
		if(state != oldState){
			switch(state){
				case 'a': cout<< "etat bottle"<<endl; 
					oldState = state; break;
				case 'b': cout<<"etat led"<<endl; 
					oldState = state; break;
				case 'c': cout<<"shutdown"<<endl;
					oldState = state; break;
				default: cout<<"default case"<<endl;break;
			}
		}
	}
		
	serialClose(fd);
	cout<<"end"<<endl;

	waitKey(10);
	return 0;
}
