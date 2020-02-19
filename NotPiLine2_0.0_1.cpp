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
		if (m2.m00 < m1.m00 / 2)					//if the second dot is less than half the size of the first, remove it
			contours.pop_back();		
	}
	
	for (int i = 0; i < contours.size(); i++) {		//sees if the green dots are of relevance
		cv::Moments m = cv::moments(contours[i]);
		g_dots.push_back(cv::Point(m.m10 / m.m00, m.m01 / m.m00));
		int count_b = 0;
		for (int iy = 0; iy < sqrt(m.m00); iy++) {
			if (g_dots[i].y - iy > 0) {
				if (img_roi.at<uchar>(g_dots[i].y - iy, g_dots[i].x) == 255) //if there is black
					count_b++;
				cv::circle(img_rgb, cv::Point(g_dots[i].x, g_dots[i].y - iy), 2, cv::Scalar((iy * 20), (255 - (iy * 20)), 0), 5);
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
	std::vector<cv::Point> contour;

	//find all contours
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	//find biggest contour
	double gcont = 0; //größte contour
	int index = -1;
	for (int i = 0; i < contours.size(); i++) {
		cv::Moments m = cv::moments(contours[i]);
		//cout << "i " << i << "  size: " << m.m00 << endl;		//
		if (m.m00 > gcont) {
			gcont = m.m00;
			index = i;
		}
	}
	
	if (index != -1) {		//if there contours
		contour = contours[index];
		contours.clear();
		contours.push_back(contour);
	}

	return contours;
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
		if (v[i-1].size() < v[i].size()) {
			change = true;
			iter_swap(v.begin() + i - 1, v.begin() + i);
		}
	}
	if (change)
		contour_sort(v);
	return v;
}