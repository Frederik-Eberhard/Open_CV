//============================================================================
// Name        : NotPiLinie2.cpp
// Author      : Frederik Eberhard
// Version     : 0.0_1
// Copyright   : Your copyright notice
// Description : OpenCV linetracking program
//============================================================================

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

cv::Mat img_rgb;	//input 
cv::Mat img_red;	//color-reduced image
cv::Mat img_roi;	//roi of black-line
cv::Mat img_gre;	//green-dot

bool lineProgramm;			//which programmpart is running


							//___TEMP____
int lslider = 0;
int diff = 0;
//___________

 

//--Threshhold / Adjustable values----
float blackThresh	= 64;	//64	minimum average brightness to be black 
float greenThresh1	= 2;	//2		
float greenThresh2  = 10;	//10	minimum average brightness to be green  -> (the smaller, the better it sees green but also adds noise)
//____________________________________


void colorReduction(cv::Mat& img);
std::vector<std::vector<cv::Point>> get_green_dots(cv::Mat, std::vector<cv::Point>& g_dots);
cv::Mat make_roi(cv::Mat img);
std::vector<std::vector<cv::Point>> get_contour(cv::Mat img);
std::vector<float> get_angles(std::vector<std::vector<cv::Point>> c);
float get_direction(std::vector<float> g_angles, std::vector<float> b_angles, int& index_);
int get_instructions(float dir);
void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours, int index);
void show();

void windows();		//displays the windows
std::vector<std::vector<cv::Point>> contour_sort(std::vector<std::vector<cv::Point>>& v);
 
int main() {
	//Erstelle Objekt von Klasse cv::VideoCapture und öffne das Video "line.avi"
	cv::VideoCapture cap("line2.avi");
	
	//Prüfe ob das Video geöffnet werden konnte
	if (!cap.isOpened()) {
		std::cout << "Problem beim öffnen der Datei" << std::endl;
		return -1;
	}
	//Setze Framerate und die Auflösung
	cap.set(cv::CAP_PROP_FPS, 30);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);			
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);		

	//Füge Fenster zum Server hinzu
	cv::namedWindow("RGB", WINDOW_NORMAL);
	cv::namedWindow("RED", WINDOW_NORMAL);
	cv::namedWindow("ROI", WINDOW_NORMAL);
	cv::namedWindow("GRE", WINDOW_NORMAL);

	lineProgramm = true;		//starts with line-driving
	while (1) {
		cap.read(img_rgb);		//needed for actual video!!!
		//img_rgb = cv::imread("RoteZonePictures/IMG_20190523_143201426.jpg", CV_LOAD_IMAGE_COLOR);	//temp!!!
		if (img_rgb.empty()) {
			std::cout << "Fehler beim holen des Bildes" << std::endl;
			return -2;
		}
		
		if (lineProgramm) {			
			img_red = img_rgb.clone();

			//--color-reduction------------------
			colorReduction(img_red);	//reduces the colors to black, white and green

			//--binarisation--------------
			cv::inRange(img_red, cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0), img_roi);		//everything not white is treated as black
			cv::inRange(img_red, cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 0), img_gre);	//everything green is treat as green
			
			//--get-green-dots-------------------
			std::vector<cv::Point> green_dots;
			std::vector<vector<cv::Point>> green_contours = get_green_dots(img_gre, green_dots);	//maybe also parse the green_dots by reference  

			//--make-roi-------------------------
			img_roi = make_roi(img_roi);

			//--get-contours---------------------
			std::vector<std::vector<cv::Point>> contours; //für schwarz
			contours = get_contour(img_roi);
			
			//--angles---------------------------
			std::vector<float> green_angles = get_angles(green_contours);
			std::vector<float> black_angles = get_angles(contours);

			//--direction------------------------
			int contours_index;
			float direction = get_direction(green_angles, black_angles, contours_index);			//from 90 -> left  to -90 -> right  (exept 180 -> 180° turn)
			//cout << "direction: " << direction << endl;								///
			if (direction != 180) {
				int sendMotorA = (direction > 0) ? 255 : get_instructions(direction);						// B   A   --> Function procedually gives values for Motors A and B for Turning
				int sendMotorB = (direction > 0) ? get_instructions(direction) : 255;
				cout << "sendMotorA " << sendMotorA << "  sendMotorB " << sendMotorB << endl;
			}
			//--show-----------------------------			
			show(contours, green_contours, contours_index);
