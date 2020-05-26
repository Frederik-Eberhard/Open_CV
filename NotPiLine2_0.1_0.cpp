//=========================================================================================
// Name			: NotPiLinie2.cpp
// Author		: Frederik Eberhard
// Version		: 0.0_1
// Copyright	: Your copyright notice
// Description	: OpenCV linetracking program
//======================U=P=D=A=T=E==L=O=G=================================================
//29.04.20		: Green Dot direction Update (orientation to black line) 
//02.05.20		: RGB Window renamed to IMG because of unknown bug
//03.05.20		: Green Dot Update changed from minAreaRect to approxPolyDP
//21.05.20		: Green Dot Update changed to manually getting approximated rectangle 
//					--> possible improvement in better defining most suitable corner
//					--> corners need to be sorted in clockwise fashion!!
//25.05.20		: Green Dot corners clockwise sorting working
//=========================================================================================

#include <iostream>
#include <math.h>
#include "opencv2/opencv.hpp"

#define PI 3.14159265

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
float blackThresh	= 64;				//64	minimum average brightness to be black 
float greenThresh1	= 2;				//2		
float greenThresh2  = 10;				//10	minimum average brightness to be green  -> (the smaller, the better it sees green but also adds noise)
float CONTOUR_APPROX_SCALE = 0.07;		//		gets approximate contour (0.1 -> scale for approximate accuracy)	[CONSTANT: 0.07]
//____________________________________

//--Structs---------------------------

 struct GreenDot {
	std::vector<cv::Point> contour;
	cv::Point center;
	RotatedRect rect;
	std::vector<cv::Point> corners;		//two dimensional Point 
	cv::Point2f perpVect[4];	//perpendicular vector to the sides (with floatvalues)
	float size,h;
	int orientationIndex;				//index of perpVetor that points up
	bool left;								//true -> left, false -> right
}g_dot;

//____________________________________

void colorReduction(cv::Mat& img);
std::vector<GreenDot> get_green_dots_struct(cv::Mat img);
std::vector<std::vector<cv::Point>> get_green_dots(cv::Mat, std::vector<cv::Point>& g_dots);
cv::Mat make_roi(cv::Mat img);
std::vector<std::vector<cv::Point>> get_contour(cv::Mat img);
std::vector<float> get_angles(std::vector<std::vector<cv::Point>> c);
float get_direction(std::vector<float> g_angles, std::vector<float> b_angles, int& index_);
int get_instructions(float dir);
void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours, std::vector<GreenDot>, int index);
void show();

void windows();		//displays the windows
void getRect(std::vector<cv::Point>& c);
std::vector<std::vector<cv::Point>> contour_sort(std::vector<std::vector<cv::Point>>& v);
float dotP(cv::Point2f a, cv::Point2f b);
float vectSize(cv::Point2f a);
float vectAngle(cv::Point2f a, cv::Point2f b);


