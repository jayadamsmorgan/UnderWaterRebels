
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include<winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include <opencv2/opencv.hpp>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "opencv2/core/mat.hpp"
#include "opencv/cxcore.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
#include <vector>
#include <string>
#include <fstream>


using namespace std;
using namespace cv;

#pragma comment(lib,"ws2_32.lib") //Winsock Library

const static int SENSITIVITY_VALUE = 35;

const static int BLUR_SIZE = 10;

int theObject[2] = { 0,0 };

Rect objectBoundingRectangle = Rect(0, 0, 0, 0);

bool kuleb = true;

vector< vector<Point> > contours;
vector<Vec4i> hierarchy;

#define ardPort 8000
#define mobPort 8888
#define BUFLEN 22

string ardIp;
string mobIp;

char toArd[5];
char fromArd[5];
char toMob[5];
char fromMob[5];

Mat temp;

Mat frame1, frame2;

Mat grayImage1, grayImage2;

Mat differenceImage;

Mat thresholdImage;

VideoCapture capture;

string intToString(int number) {

	std::stringstream ss;
	ss << number;
	return ss.str();
}

void searchForMovement(Mat thresholdImage, Mat &cameraFeed) {

	bool objectDetected = false;
	
	//thresholdImage.copyTo(temp);
	temp = thresholdImage.clone();
	waitKey(10);

	findContours(temp, contours, hierarchy, CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);// retrieves external contours

	if (contours.size()>0)objectDetected = true;
	else objectDetected = false;

	if (objectDetected) {
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(contours.size() - 1));
		//make a bounding rectangle around the largest contour then find its centroid
		//this will be the object's final estimated position.
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
		int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

		//update the objects positions by changing the 'theObject' array values
		theObject[0] = xpos, theObject[1] = ypos;
		cout << "We have movement" << endl;
	}
	//make some temp x and y variables so we dont have to type out so much
	
	int x = theObject[0];
	int y = theObject[1];

	//draw some crosshairs around the object
	circle(cameraFeed, Point(x, y), 20, Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);

	//write the position of the object to the screen
	putText(cameraFeed, "Tracking object at (" + intToString(x) + "," + intToString(y) + ")", Point(x, y), 1, 1, Scalar(255, 0, 0), 2);
	



}
int main() {

	fstream in("NetworkData.txt");

	in >> ardIp;
	in >> mobIp;

	struct sockaddr_in Arduino;
	struct sockaddr_in Mobile;
	int a, Ardlen = sizeof(Arduino);
	int m,Moblen = sizeof(Mobile);
	WSADATA wsa;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	if ((a = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() a failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	if ((m = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() m failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}


	memset((char *)&Arduino, 0, sizeof(Arduino));
	Arduino.sin_family = AF_INET;
	Arduino.sin_port = htons(ardPort);
	char* ardc = new char[ardIp.size()];
	Arduino.sin_addr.S_un.S_addr = inet_addr(ardc); 

	memset((char *)&Mobile, 0, sizeof(Mobile));
	Mobile.sin_family = AF_INET;
	Mobile.sin_port = htons(ardPort);
	char* mobc = new char[mobIp.size()];
	Mobile.sin_addr.S_un.S_addr = inet_addr(mobc);

	//some boolean variables for added functionality
	bool objectDetected = false;
	//these two can be toggled by pressing 'd' or 't'
	bool debugMode = false;
	bool trackingEnabled = false;
	//pause and resume code
	bool pause = false;
	
	//set up the matrices that we will need
	//the two frames we will be comparing


	while (1) {

		cv::VideoCapture camera(0);
		
		while (true) {

			//read first frame
			camera >> frame1;
			//convert frame1 to gray scale for frame differencing
			cv::cvtColor(frame1, grayImage1, COLOR_BGR2GRAY);
			//copy second frame
			camera >> frame2;
			//convert frame2 to gray scale for frame differencing
			cv::cvtColor(frame2, grayImage2, COLOR_BGR2GRAY);
			//perform frame differencing with the sequential images. This will output an "intensity image"
			//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
			cv::absdiff(grayImage1, grayImage2, differenceImage);
			//threshold intensity image at a given sensitivity value
			cv::threshold(differenceImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			if (debugMode == true) {
				//show the difference image and threshold image
				cv::imshow("Difference Image", differenceImage);
				cv::imshow("Threshold Image", thresholdImage);
				waitKey(10);
			}
			else {
				//if not in debug mode, destroy the windows so we don't see them anymore
				cv::destroyWindow("Difference Image");
				cv::destroyWindow("Threshold Image");
				waitKey(10);
			}

			
			//blur the image to get rid of the noise. This will output an intensity image
			cv::blur(thresholdImage, thresholdImage, cv::Size(BLUR_SIZE, BLUR_SIZE));
			//threshold again to obtain binary image from blur output
			cv::threshold(thresholdImage, thresholdImage, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			
			waitKey(10);
			
			if (kuleb)
			{
				cout << "kuleb  " << kuleb;
				imshow("Difference Image", differenceImage);
				waitKey(10);
				imshow("Final Threshold Image", thresholdImage);
				
				waitKey(10);
				kuleb = false;
			}

			if (debugMode == true) {
				//show the threshold image after it's been "blurred"

				imshow("Final Threshold Image", thresholdImage);
				waitKey(10);

			}
			else {
				//if not in debug mode, destroy the windows so we don't see them anymore
				cv::destroyWindow("Final Threshold Image");
			}

			//if tracking enabled, search for contours in our thresholded image
			trackingEnabled = true;
			if (trackingEnabled) {

				searchForMovement(thresholdImage, frame1);
			}

			//show our captured frame
			imshow("Frame1", frame1);
			waitKey(10);
			//check to see if a button has been pressed.
			//this 10ms delay is necessary for proper operation of this program
			//if removed, frames will not have enough time to referesh and a blank 
			//image will appear.
			switch (waitKey(10)) {
				trackingEnabled = true;
			case 27: //'esc' key has been pressed, exit program.
				return 0;
			//case 116: //'t' has been pressed. this will toggle tracking
			//	trackingEnabled = !trackingEnabled;
			//	if (trackingEnabled == false) cout << "Tracking disabled." << endl;
			//	else cout << "Tracking enabled." << endl;
			//	break;
			case 100: //'d' has been pressed. this will debug mode
				debugMode = !debugMode;
				if (debugMode == false) cout << "Debug mode disabled." << endl;
				else cout << "Debug mode enabled." << endl;
				break;
			case 112: //'p' has been pressed. this will pause/resume the code.
				pause = !pause;
				if (pause == true) {
					cout << "Code paused, press 'p' again to resume" << endl;
					while (pause == true) {
						//stay in this loop until 
						switch (waitKey()) {
							//a switch statement inside a switch statement? Mind blown.
						case 112:
							//change pause back to false
							pause = false;
							cout << "Code Resumed" << endl;
							break;
						}
					}
				}



			}
		}
		//release the capture before re-opening and looping again.
		capture.release();
	}

	return 0;

}