//			show();	//TEMP
		}
		else {		//--rote Zone----------------
			//--grayscale-conversion--
			cv::cvtColor(img_rgb, img_roi, cv::COLOR_BGR2GRAY);

			vector<Vec3f> circles;
			std::cout << "calculating circles" << std::endl;
			cv::Canny(img_roi,img_roi,150,300,5,true);			//60,300
			cv::HoughCircles(img_roi, circles, cv::HOUGH_GRADIENT, 1, 70, 300, 80, 10, 300);
			std::cout << "circles: " << circles.size() << std::endl;

			int a = 255;
			for (Vec3f c : circles) {
				cv::circle(img_rgb,Point(c[0],c[1]), c[2],Scalar(50,20,a),2);
				a--;
			}
			show();
			std::cout << "done" << std::endl;
		}
	}

	return 0;
}

void colorReduction(cv::Mat& img) {
	uchar* pixelPtr = img.data;
	for (int i = 0; i < img.rows; i++) {	
		for (int j = 0; j < img.cols; j++) {
			const int p = i*img.cols * 3 + j * 3;
			int a = (pixelPtr[p + 0] + pixelPtr[p + 1] + pixelPtr[p + 2]) / 3;	//average color value
			int b = (pixelPtr[p + 0] + pixelPtr[p + 2]) / 2 * greenThresh1;		//average color value without green multiplied by the greenThreshhold 1
			int c = pixelPtr[p + 1];
			if (b < c && a > greenThresh2) {				//b compared to the green color Value and a compared to greenThreshhold 2
				pixelPtr[p + 0] = 0;
				pixelPtr[p + 1] = 255;
				pixelPtr[p + 2] = 0;
			}else if(a < blackThresh){	//a compared to the blackThreshhold
				pixelPtr[p + 0] = 0;
				pixelPtr[p + 1] = 0;
				pixelPtr[p + 2] = 0;
			}else {
				pixelPtr[p + 0] = 255;
				pixelPtr[p + 1] = 255;
				pixelPtr[p + 2] = 255;
			}
		}
	}
}

std::vector<std::vector<cv::Point>> get_green_dots(cv::Mat img, std::vector<cv::Point>& g_dots) {
	std::vector<std::vector<cv::Point>> contours;
	
	//--green-dots-contours--------------
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	contour_sort(contours);

	for (int i = contours.size(); i > 2; i--) {		//goes backwards and removes all but the biggest two green dots
		contours.pop_back();
	}
	if (contours.size() > 1) {						//if there are two dots, see if they are similar size
		cv::Moments m1 = cv::moments(contours[0]);
		cv::Moments m2 = cv::moments(contours[1]);
		if (m2.m00 * 2 < m1.m00)					//if the second dot is less than half the size of the first, remove it			[CONSTANT: 2]
			contours.pop_back();		
	}
	
	for (int i = 0; i < contours.size(); i++) {		//sees if the green dots are of relevance
		cv::Moments m = cv::moments(contours[i]);
		g_dots.push_back(cv::Point(m.m10 / m.m00, m.m01 / m.m00));
		int count_b = 0;
		for (int iy = 0; iy < sqrt(m.m00); iy++) {
			if (g_dots[i].y - sqrt(m.m00)/2 - iy > 0) {
				if (img_roi.at<uchar>(g_dots[i].y - sqrt(m.m00) / 2 - iy, g_dots[i].x) == 255) //if there is black		(y-pos of middle of green dot - half the height of the green dot - iy) 
					count_b++;
				cv::circle(img_rgb, cv::Point(g_dots[i].x, g_dots[i].y - sqrt(m.m00) / 2 - iy), 2, cv::Scalar((iy * 20), (255 - (iy * 20)), 0), 5);
			}
		}
		if (count_b < sqrt(m.m00) * 0.5) {			//if less than half off all point checked aren't black, remove that point		[CONSTANT: 0.5]
			contours.erase(contours.begin() + i);
			g_dots.erase(g_dots.begin() + i);
			if (i == 0) i = -1;						//making sure that no dot is missed
		}
	}

	//--print--
	//for (int i = 0; i < contours.size(); i++) {
	//	cout << "i " << i << "  size: " << contours[i].size() << "  position: " << g_dots[i] << endl;	///
	//}
	//--print--

	return contours;
}

cv::Mat make_roi(cv::Mat img) {
	
	//get center and radius of bigger circle
	cv::Point cen(img.cols / 2, img.rows);
	int radius = img.cols / 2;
	
	//get roi containing the bigger circle
	cv::Rect r (0,cen.y - radius, radius*2,radius*2);

	//mask the roi to only let the hollow cicle through
	cv::Mat mask(img.size(), img.type(), Scalar::all(0));
	circle(mask, cen, radius, Scalar::all(255), -1);
	circle(mask, cen, radius - (img.cols / 8), Scalar::all(0), -1);

	//combine mask and image and return the image
	
	img = img & mask;
	return img;
}

