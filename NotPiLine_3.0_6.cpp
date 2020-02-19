//============================================================================
// Name        : VideoShow.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "opencv2/opencv.hpp"



using namespace std;
using namespace cv;

int pic_rows;
int pic_cols;
const int ANZAHL = 5;

float ymin = 0;
float ymax = 0;
float xmin = 0;
float xmax = 0;

int index_arr[ANZAHL][ANZAHL];
int roisize = 0;

cv::Mat img_rgb;
cv::Mat img_gminb;		 //gruen minus blau
cv::Mat img_gminr;		 //gruen minus rot
cv::Mat img_unterschied; //unterschied zwichen den beiden oberen /\ ( gminb und gminr )
cv::Mat img_binunt; 		 //Binärisiert auf den Unterschied
cv::Mat img_binhell; 		 //Binärisiert auf die Helligkeit
cv::Mat img_binsw;
cv::Mat img_blur;
cv::Mat img_roi;
cv::Mat img_bin;
cv::Mat img_hsv;
cv::Mat img_sw;

std::vector<cv::Mat> splitmat;  //Ein Vektor von Matritzen für Rot, Gruen und Blau

cv::Mat ROIs[ANZAHL][ANZAHL]; //Vector of Matritzen
std::vector<std::vector<cv::Point> > contours_arr[ANZAHL][ANZAHL]; //für schwarz

cv::Point Dots[ANZAHL][ANZAHL];
cv::Point DotsReal[ANZAHL][ANZAHL];
bool Connections[ANZAHL][ANZAHL][4];  //[x][y]Point on grid 0123 Connections to  top left bottom right

void img_set();
void makeROIs();
void makeConturesandDots();
void lines();
void draw();
void show();

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
	cv::namedWindow("BLUR");

	while (1) {
		std::cout << "Hallo1 " << std::endl;
		cap.read(img_rgb);
		std::cout << "Hallo2 " << std::endl;
		if (img_rgb.empty()) {
			std::cout << "Fehler beim holen des Bildes" << std::endl;
			return -2;
		}

		std::cout << "Hallo3 " << std::endl;
		pic_rows = img_rgb.rows;
		pic_cols = img_rgb.cols;
		std::cout << "Rows, Cols" << pic_rows << pic_cols << std::endl;

		img_set();
		std::cout << "Hallo4 " << std::endl;
		makeROIs();
		std::cout << "Hallo5 " << std::endl;
		makeConturesandDots();
		std::cout << "Hallo6 " << std::endl;
		lines();
		std::cout << "Hallo7 " << std::endl;
		draw();
		std::cout << "Hallo8 " << std::endl;
		show();
		std::cout << "Hallo9 " << std::endl;
	}
	std::cout << "BYYYY " << std::endl;
	return 0;
}	//Oben wurde zuweit eingeschoben

void img_set() {

	//RGB -> Graustufenwandlung
	cv::cvtColor(img_rgb, img_hsv, cv::COLOR_BGR2HSV);

	//Binarisierung + Green Image
	cv::inRange(img_hsv, cv::Scalar(60, 100, 25), cv::Scalar(90, 255, 150), img_bin);
	cv::inRange(img_hsv, cv::Scalar(0, 0, 0), cv::Scalar(180, 255, 100), img_binhell); //letzet Wert ist Helligkeit

																					   //White Image
	cv::split(img_rgb, splitmat);
	img_gminb = cv::abs(splitmat[0] - splitmat[1]);		// gruen - blau
	img_gminr = cv::abs(splitmat[1] - splitmat[2]);		// gruen - rot
	img_unterschied = cv::abs(img_gminb - img_gminr);	// Unterschied ausrechenen; klein=schwarz oder weiß
	cv::inRange(img_unterschied, cv::Scalar(0), cv::Scalar(8), img_binunt);

	cv::bitwise_and(img_binunt, img_binhell, img_binsw);

	/*
	//Blured Image
	cv::Rect roi(0, pic_rows / 3, pic_cols, (pic_cols / 5) * 4); //Laufwerte (0, 240, 640, 240)
	img_roi = img_binsw(roi);
	cv::GaussianBlur(img_roi, img_blur, Size(5, 5), 0, 0);
	cv::inRange(img_blur, cv::Scalar(0, 0, 0), cv::Scalar(50, 45, 45),img_blur);
	cv::bitwise_not(img_blur, img_sw); //schwarz weiß tauschen
	*/
}

void makeROIs() {
	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;
	for (int x = 0; x < ANZAHL; x++) {
		xmin = (img_binsw.cols / (ANZAHL)) * x;
		xmax = (img_binsw.cols / (ANZAHL)) * (x + 1);
		for (int y = 0; y < ANZAHL; y++) {
			ymin = (img_binsw.rows / (ANZAHL)) * y;
			ymax = (img_binsw.rows / (ANZAHL)) * (y + 1);

			std::cout << "xmin " << xmin << std::endl;
			std::cout << "xmax " << xmax << std::endl;
			std::cout << "ymin " << ymin << std::endl;
			std::cout << "ymax " << ymax << std::endl;

			//			  X  ,  Y  ,   WIDTH  ,   HEIGHT
			cv::Rect roi(xmin, ymin, xmax - xmin, ymax - ymin);
			std::cout << "Hallo4.1 " << std::endl;
			ROIs[x][y] = img_binsw(roi);
			std::cout << "Hallo4.2 " << std::endl;
			cv::GaussianBlur(ROIs[x][y], ROIs[x][y], Size(5, 5), 0, 0);
			std::cout << "Hallo4.3 " << std::endl;
			cv::inRange(ROIs[x][y], cv::Scalar(0, 0, 0), cv::Scalar(50, 45, 45), ROIs[x][y]);
			std::cout << "Hallo4.4 " << std::endl;
			cv::bitwise_not(ROIs[x][y], ROIs[x][y]); //schwarz weiß tauschen
			std::cout << "Hallo4.5 " << std::endl;
			cv::imshow("BLUR", ROIs[x][y]);
		}
	}
}