int main() {
	std::cout << "START" << std::endl;
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
	cv::namedWindow("IMG", WINDOW_NORMAL);
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
			std::cout << "Get Green Dots" << std::endl;

			std::vector<GreenDot> green_dots_a = get_green_dots_struct(img_gre);		//replaces above function
			std::cout << "Got GD" << std::endl;
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
			show(contours, green_contours, green_dots_a, contours_index);		
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

std::vector<GreenDot> get_green_dots_struct(cv::Mat img) {
	std::cout << "Get Green Dots Structs" << std::endl;
	std::vector<GreenDot> g_dots;
	std::vector<std::vector<cv::Point>> contours;						//contours of all green dots

	//--green-dots-contours--------------
	cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	contour_sort(contours);
	for (int i = contours.size(); i > 2; i--) {		//goes backwards and removes all but the biggest two green dots
		contours.pop_back();
	}
	for (int i = 0; i < contours.size(); i++) {
		cv::Moments m = cv::moments(contours[i]);							//gets a contour
		std::cout << "Sortet Contours Size: " << m.m00 << std::endl;
	}
	std::cout << "Contours" << std::endl;

	for (int i = contours.size() - 1; i >= 0; i--) {							//sees of the green dots are of relevance		(contours.size() cant be bigger than 2)
		cv::Moments m = cv::moments(contours[i]);							//gets a contour
		if (m.m00 < 1000) continue;											//removes dots smaller than 100
		g_dots.push_back(g_dot);											//adds a new green dot struct
		GreenDot *cur_d = &g_dots[g_dots.size() - 1];						//pointer to current GreenDot
		cur_d->contour = contours[i];										//-> notation dereferences a pointer to be able to have access to a pointed to objects methods
		double perim = cv::arcLength(contours[i], true);					//gets perimiter length of greendot
		std::cout << "Perim: " << perim << std::endl;
		cv::approxPolyDP(cur_d->contour, cur_d->corners, CONTOUR_APPROX_SCALE*perim, true);	//gets approximate contour (0.1 -> scale for approximate accuracy)	
		std::cout << i << " corners: " << cur_d->corners << std::endl;
		getRect(cur_d->corners);															//gets rect
		int x = 0, y = 0;																	//gets center
		for (int j = 0; j < cur_d->corners.size(); j++) {
			x += cur_d->corners[j].x;
			y += cur_d->corners[j].y;
		}
		cur_d->center = cv::Point(x / cur_d->corners.size(), y / cur_d->corners.size());
		cur_d->size = sqrt(pow(cur_d->corners[0].x - cur_d->corners[1].x, 2) + pow(cur_d->corners[0].y - cur_d->corners[1].y, 2)) * sqrt(pow(cur_d->corners[1].x - cur_d->corners[2].x, 2) + pow(cur_d->corners[1].y - cur_d->corners[2].y, 2));
		
		//---Swappíng Order to ensure Left being Left------------------------
		cv::Point2f  a = cv::Point2f(cur_d->corners[0].x - cur_d->center.x, cur_d->corners[0].y - cur_d->center.y);
		cv::Point2f  b = cv::Point2f(cur_d->corners[1].x - cur_d->center.x, cur_d->corners[1].y - cur_d->center.y);
		std::cout << vectAngle(a, b) << " A: " << a << " B: " << b << std::endl;
		if (vectAngle(a,b) < 0) {	//if next point is further left than current then swap order of vector
			std::reverse(cur_d->corners.begin(), cur_d->corners.end());
			std::cout << "Swapped Order" << std::endl;
		}

		//---Getting Normalized Perpendicular Vector for every side----------
		for (int j = 0; j < 4; j++) {
			cv::Point2f pa = cur_d->corners[(j + 1) % 4];
			cv::Point2f pb = cur_d->corners[j];
			cv::Point2f pc = cur_d->center;
			float dx = ((pa.x - pb.x) / 2) + pb.x - pc.x;
			float dy = ((pa.y - pb.y) / 2) + pb.y - pc.y;
			cur_d->perpVect[j] = cv::Point2f(dx / sqrt(pow(dx, 2) + pow(dy, 2)), dy / sqrt(pow(dx, 2) + pow(dy, 2)));			//gets a normalized vector from center outward perpendicular to sides of the square
			std::cout << "perpVector " << j << " : " << cur_d->perpVect[j] << std::endl;
			float size = 1.5*sqrt(pow((cur_d->corners[(j + 1) % 4].x - cur_d->corners[(j + 2) % 4].x), 2) + pow((cur_d->corners[(j + 1) % 4].y - cur_d->corners[(j + 2) % 4].y), 2));
			line(img_rgb, cur_d->center, cv::Point(cur_d->center.x + size * cur_d->perpVect[j].x, cur_d->center.y + size * cur_d->perpVect[j].y), cv::Scalar(0 + j * 50, 255 - j * 50, 0));
		}

		//---Getting Orientation of Dot-------------------------------------
		float angle = 180;
		for (int j = 0; j < 4; j++) {
			float a = acos((cur_d->perpVect[j].y*-1))* 180.0 / PI;			//gets angle between current vector and ideal vector (0, -1) --> since both are normalized and the x value is multiplied by 0 only the y value remains
			if (a < angle) {
				angle = a;
				cur_d->orientationIndex = j;
			}
			std::cout << "Angle " << j << " : " << a << std::endl;
		}
		std::cout << "Orientation " << cur_d->orientationIndex << " : " << angle << std::endl;

		//--Getting Relevance and Direction of Dot--------------------------
		float perc[] = { 0,0,0,0 };
		for (int j = 0; j < 4; j++) {
			cv::Point2f s = cv::Point((cur_d->corners[(j + 1) % 4].x - cur_d->corners[j].x) / 2 + cur_d->corners[j].x, (cur_d->corners[(j + 1) % 4].y - cur_d->corners[j].y) / 2 + cur_d->corners[j].y);
			cv::Point2f v = cur_d->perpVect[j];
			float h = sqrt(pow((cur_d->corners[(j + 1) % 4].x - cur_d->corners[(j + 2) % 4].x), 2) + pow((cur_d->corners[(j + 1) % 4].y - cur_d->corners[(j + 2) % 4].y), 2));
			int count = 0;
			for (int k = 0; k < h; k++) {
				cv::Point2f p = cv::Point(s.x + v.x*k, s.y + v.y*k);
				if (p.x < 0 || p.x >= img.cols || p.y < 0 || p.y >= img.rows)
					break;													//end for, since reached border
				if (img_roi.at<uchar>(cv::Point(floor(p.x), floor(p.y))) == 255) count++;
				cv::circle(img_rgb, p, 2, cv::Scalar((k * 20), (255 - (k * 20)), j == cur_d->orientationIndex? 200: j == (cur_d->orientationIndex+1)%4 ? 100: 0), 5);
			}
			perc[(j + 4 - cur_d->orientationIndex) % 4] = count / h;			//gets black percentages and sorts it depending on orientation -> 0 = top, 1 = right, 2 = bottom, 3 = left
		}
		if (perc[0] < .7 || perc[2] > 0.3) {								//if on top less than 70% or beneath more than 30% are black -> not of relevance
			contours.pop_back();
			g_dots.pop_back();
			std::cout << "Not Relevant" << std::endl;
			continue;
		}
		if (perc[1] > .7 || perc[3] < 0.3) {								//if right more than 70% or left less than 30% -> right
			cur_d->left = false;
		}
		else {
			cur_d->left = true;
		}
		for (int i = 0; i < 4; i++) {
			std::cout << "Perc: " << i << "  " << perc[i] << std::endl;
		}
		std::cout << "Left: " << cur_d->left << std::endl;
	}
	
	//---Compare Size Difference between multiple points----------------
	if (g_dots.size() > 1) {
		std::cout << "G_Dots Size: " << g_dots.size() << std::endl;
		if (g_dots[0].size > g_dots[1].size * 2) {								//remove second Dot, if it is smaller than half the size of first Dot
			contours.pop_back();
			g_dots.pop_back();
		}
	}
	for (int i = 0; i < g_dots.size(); i++) {
		std::cout << "Final Contours Size: " << g_dots[i].size << std::endl;
	}
	std::cout << "Size: " << g_dots.size() << std::endl;
	std::cout << "Done" << std::endl;
	return g_dots;
}

std::vector<std::vector<cv::Point>> get_green_dots(cv::Mat img, std::vector<cv::Point>& g_dots){
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
		if (count_b < sqrt(m.m00) * 2/3 || m.m00 > 1000) {			//if less than two thirds off all point checked aren't black, remove that point	or if size is smaller than 1000	[CONSTANT: 0.5]
			contours.erase(contours.begin() + i);
			g_dots.erase(g_dots.begin() + i);
			if (i == 0) i = -1;						//making sure that no dot is missed
		}
		if (contours.size() > 1 && i < contours.size()-1) {						//if there are two dots, see if they are similar size
			cv::Moments m1 = cv::moments(contours[i]);
			cv::Moments m2 = cv::moments(contours[i+1]);
			if (m2.m00 * 2 < m1.m00){					//if the second dot is less than half the size of the first, remove it			[CONSTANT: 2]
				contours.erase(contours.begin() + i);
				g_dots.erase(g_dots.begin() + i);
				if (i == 0) i = -1;						//making sure that no dot is missed
			}
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

void show(std::vector<std::vector<cv::Point>>& contours, std::vector<std::vector<cv::Point>>& g_contours, std::vector<GreenDot> g_dots, int index) {
	cv::Scalar S = cv::Scalar(0, 0, 255);
	if (index == -2) {					//if there is a 180° turn
		S = cv::Scalar(0, 131, 255);
	}
	cv::drawContours(img_rgb, contours, index, S, 2);
	//cv::drawContours(img_rgb, g_contours, -1, cv::Scalar(0, 255, 0), 2);
	for (int i = 0; i < g_dots.size(); i++) {
		cv::circle(img_rgb, g_dots[i].center, 2, cv::Scalar(0,255,0), 5);
		for (int j = 0; j < g_dots[i].corners.size(); j++)
		{
			line(img_rgb, g_dots[i].corners[j], g_dots[i].corners[(j + 1) % g_dots[i].corners.size()], cv::Scalar(0 + j * 50, 255 - j * 50, 0));
		}
	}
	windows();
}

void show() {
	windows();
}

void windows() {
	cv::imshow("IMG", img_rgb);
	cv::imshow("RED", img_red);
	cv::imshow("ROI", img_roi);
//	cv::imshow("GRE", img_gre);
	cv::waitKey(300000);
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

void getRect(std::vector<cv::Point>& c	) {		//returns rectangle for greendots
	std::vector<cv::Point2f> v;					//vectors going from corner to corner
	std::vector<float> angles, sidelengths;

	for (int i = 0; i < c.size(); i++) {		//calculates vectors between corners
		v.push_back(cv::Point2f(c[i].x - c[(i + 1) % c.size()].x, c[i].y - c[(i + 1) % c.size()].y));		//adds two points so that second can be replaced soon
	} 
	for (int i = 0; i < v.size(); i++) {
		angles.push_back(vectAngle(v[i],v[(i+1)%v.size()]));
		sidelengths.push_back(sqrt(pow(v[i].x, 2) + pow(v[i].y, 2)) + sqrt(pow(v[(i + 1) % v.size()].x, 2) + pow(v[(i + 1) % v.size()].y, 2)));				//sidelength sum of both sides leading to corner
		std::cout << "angle: " << angles[i] << " length: " << sidelengths[i] << " corners: " << c[i] << " vectors: " << v[i] << "  " << v[(i + 1) % v.size()] << std::endl;		//PRINT
	}											

	int index = 0;								//gets maximum sidelength
	float maxlength = 0;
	for (int i = 0; i < sidelengths.size(); i++) {
		if (sidelengths[i] > maxlength) {
			maxlength = sidelengths[i];
			index = i;
		}
	}
	int cornerindex = 0; 
	float maxval = 0;
	for (int i = 0; i < angles.size(); i++) {	//get the most suitable corner
		float a = abs(90 - angles[i]) == 0 ? 1 : 1 / abs(90 - angles[i]) * sidelengths[i] / maxlength;
		if (a > maxval) {
			maxval = a;
			cornerindex = i;
		}
	}
	cv::Point p = c[cornerindex];				//sets new points
	cv::Point2f vec[2] = {v[cornerindex],v[(cornerindex+1)%v.size()] };
	for (int i = c.size(); c.size() > 0; i--) {
		c.pop_back();
	}
	c.push_back(cv::Point(p.x,p.y));
	c.push_back(cv::Point(p.x - vec[0].x, p.y - vec[0].y));
	c.push_back(cv::Point(p.x - vec[0].x - vec[1].x, p.y - vec[0].y - vec[1].y));
	c.push_back(cv::Point(p.x - vec[1].x, p.y - vec[1].y));
	
	for (int i = 0; i < c.size(); i++) {
		std::cout << "New Corners: " << c[i] << std::endl;
	}
}


float dotP(cv::Point2f a, cv::Point2f b) {
	return a.x*b.x + a.y*b.y;
}

float vectSize(cv::Point2f a) {
	return sqrt(pow(a.x, 2) + pow(a.y, 2));
}

float vectAngle(cv::Point2f a, cv::Point2f b) {
	return acos(dotP(a, b) / (vectSize(a)*vectSize(b))) * 180.0 / PI;
}