std::vector<std::vector<cv::Point>> get_contour(cv::Mat img) {		
	std::vector<std::vector<cv::Point>> contours;
	
	//find all contours
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	//sort contours
	contour_sort(contours);
	
	if (contours.size() != 0) {		//adding an epty conntour for comparison incase of only two right lines with a small gap (only adding it if there is already a black contour in the picture)
		std::vector<cv::Point> c;
		contours.push_back(c);
	}
	//cout << "Black Contours" << endl;					///
	int gap = 0;
	int index = 0;
	for (int i = 1; i < contours.size(); i++) {
		cv::Moments m1 = cv::moments(contours[i-1]);
		cv::Moments m2 = cv::moments(contours[i]);
		if (m1.m00 - m2.m00 > gap) {
			gap = m1.m00 - m2.m00;
			index = i-1;
		}
	//	cout << "i " << i << "  m1: " << m1.m00 << "  m2: " << m2.m00 << "  diff:" << m1.m00 - m2.m00 << endl;		///
	}
	//cout << "biggest gap " << gap << "  index " << index << "  size " << contours.size() <<endl;			///
	

	for (int i = contours.size() - 1; i > index ; i--) {
		contours.pop_back();	
	}
	//cout << "size: " << contours.size() << endl;			///

	return contours;
}

std::vector<float> get_angles(std::vector<std::vector<cv::Point>> c) {
	#define PI 3.14159265
	float rows = img_rgb.rows;		//needed in float form for atan
	float cols = img_rgb.cols;
	std::vector<float> a;

	for (int i = 0; i < c.size(); i++) {
		cv::Moments m = moments(c[i]);
		cv::Point p = cv::Point(m.m10 / m.m00, m.m01 / m.m00);
		a.push_back(acos((p.x-cols/2) / sqrt(pow(p.x-cols/2,2)+pow(rows-p.y,2))) * 180 / PI);		//angel between 0 - 180 (>90 -> rechts; <90 -> links)
	//	cout << i << "Pos: " << p << "  angle: " << a[i] << endl;									///
	}
	return a;
}

float get_direction(std::vector<float> g_angles, std::vector<float> b_angles, int& index_) {		//index_ only for show
	if (g_angles.size() > 1) {		//180° turn
		index_ = -2;						//only for show()
		return 180;
	}
	if (b_angles.size() < 1) {		//if no black lines -> drive straight
		return 0;
	}

	if (g_angles.size() != 0) {		//if one green dot
		int index = 0;				//closest black angle
		float dif = 180;			//difference between closest black angle and green point
		for (int i = 0; i < b_angles.size(); i++) {
			if (abs(b_angles[i] - g_angles[0]) < dif) {
				index = i;
				dif = abs(b_angles[i] - g_angles[0]);
			}
		}
		index_ = index;						//only for show()
		return b_angles[index] - 90;
	}

	int index = 0;
	float angle = 180;
	for (int i = 0; i < b_angles.size(); i++) {
		if (abs(b_angles[i] - 90) < angle) {		//if the current distance to driving straight is smaller
			index = i; 
			angle = abs(b_angles[i] - 90);
		}
	}
	index_ = index;						//only for show()
	return b_angles[index] - 90;
}

int get_instructions(float dir) {			//-10° kaum   70+ ganz		//255 255 to 255 -255  --  nur die sich änderde zahl wird übergeben  --> map 90 - 0 to 255 - -255
	return 255 - (dir*dir * 51 / 490);				//currently only returns turning amout, without direction
}

void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours, int index) {
	cv::Scalar S = cv::Scalar(0, 0, 255);
	if (index == -2) {					//if there is a 180° turn
		S = cv::Scalar(0, 131, 255);
	}
	cv::drawContours(img_rgb, contours, index, S, 2);
	cv::drawContours(img_rgb, g_contours, -1, cv::Scalar(0, 255, 0), 2);
	windows();
}

void show() {
	windows();
}

void windows() {
	cv::imshow("RGB", img_rgb);
	cv::imshow("RED", img_red);
	cv::imshow("ROI", img_roi);
//	cv::imshow("GRE", img_gre);
	cv::waitKey(30000);
}


std::vector<std::vector<cv::Point>> contour_sort(std::vector<std::vector<cv::Point>>& v) {  //big -> small
	bool change = false;
	for (int i = 1; i < v.size(); i++) {
		cv::Moments m1 = cv::moments(v[i - 1]);
		cv::Moments m2 = cv::moments(v[i]);
		if (m1.m00 < m2.m00) {					//if m1 has a smaller Area than m2
			change = true;
			iter_swap(v.begin() + i - 1, v.begin() + i);
		}
	}
	if (change)
		contour_sort(v);
	return v;
}