void makeConturesandDots() {
	//-----all-contures----
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			std::cout << "Hallo5.1 " << std::endl;
			cv::findContours(ROIs[x][y], contours_arr[x][y], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		}
	}

	//-----sorting-of-biggest-contour-and-getting-the-dots----
	std::cout << "Hallo5.2 " << std::endl;
	//-----Zurücksetzen--------------
	for (int a = 0; a < ANZAHL; a++) {
		for (int b = 0; b < ANZAHL; b++) {
			index_arr[a][b] = 0;
			Dots[a][b].x = 0;
			Dots[a][b].y = 0;
		}
	}

	roisize = ROIs[0][0].rows*ROIs[0][0].cols; //größe eines Rois

	for (int ix = 0; ix < ANZAHL; ix++) {
		for (int iy = 0; iy < ANZAHL; iy++) {
			double grocont = 0;  //größte Kontur
			for (unsigned int i = 0; i<contours_arr[ix][iy].size(); i++) {
				cv::Moments m = cv::moments(contours_arr[ix][iy][i]);
				//cv::circle
				if (m.m00 > grocont) {
					grocont = m.m00;
					if (grocont > roisize / 10) { //wie viel des Felds soll die Kontur belegen, bevor sie gezählt wird
						std::cout << "Hallo5.3 " << grocont << std::endl;
						Dots[ix][iy].x = m.m10 / m.m00;
						Dots[ix][iy].y = m.m01 / m.m00;
					}
					index_arr[ix][iy] = i;
				}
			}
			std::cout << "index_arr : " << index_arr[ix][iy] << std::endl;
		}
	}
	std::cout << "Hallo3.4 " << std::endl;
}

void lines() {
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			DotsReal[x][y] = cv::Point(0, 0);
			for (int a = 0; a < 4; a++) {
				Connections[x][y][a] = false;
			}
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			if (Dots[x][y].x != 0 && Dots[x][y].y != 0) {
				DotsReal[x][y].x = Dots[x][y].x + (pic_cols / ANZAHL)*x;  //DotsReal hat die echten Standpunkte der Punkte
				DotsReal[x][y].y = Dots[x][y].y + (pic_rows / ANZAHL)*y;
				std::cout << x << y << "Dots.x " << DotsReal[x][y].x << std::endl;
				std::cout << x << y << "Dots.y " << DotsReal[x][y].y << std::endl;
			}
		}
	}
	for (int x = 0; x < ANZAHL; x++) {  //go through all and save neighbor field
		for (int y = 0; y < ANZAHL; y++) {
			if (DotsReal[x][y].x != 0 && DotsReal[x][y].y != 0) {
				if (y != 0) {
					if (DotsReal[x][y - 1] != cv::Point(0, 0)) {  //top
						Connections[x][y][0] = true;
					}
				}
				if (x < ANZAHL) {
					if (DotsReal[x + 1][y] != cv::Point(0, 0)) {  //right
						Connections[x][y][1] = true;
					}
				}
				if (y < ANZAHL) {
					if (DotsReal[x][y + 1] != cv::Point(0, 0)) {  //bottom
						Connections[x][y][2] = true;
					}
				}
				if (x != 0) {
					if (DotsReal[x - 1][y] != cv::Point(0, 0)) {  //left
						Connections[x][y][3] = true;
					}
				}
				std::cout << x << y << "Connections0 " << Connections[x][y][0] << std::endl;
				std::cout << x << y << "Connections1 " << Connections[x][y][1] << std::endl;
				std::cout << x << y << "Connections2 " << Connections[x][y][2] << std::endl;
				std::cout << x << y << "Connections3 " << Connections[x][y][3] << std::endl;
			}
		}
	}
}

void draw() {
	if (true) {
		int i = 0;
		for (int x = 0; x < ANZAHL; x++) {
			for (int y = 0; y < ANZAHL; y++) {
				if (Dots[x][y].x != 0 && Dots[x][y].y != 0) {
					cv::drawContours(img_rgb, contours_arr[x][y], index_arr[x][y], cv::Scalar(255 - (i * 10), 0, 0 + (i * 10)), 2, 8, cv::noArray(), INT_MAX, cv::Point((pic_cols / ANZAHL)*x, (pic_rows / ANZAHL)*y));
					cv::circle(img_rgb, Dots[x][y] + cv::Point((pic_cols / ANZAHL)*x, (pic_rows / ANZAHL)*y), 2, cv::Scalar(0, 255, 0), 5);
				}
				i++;
			}
		}
	}
}

void show() {
	//Übergebe Bilder an den Server
	cv::imshow("RGB", img_rgb);

	cv::waitKey(30000);
}
