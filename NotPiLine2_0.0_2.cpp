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
cv::Mat img_gre;	//green-dot
cv::Mat img_roi;	//roi of black-line

std::vector<std::vector<cv::Point>> get_green_dots(cv::Mat, std::vector<cv::Point>& g_dots);
cv::Mat make_roi(cv::Mat img);
std::vector<std::vector<cv::Point>> get_contour(cv::Mat img);
std::vector<float> get_angles(std::vector<std::vector<cv::Point>> c);
float get_direction(std::vector<float> g_angles, std::vector<float> b_angles);
void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours);

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
	cv::namedWindow("RGB");
	cv::namedWindow("ROI");
	cv::namedWindow("GRE");


	while (1) {
		cap.read(img_rgb);
		if (img_rgb.empty()) {
			std::cout << "Fehler beim holen des Bildes" << std::endl;
			return -2;
		}
		
		//--green-binarisation---------------
		cv::cvtColor(img_rgb, img_gre, cv::COLOR_BGR2HSV);
		cv::inRange(img_gre, cv::Scalar(60, 100, 25), cv::Scalar(90, 255, 150), img_gre);

		//--black-binarisation---------------				//temp 
		cv::cvtColor(img_rgb, img_roi, cv::COLOR_BGR2GRAY);
		cv::inRange(img_roi, 0, 50, img_roi);
		img_roi -= img_gre;									//subtracting green image from line image (roi)

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
		float direction = get_direction(green_angles, black_angles);			//from 90 -> left  to -90 -> right  (exept 180 -> 180° turn)
		cout << "direction: " << direction << endl;								//
		//--show-----------------------------
		show(contours, green_contours);
	}

	return 0;
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
	for (int i = 0; i < contours.size(); i++) {
		cout << "i " << i << "  size: " << contours[i].size() << "  position: " << g_dots[i] << endl;
	}
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

std::vector<std::vector<cv::Point>> get_contour(cv::Mat img) {		//WIP
	std::vector<std::vector<cv::Point>> contours;
	
	//find all contours
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	//sort contours
	contour_sort(contours);
	
	if (contours.size() != 0) {		//adding an epty conntour for comparison incase of only two right lines with a small gap (only adding it if there is already a black contour in the picture)
		std::vector<cv::Point> c;
		contours.push_back(c);
	}
	//cout << "Black Contours" << endl;					//
	int gap = 0;
	int index = 0;
	for (int i = 1; i < contours.size(); i++) {
		cv::Moments m1 = cv::moments(contours[i-1]);
		cv::Moments m2 = cv::moments(contours[i]);
		if (m1.m00 - m2.m00 > gap) {
			gap = m1.m00 - m2.m00;
			index = i-1;
		}
	//	cout << "i " << i << "  m1: " << m1.m00 << "  m2: " << m2.m00 << "  diff:" << m1.m00 - m2.m00 << endl;		//
	}
	cout << "biggest gap " << gap << "  index " << index << "  size " << contours.size() <<endl;			//
	

	for (int i = contours.size() - 1; i > index ; i--) {
		contours.pop_back();	
	}
	cout << "size: " << contours.size() << endl;			//

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
		cout << i << "Pos: " << p << "  angle: " << a[i] << endl;									//
	}
	return a;
}

float get_direction(std::vector<float> g_angles, std::vector<float> b_angles) {
	if (g_angles.size() > 1) {		//180° turn
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
	return b_angles[index] - 90;

}

void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours) {
	cv::drawContours(img_rgb, contours, -1, cv::Scalar(0, 0, 255), 2);
	cv::drawContours(img_rgb, g_contours, -1, cv::Scalar(0, 255, 0), 2);
	cv::imshow("RGB", img_rgb);
	cv::imshow("ROI", img_roi);
	cv::imshow("GRE", img_gre);
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