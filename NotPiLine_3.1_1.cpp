﻿//============================================================================
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
cv::Mat img_binunt; 		 //Bin�risiert auf den Unterschied
cv::Mat img_binhell; 		 //Bin�risiert auf die Helligkeit
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

cv::Point Links[ANZAHL][ANZAHL][4][4];  //first two [][] for current x,y pos  next[] for which connection and last[] for wich connection on the deleted node
cv::Point point;  //temp point storage
int tempdirection1 = 0; //tempdirection
int tempdirection2 = 0; //tempdirection
int extx = 0;	// extra x value
int exty = 0;	// extra y value
cv::Point TempLinks[ANZAHL][ANZAHL][4][4];  //first two [][] for current x,y pos  next[] for which connection and last[] for wich connection on the deleted node

cv::Point temppoint = cv::Point(0, 0);
bool TempConnections[ANZAHL][ANZAHL][4];  //Connections that will get changed in repeat
cv::Point linesstart[ANZAHL*ANZAHL];
cv::Point linesend[ANZAHL*ANZAHL];
int amount = 0;
int linksamount = 0;

cv::Point greendots[2];
std::vector<std::vector<cv::Point> > contours_green[2]; //für grün ( 2 für die möglichen zwei grünen Punkte )
int index_green[2];

int linethickness = 0;
int extravalue = 10;

void img_set();
void makeROIs();
void makeConturesandDots();
void lines();
void linescheck();
void find_begining();
void repeat(int x, int y);
void find_greendot();
void draw();
void show();

int find_linethickness();
int get_av_width(int x, int y);
int get_av_height(int x, int y);

int main() {

	//Erstelle Objekt von Klasse cv::VideoCapture und �ffne das Video "line.avi"
	cv::VideoCapture cap("line2.avi");

	//Pr�fe ob das Video geöffnet werden konnte
	if (!cap.isOpened()) {
		std::cout << "Problem beim �ffnen der Datei" << std::endl;
		return -1;
	}
	//Setze Framerate und die Auflösung
	cap.set(cv::CAP_PROP_FPS, 30);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	//Füge Fenster zum Server hinzu
	cv::namedWindow("RGB");
	cv::namedWindow("BLUR");
	cv::namedWindow("GREEN");

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
		linethickness = find_linethickness();
		std::cout << "Hallo8 " << std::endl;
		linescheck();
		std::cout << "Hallo9 " << std::endl;
		find_begining();



		for (int x = 0; x < ANZAHL; x++) {
			for (int y = 0; y < ANZAHL; y++) {
				for (int i = 0; i < 4; i++) {
					Connections[x][y][i] = TempConnections[x][y][i];
				}
			}
		}

		for (int x = 0; x < ANZAHL; x++) {
			for (int y = 0; y < ANZAHL; y++) {
				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 4; j++) {
						Links[x][y][i][j] = TempLinks[x][y][i][j];
					}
				}
			}
		}
		std::cout << "LINKS" << std::endl;
		for (int y = 0; y < ANZAHL; y++) {  //6 spots per link  -> [0, 0]
			std::cout << "              " << Links[0][y][0][0] << "              " << "  " << "              " << Links[1][y][0][0] << "              " << "  " << "              " << Links[2][y][0][0] << "              " << "  " << "              " << Links[3][y][0][0] << "              " << "  " << "              " << Links[4][y][0][0] << "              " << "  " << std::endl;
			std::cout << "          " << Links[0][y][0][3] << " " << Connections[0][y][0] << Links[0][y][0][1] << "          " << "  " << "          " << Links[1][y][0][3] << " " << Connections[1][y][0] << Links[1][y][0][1] << "          " << "  " << "          " << Links[2][y][0][3] << " " << Connections[2][y][0] << Links[2][y][0][1] << "          " << "  " << "          " << Links[3][y][0][3] << " " << Connections[3][y][0] << Links[3][y][0][1] << "          " << "  " << "          " << Links[4][y][0][3] << " " << Connections[4][y][0] << Links[4][y][0][1] << "          " << "  " << std::endl;
			std::cout << "              " << Links[0][y][0][2] << "              " << "  " << "              " << Links[1][y][0][2] << "              " << "  " << "              " << Links[2][y][0][2] << "              " << "  " << "              " << Links[3][y][0][2] << "              " << "  " << "              " << Links[4][y][0][2] << "              " << "  " << std::endl;

			std::cout << "    " << Links[0][y][3][0] << "    " << "      " << "    " << Links[0][y][1][0] << "    " << "  " << "    " << Links[1][y][3][0] << "    " << "      " << "    " << Links[1][y][1][0] << "    " << "  " << "    " << Links[2][y][3][0] << "    " << "      " << "    " << Links[2][y][1][0] << "    " << "  " << "    " << Links[3][y][3][0] << "    " << "      " << "    " << Links[3][y][1][0] << "    " << "  " << "    " << Links[4][y][3][0] << "    " << "      " << "    " << Links[4][y][1][0] << "    " << "  " << std::endl;
			std::cout << Links[0][y][3][3] << " " << Connections[0][y][3] << Links[0][y][3][1] << "      " << Links[0][y][1][3] << " " << Connections[0][y][1] << Links[0][y][1][1] << "  " << Links[1][y][3][3] << " " << Connections[1][y][3] << Links[1][y][3][1] << "      " << Links[1][y][1][3] << " " << Connections[1][y][1] << Links[1][y][1][1] << "  " << Links[2][y][3][3] << " " << Connections[2][y][3] << Links[2][y][3][1] << "      " << Links[2][y][1][3] << " " << Connections[2][y][1] << Links[2][y][1][1] << "  " << Links[3][y][3][3] << " " << Connections[3][y][3] << Links[3][y][3][1] << "      " << Links[3][y][1][3] << " " << Connections[3][y][1] << Links[3][y][1][1] << "  " << Links[4][y][3][3] << " " << Connections[4][y][3] << Links[4][y][3][1] << "      " << Links[4][y][1][3] << " " << Connections[4][y][1] << Links[4][y][1][1] << "  " << std::endl;
			std::cout << "    " << Links[0][y][3][2] << "    " << "      " << "    " << Links[0][y][1][2] << "    " << "  " << "    " << Links[1][y][3][2] << "    " << "      " << "    " << Links[1][y][1][2] << "    " << "  " << "    " << Links[2][y][3][2] << "    " << "      " << "    " << Links[2][y][1][2] << "    " << "  " << "    " << Links[3][y][3][2] << "    " << "      " << "    " << Links[3][y][1][2] << "    " << "  " << "    " << Links[4][y][3][2] << "    " << "      " << "    " << Links[4][y][1][2] << "    " << "  " << std::endl;

			std::cout << "              " << Links[0][y][2][0] << "              " << "  " << "              " << Links[1][y][2][0] << "              " << "  " << "              " << Links[2][y][2][0] << "              " << "  " << "              " << Links[3][y][2][0] << "              " << "  " << "              " << Links[4][y][2][0] << "              " << "  " << std::endl;
			std::cout << "          " << Links[0][y][2][3] << " " << Connections[0][y][2] << Links[0][y][2][1] << "          " << "  " << "          " << Links[1][y][2][3] << " " << Connections[1][y][2] << Links[1][y][2][1] << "          " << "  " << "          " << Links[2][y][2][3] << " " << Connections[2][y][2] << Links[2][y][2][1] << "          " << "  " << "          " << Links[3][y][2][3] << " " << Connections[3][y][2] << Links[3][y][2][1] << "          " << "  " << "          " << Links[4][y][2][3] << " " << Connections[4][y][2] << Links[4][y][2][1] << "          " << "  " << std::endl;
			std::cout << "              " << Links[0][y][2][2] << "              " << "  " << "              " << Links[1][y][2][2] << "              " << "  " << "              " << Links[2][y][2][0] << "              " << "  " << "              " << Links[3][y][2][0] << "              " << "  " << "              " << Links[4][y][2][0] << "              " << "  " << std::endl;

			std::cout << "" << std::endl;
		}





		std::cout << "Hallo10 " << std::endl;
		find_greendot();
		std::cout << "Hallo11 " << std::endl;
		draw();
		std::cout << "Hallo12 " << std::endl;
		show();
		std::cout << "Hallo13 " << std::endl;
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
	cv::bitwise_not(img_blur, img_sw); //schwarz wei� tauschen
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
			//cv::imshow("BLUR", ROIs[x][y]);
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
					if (grocont > (roisize * 0.1)) { //wie viel des Felds soll die Kontur belegen, bevor sie gez�hlt wird
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
				if (x < ANZAHL - 1) {		//ANZAHL - 1, da der Array von 0 - 4 geht und ich wil, dass er schon bei 4 dieses nicht macht
					if (DotsReal[x + 1][y] != cv::Point(0, 0)) {  //right
						Connections[x][y][1] = true;
					}
				}
				if (y < ANZAHL - 1) {
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

	std::cout << "   0   1   2   3   4 " << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		std::cout << "   " << Connections[0][i][0] << "   " << Connections[1][i][0] << "   " << Connections[2][i][0] << "   " << Connections[3][i][0] << "   " << Connections[4][i][0] << std::endl;
		std::cout << i << " " << Connections[0][i][3] << " " << Connections[0][i][1] << " " << Connections[1][i][3] << " " << Connections[1][i][1] << " " << Connections[2][i][3] << " " << Connections[2][i][1] << " " << Connections[3][i][3] << " " << Connections[3][i][1] << " " << Connections[4][i][3] << " " << Connections[4][i][1] << std::endl;
		std::cout << "   " << Connections[0][i][2] << "   " << Connections[1][i][2] << "   " << Connections[2][i][2] << "   " << Connections[3][i][2] << "   " << Connections[4][i][2] << std::endl;

	}
}

void linescheck() {
	point = cv::Point(0, 0);
	tempdirection1 = 0;
	tempdirection2 = 0;
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					Links[x][y][i][j] = cv::Point(5, 5);  // 0,0 sind ja wichtige koordinaten -> nicht ironisch gemeint
				}
			}
		}
	}

	if (true) {
		bool otherbigger = false;
		bool morelinks = false;
		int a = 0;
		int b = 0;
		for (int y = ANZAHL - 1; y >= 0; y--) {
			for (int x = ANZAHL - 1; x >= 0; x--) {
				if (DotsReal[x][y] != cv::Point(0, 0)) {
					std::cout << "Hallo8.1 " << x << "  " << y << std::endl;
					for (int i = 0; i < 4; i++) {
						otherbigger = false;
						morelinks = false;
						if (Connections[x][y][i] == true) {
							std::cout << "Hallo8.2 " << std::endl;
							if (i == 0 || i == 2) {  // top || bottom
								extx = 0;
								if (i == 0) {
									exty = -1;	//für top -> x + 0, y - 1
								}
								if (i == 2) {
									exty = 1;	 //für bottom -> x + 0, y + 1
								}
								a = get_av_height(x, y);
								b = get_av_height(x + extx, y + exty);
							}
							if (i == 1 || i == 3) {  // right || left
								exty = 0;
								if (i == 1) {
									extx = 1;	//für right -> x + 1, y + 0
								}
								if (i == 3) {
									extx = -1;	 //für bottom -> x - 1, y + 0
								}
								a = get_av_width(x, y);
								b = get_av_width(x + extx, y + exty);
							}
							if (DotsReal[x + extx][y + exty] != cv::Point(0, 0)) {
								if (a + b < linethickness + extravalue) {
									int avx = (DotsReal[x][y].x + DotsReal[x + extx][y + exty].x) / 2;
									int avy = (DotsReal[x][y].y + DotsReal[x + extx][y + exty].y) / 2;
									if (a < b) {	//other bigger than x,y
										x += extx;  //making the bigger one the current one
										y += exty;
										extx *= -1;
										exty *= -1;
										otherbigger = true;
										//goto icount;
										std::cout << " x " << x << " y " << y  << std::endl;
										std::cout << " extx " << extx << " exty " << exty << std::endl;
									}
									DotsReal[x][y] = cv::Point(avx, avy);
									DotsReal[x + extx][y + exty] = cv::Point(0, 0);
							
									switch (i) {
									case 0: tempdirection2 = 2; break;  //direction from missing node to current node
									case 1: tempdirection2 = 3; break;
									case 2: tempdirection2 = 0; break;
									case 3: tempdirection2 = 1; break;
									default: break;
									}
									
									if(otherbigger == true) {
										i = tempdirection2;
										switch (i) {
										case 0: tempdirection2 = 2; break;  //direction from missing node to current node
										case 1: tempdirection2 = 3; break;
										case 2: tempdirection2 = 0; break;
										case 3: tempdirection2 = 1; break;
										default: break;
										}
										std::cout << "else" << std::endl;
									}
									for (int j = 0; j < 4; j++) {
										if (Connections[x + extx][y + exty][j] == true && j != tempdirection2) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x + extx, y + exty - 1);
												tempdirection1 = 2;  //direction from linked-to-node to missing node
												morelinks = true;
											}
											if (j == 1) {  //right
												point = cv::Point(x + extx + 1, y + exty);
												tempdirection1 = 3;
												morelinks = true;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x + extx, y + exty + 1);
												tempdirection1 = 0;
												morelinks = true;
											}
											if (j == 3) {  //left
												point = cv::Point(x + extx - 1, y + exty);
												tempdirection1 = 1;
												morelinks = true;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);

											std::cout << " x " << x << " y " << y << " i " << i << " j " << j << std::endl;
											std::cout << "LINKS" << Links[x][y][i][j] << std::endl;
											std::cout << " point.x " << point.x << " point.y " << point.y << " tempdirection1 " << tempdirection1 << " tempdirection2 " << tempdirection2 << std::endl;
											std::cout << "LINKS" << Links[point.x][point.y][tempdirection1][tempdirection2] << std::endl;
										}
										Connections[x + extx][y + exty][j] = false;
									}
									if (!morelinks) {
										Connections[x][y][i] = false;  // no linking to oneself  (-> REWORK!!! (if only link is to oneself))
									}
								}
							}
						}
					}
				}
			}
		}
	}
	std::cout << "LINKS" << Links[2][2][1][1] << std::endl;
	for (int a = 0; a < ANZAHL; a++) {
		for (int b = 0; b < ANZAHL; b++) {
			if (DotsReal[a][b] == cv::Point(0, 0)) {
				Dots[a][b] = cv::Point(0, 0);
			}
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				TempConnections[x][y][i] = Connections[x][y][i];
			}
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					TempLinks[x][y][i][j] = Links[x][y][i][j];
				}
			}
		}
	}

	std::cout << "   0   1   2   3   4 " << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		std::cout << "   " << Connections[0][i][0] << "   " << Connections[1][i][0] << "   " << Connections[2][i][0] << "   " << Connections[3][i][0] << "   " << Connections[4][i][0] << std::endl;
		std::cout << i << " " << Connections[0][i][3] << " " << Connections[0][i][1] << " " << Connections[1][i][3] << " " << Connections[1][i][1] << " " << Connections[2][i][3] << " " << Connections[2][i][1] << " " << Connections[3][i][3] << " " << Connections[3][i][1] << " " << Connections[4][i][3] << " " << Connections[4][i][1] << std::endl;
		std::cout << "   " << Connections[0][i][2] << "   " << Connections[1][i][2] << "   " << Connections[2][i][2] << "   " << Connections[3][i][2] << "   " << Connections[4][i][2] << std::endl;

	}
	std::cout << "LINKS" << std::endl;
	for (int y = 0; y < ANZAHL; y++) {  //6 spots per link  -> [0, 0]
		std::cout << "              " << Links[0][y][0][0] << "              "                                                                                                   << "  " << "              " << Links[1][y][0][0] << "              "                                                                                                   << "  " << "              " << Links[2][y][0][0] << "              "                                                                                                   << "  " << "              " << Links[3][y][0][0] << "              "                                                                                                   << "  " << "              " << Links[4][y][0][0] << "              "                                                                                                   << "  " << std::endl;
		std::cout << "          " << Links[0][y][0][3] << " " << Connections[0][y][0] << Links[0][y][0][1] << "          "                                                       << "  " << "          " << Links[1][y][0][3] << " " << Connections[1][y][0] << Links[1][y][0][1] << "          "                                                       << "  " << "          " << Links[2][y][0][3] << " " << Connections[2][y][0] << Links[2][y][0][1] << "          "                                                       << "  " << "          " << Links[3][y][0][3] << " " << Connections[3][y][0] << Links[3][y][0][1] << "          "                                                       << "  " << "          " << Links[4][y][0][3] << " " << Connections[4][y][0] << Links[4][y][0][1] << "          "                                                       << "  " << std::endl;
		std::cout << "              " << Links[0][y][0][2] << "              "                                                                                                   << "  " << "              " << Links[1][y][0][2] << "              "                                                                                                   << "  " << "              " << Links[2][y][0][2] << "              "                                                                                                   << "  " << "              " << Links[3][y][0][2] << "              "                                                                                                   << "  " << "              " << Links[4][y][0][2] << "              "                                                                                                   << "  " << std::endl;
																																																																																																																																																																																																																											
		std::cout << "    " << Links[0][y][3][0] << "    " << "      " << "    " << Links[0][y][1][0] << "    "                                                                  << "  " << "    " << Links[1][y][3][0] << "    " << "      " << "    " << Links[1][y][1][0] << "    "                                                                  << "  " << "    " << Links[2][y][3][0] << "    " << "      " << "    " << Links[2][y][1][0] << "    "                                                                  << "  " << "    " << Links[3][y][3][0] << "    " << "      " << "    " << Links[3][y][1][0] << "    "                                                                  << "  " << "    " << Links[4][y][3][0] << "    " << "      " << "    " << Links[4][y][1][0] << "    "                                                                  << "  " << std::endl;
		std::cout << Links[0][y][3][3] << " " << Connections[0][y][3] << Links[0][y][3][1] << "      " << Links[0][y][1][3] << " " << Connections[0][y][1] << Links[0][y][1][1]  << "  " << Links[1][y][3][3] << " " << Connections[1][y][3] << Links[1][y][3][1] << "      " << Links[1][y][1][3] << " " << Connections[1][y][1] << Links[1][y][1][1]  << "  " << Links[2][y][3][3] << " " << Connections[2][y][3] << Links[2][y][3][1] << "      " << Links[2][y][1][3] << " " << Connections[2][y][1] << Links[2][y][1][1]  << "  " << Links[3][y][3][3] << " " << Connections[3][y][3] << Links[3][y][3][1] << "      " << Links[3][y][1][3] << " " << Connections[3][y][1] << Links[3][y][1][1]  << "  " << Links[4][y][3][3] << " " << Connections[4][y][3] << Links[4][y][3][1] << "      " << Links[4][y][1][3] << " " << Connections[4][y][1] << Links[4][y][1][1]  << "  " << std::endl;
		std::cout << "    " << Links[0][y][3][2] << "    " << "      " << "    " << Links[0][y][1][2] << "    "                                                                  << "  " << "    " << Links[1][y][3][2] << "    " << "      " << "    " << Links[1][y][1][2] << "    "                                                                  << "  " << "    " << Links[2][y][3][2] << "    " << "      " << "    " << Links[2][y][1][2] << "    "                                                                  << "  " << "    " << Links[3][y][3][2] << "    " << "      " << "    " << Links[3][y][1][2] << "    "                                                                  << "  " << "    " << Links[4][y][3][2] << "    " << "      " << "    " << Links[4][y][1][2] << "    "                                                                  << "  " << std::endl;
																																																																																																																																																																																																																																				
		std::cout << "              " << Links[0][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[1][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[2][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[3][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[4][y][2][0] << "              "                                                                                                   << "  " << std::endl;
		std::cout << "          " << Links[0][y][2][3] << " " << Connections[0][y][2] << Links[0][y][2][1] << "          "                                                       << "  " << "          " << Links[1][y][2][3] << " " << Connections[1][y][2] << Links[1][y][2][1] << "          "                                                       << "  " << "          " << Links[2][y][2][3] << " " << Connections[2][y][2] << Links[2][y][2][1] << "          "                                                       << "  " << "          " << Links[3][y][2][3] << " " << Connections[3][y][2] << Links[3][y][2][1] << "          "                                                       << "  " << "          " << Links[4][y][2][3] << " " << Connections[4][y][2] << Links[4][y][2][1] << "          "                                                       << "  " << std::endl;
		std::cout << "              " << Links[0][y][2][2] << "              "                                                                                                   << "  " << "              " << Links[1][y][2][2] << "              "                                                                                                   << "  " << "              " << Links[2][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[3][y][2][0] << "              "                                                                                                   << "  " << "              " << Links[4][y][2][0] << "              "                                                                                                   << "  " << std::endl;

		std::cout << "" << std::endl;
	}
	std::cout << "LINKS" << Links[2][2][1][1] << std::endl;

}
/*
void linescheck() {
	point = cv::Point(0, 0);
	tempdirection1 = 0;
	tempdirection2 = 0;
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					Links[x][y][i][j] = cv::Point(5,5);  // 0,0 sind ja wichtige koordinaten -> nicht ironisch gemeint
				}
			}
		}
	}
	std::cout << "links[1][1][1][1]" << Links[1][1][1][1] << std::endl;

	for (int y = ANZAHL - 1; y >= 0; y--) {
		for (int x = ANZAHL - 1; x >= 0; x--) {
			if (DotsReal[x][y] != cv::Point(0, 0)) {
				std::cout << "Hallo8.1 " << x << "  " << y << std::endl;
				for (int i = 0; i < 4; i++) {
					if (Connections[x][y][i] == true) {
						std::cout << "Hallo8.2 " << std::endl;
						if (i == 0) {  //top
							int a = get_av_height(x, y);
							int b = get_av_height(x, y - 1);
							std::cout << "Hallo8.3.1 " << a << "  " << b << std::endl;
							if (a + b < linethickness + extravalue) {
								int avx = (DotsReal[x][y].x + DotsReal[x][y - 1].x) / 2;
								int avy = (DotsReal[x][y].y + DotsReal[x][y - 1].y) / 2;
								if (a > b) {  // bottom bigger than top
									std::cout << "Hallo8.4.1.1 " << std::endl;
									DotsReal[x][y]		 = cv::Point(avx, avy);
									DotsReal[x][y - 1]	 = cv::Point(0, 0);
									tempdirection2 = 2;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y - 1][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x, y-2);
												tempdirection1 = 2;  //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y - 1);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y - 1);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y - 1][j] = false;
									}
								}
								else {  //top bigger than bottom
									std::cout << "Hallo8.4.1.2 " << std::endl;
									DotsReal[x][y - 1] = cv::Point(avx, avy);
									DotsReal[x][y] = cv::Point(0, 0);
									tempdirection2 = 0;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x,y);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y - 1);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y-2);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y - 1);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y][j] = false;
									}
								}
							}
						}
						if (i == 1) {  //right
							int a = get_av_width(x, y);
							int b = get_av_width(x + 1, y);
							std::cout << "Hallo8.3.2 " << a << "  " << b << std::endl;
							if (a + b < linethickness + extravalue) {
								int avx = (DotsReal[x][y].x + DotsReal[x + 1][y].x) / 2;
								int avy = (DotsReal[x][y].y + DotsReal[x + 1][y].y) / 2;
								if (a > b) {  //left bigger than right
									std::cout << "Hallo8.4.2.1 " << std::endl;
									DotsReal[x][y] = cv::Point(avx, avy);
									DotsReal[x + 1][y] = cv::Point(0, 0);
									tempdirection2 = 3;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x + 1][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x + 1, y - 1);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x +  2, y);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x + 1, y + 1);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x , y);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x + 1][y][j] = false;
									}
								}
								else {  //right bigger than left
									std::cout << "Hallo8.4.2.2 " << std::endl;
									DotsReal[x + 1][y] = cv::Point(avx, avy);
									DotsReal[x][y] = cv::Point(0, 0);
									tempdirection2 = 1;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x, y - 1);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y + 1);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y][j] = false;
									}
								}
							}
						}
						if (i == 2) {  //bottom
							int a = get_av_width(x, y);
							int b = get_av_width(x, y + 1);
							std::cout << "Hallo8.3.3 " << a << "  " << b << std::endl;
							if (a + b < linethickness + extravalue) {
								int avx = (DotsReal[x][y].x + DotsReal[x][y + 1].x) / 2;
								int avy = (DotsReal[x][y].y + DotsReal[x][y + 1].y) / 2;
								if (a > b) {  //top bigger than bottom
									std::cout << "Hallo8.4.3.1 " << std::endl;
									DotsReal[x][y] = cv::Point(avx, avy);
									DotsReal[x][y + 1] = cv::Point(0, 0);
									tempdirection2 = 0;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y + 1][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x, y);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y + 1);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y + 2);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y - 1);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y + 1][j] = false;
									}
								}
								else {  //bottom bigger than top
									std::cout << "Hallo8.4.3.2 " << std::endl;
									DotsReal[x][y + 1] = cv::Point(avx, avy);
									DotsReal[x][y] = cv::Point(0, 0);
									tempdirection2 = 2;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x, y - 1);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y + 1);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y][j] = false;
									}
								}
							}
						}
						if (i == 3) {  //left
							int a = get_av_width(x, y);
							int b = get_av_width(x - 1, y);
							std::cout << "Hallo8.3.4 " << a << "  " << b << std::endl;
							if (a + b < linethickness + extravalue) {
								int avx = (DotsReal[x][y].x + DotsReal[x - 1][y].x) / 2;
								int avy = (DotsReal[x][y].y + DotsReal[x - 1][y].y) / 2;
								if (a > b) {  //right bigger than left
									std::cout << "Hallo8.4.4.1 " << std::endl;
									DotsReal[x][y] = cv::Point(avx, avy);
									DotsReal[x - 1][y] = cv::Point(0, 0);
									tempdirection2 = 1;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x - 1, y - 1);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x , y);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x - 1, y + 1);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 2, y);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x - 1][y][j] = false;
									}
								}
								else {  //left bigger than right
									std::cout << "Hallo8.4.4.2 " << std::endl;
									DotsReal[x - 1][y] = cv::Point(avx, avy);
									DotsReal[x][y] = cv::Point(0, 0);
									tempdirection2 = 3;  //direction from missing node to current node
									for (int j = 0; j < 4; j++) {
										if (Connections[x][y][j] == true) {
											point = cv::Point(0, 0);
											if (j == 0) {  //top
												point = cv::Point(x, y - 1);
												tempdirection1 = 2;	 //direction from linked-to-node to missing node
											}
											if (j == 1) {  //right
												point = cv::Point(x + 1, y);
												tempdirection1 = 3;
											}
											if (j == 2) {  //bottom
												point = cv::Point(x, y + 1);
												tempdirection1 = 0;
											}
											if (j == 3) {  //left
												point = cv::Point(x - 1, y);
												tempdirection1 = 1;
											}
											Links[x][y][i][j] = point;
											Links[point.x][point.y][tempdirection1][tempdirection2] = cv::Point(x, y);
										}
										Connections[x][y][j] = false;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	for (int a = 0; a < ANZAHL; a++) {
		for (int b = 0; b < ANZAHL; b++) {
			if (DotsReal[a][b] == cv::Point(0, 0)) {
				Dots[a][b] = cv::Point(0, 0);
			}
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				TempConnections[x][y][i] = Connections[x][y][i];
			}
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					TempLinks[x][y][i][j] = Links[x][y][i][j];
				}
			}
		}
	}

	std::cout << "   0   1   2   3   4 " << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		std::cout << "   " << Connections[0][i][0] << "   " << Connections[1][i][0] << "   " << Connections[2][i][0] << "   " << Connections[3][i][0] << "   " << Connections[4][i][0] << std::endl;
		std::cout << i << " " << Connections[0][i][3] << " " << Connections[0][i][1] << " " << Connections[1][i][3] << " " << Connections[1][i][1] << " " << Connections[2][i][3] << " " << Connections[2][i][1] << " " << Connections[3][i][3] << " " << Connections[3][i][1] << " " << Connections[4][i][3] << " " << Connections[4][i][1] << std::endl;
		std::cout << "   " << Connections[0][i][2] << "   " << Connections[1][i][2] << "   " << Connections[2][i][2] << "   " << Connections[3][i][2] << "   " << Connections[4][i][2] << std::endl;

	}
	std::cout << "LINKS" << std:: endl;
	for (int y = 0; y < ANZAHL; y++) {  //6 spots per link  -> [0, 0]
		std::cout << "              " << Links[0][y][0][0] << "              "																									<< "  " << "              " << Links[1][y][0][0] << "              "																								  << "  " << "              " << Links[2][y][0][0] << "              "																									<< "  " << "              " << Links[3][y][0][0] << "              "																								  << "  " << "              " << Links[4][y][0][0] << "              "																									<< "  " << std::endl;
		std::cout << "          " << Links[0][y][0][3] << " " << Connections[0][y][0] << Links[0][y][3][1] << "          "														<< "  " << "          " << Links[1][y][0][3] << " " << Connections[1][y][0] << Links[1][y][3][1] << "          "													  << "  " << "          " << Links[2][y][0][3] << " " << Connections[2][y][0] << Links[2][y][3][1] << "          "														<< "  " << "          " << Links[3][y][0][3] << " " << Connections[3][y][0] << Links[3][y][3][1] << "          "													  << "  " << "          " << Links[4][y][0][3] << " " << Connections[4][y][0] << Links[4][y][3][1] << "          "														<< "  " << std::endl;
		std::cout << "              " << Links[0][y][0][2] << "              "																									<< "  " << "              " << Links[1][y][0][2] << "              "																								  << "  " << "              " << Links[2][y][0][2] << "              "																									<< "  " << "              " << Links[3][y][0][2] << "              "																								  << "  " << "              " << Links[4][y][0][2] << "              "																									<< "  " << std::endl;
																																																																																																																																																																																																																										
		std::cout << "    " << Links[0][y][3][0] << "    " << "      " << "    " << Links[0][y][1][0] << "    "																	<< "  " << "    " << Links[1][y][3][0] << "    " << "      " << "    " << Links[1][y][1][0] << "    "																  << "  " << "    " << Links[2][y][3][0] << "    " << "      " << "    " << Links[2][y][1][0] << "    "																	<< "  " << "    " << Links[3][y][3][0] << "    " << "      " <<	"    " << Links[3][y][1][0] << "    "																  << "  " << "    " << Links[4][y][3][0] << "    " << "      " << "    " << Links[4][y][1][0] << "    "																	<< "  " << std::endl;
		std::cout << Links[0][y][3][3] << " " << Connections[0][y][3] << Links[0][y][3][1] << "      " << Links[0][y][2][3] << " " << Connections[0][y][1] << Links[0][y][2][1]	<< "  " << Links[1][y][3][3] << " " << Connections[1][y][3] << Links[1][y][3][1] << "      " << Links[1][y][2][3] << " " << Connections[1][y][1] << Links[1][y][2][1] << "  " << Links[2][y][3][3] << " " << Connections[2][y][3] << Links[2][y][3][1] << "      " << Links[2][y][2][3] << " " << Connections[2][y][1] << Links[2][y][2][1] << "  " << Links[3][y][3][3] << " " << Connections[3][y][3] << Links[3][y][3][1] << "      " << Links[3][y][2][3] << " " << Connections[3][y][1] << Links[3][y][2][1] << "  " << Links[4][y][3][3] << " " << Connections[4][y][3] << Links[4][y][3][1] << "      " << Links[4][y][2][3] << " " << Connections[4][y][1] << Links[4][y][2][1] << "  " << std::endl;
		std::cout << "    " << Links[0][y][3][2] << "    " << "      " << "    " << Links[0][y][1][2] << "    "																	<< "  " << "    " << Links[1][y][3][2] << "    " << "      " << "    " << Links[1][y][1][2] << "    "																  << "  " << "    " << Links[2][y][3][2] << "    " << "      " << "    " << Links[2][y][1][2] << "    "																	<< "  " << "    " << Links[3][y][3][2] << "    " << "      " << "    " << Links[3][y][1][2] << "    "																  << "  " << "    " << Links[4][y][3][2] << "    " << "      " << "    " << Links[4][y][1][2] << "    "																	<< "  " << std::endl;
																																																																																																																																																																																																																								
		std::cout << "              " << Links[0][y][2][0] << "              "																									<< "  " << "              " << Links[1][y][2][0] << "              "																								  << "  " << "              " << Links[2][y][2][0] << "              "																									<< "  " << "              " << Links[3][y][2][0] << "              "																								  << "  " << "              " << Links[4][y][2][0] << "              "																									<< "  " << std::endl;
		std::cout << "          " << Links[0][y][2][3] << " " << Connections[0][y][2] << Links[0][y][3][1] << "          "														<< "  " << "          " << Links[1][y][2][3] << " " << Connections[1][y][2] << Links[1][y][3][1] << "          "													  << "  " << "          " << Links[2][y][2][3] << " " << Connections[2][y][2] << Links[2][y][3][1] << "          "														<< "  " << "          " << Links[3][y][2][3] << " " << Connections[3][y][2] << Links[3][y][3][1] << "          "													  << "  " << "          " << Links[4][y][2][3] << " " << Connections[4][y][2] << Links[4][y][3][1] << "          "														<< "  " << std::endl;
		std::cout << "              " << Links[0][y][2][2] << "              "																									<< "  " << "              " << Links[1][y][2][2] << "              "																								  << "  " << "              " << Links[2][y][2][2] << "              "																									<< "  " << "              " << Links[3][y][2][2] << "              "																								  << "  " << "              " << Links[4][y][2][2] << "              "																									<< "  " << std::endl;
																																																																																																																																																																																																																					
		std::cout << "" << std::endl;
	}
}*/

void find_begining() {
	amount = 0;
	for (int i = 0; i < ANZAHL*ANZAHL; i++) {
		linesstart[i] = cv::Point(0, 0);
		linesend[i] = cv::Point(0, 0);
	}

	for (int y = ANZAHL - 1; y >= 0; y--) {
		for (int x = ANZAHL - 1; x >= 0; x--) {
			if (DotsReal[x][y].x != 0 && DotsReal[x][y].y != 0) {
				std::cout << "Hallo7.1 " << x << " " << y << std::endl;
				std::cout << "Hallo7.1 dotsReal " << DotsReal[x][y].x << " " << DotsReal[x][y].y << std::endl;
				repeat(x, y);
				for (int a = 0; a < amount; a++) {
					std::cout << "Hallo7.1 lines " << linesstart[a].x << " " << linesstart[a].y << " , " << linesend[a].x << " " << linesend[a].y << std::endl;
				}
				return;
			}
			std::cout << "Hallo7.1.5 " << x << " " << y << std::endl;
		}
	}
}
/*
void repeat(int x, int y) {
		for (int i = 0; i < 4; i++) {
			std::cout << "Hallo7.2 " << amount << " " << i << " , " << x << " " << y << " " << std::endl;
			if (TempConnections[x][y][i] == true) {
				if (i == 0) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x][y - 1].x), 2) + pow((DotsReal[x][y].y - DotsReal[x][y - 1].y), 2)) > linethickness) {  // if the distance (Pythagorean Therom) between the two points is bigger than the linethickness 
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y - 1];
					TempConnections[x][y - 1][2] = false;
					amount++;
					repeat(x, y - 1);
					//}
				}
				if (i == 1) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x + 1][y].x), 2) + pow((DotsReal[x][y].y - DotsReal[x + 1][y].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x + 1][y];
					TempConnections[x + 1][y][3] = false;
					amount++;
					repeat(x + 1, y);
					//}
				}
				if (i == 2) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x][y + 1].x), 2) + pow((DotsReal[x][y].y - DotsReal[x][y + 1].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y + 1];
					TempConnections[x][y + 1][0] = false;
					amount++;
					repeat(x, y + 1);
					//}
				}
				if (i == 3) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x - 1][y].x), 2) + pow((DotsReal[x][y].y - DotsReal[x - 1][y].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x - 1][y];
					TempConnections[x - 1][y][1] = false;
					amount++;
					repeat(x - 1, y);
					//}
				}
			}
	}
}*/

void repeat(int x, int y) {
	tempdirection1 = 0;  //direction from linked-to-node to missing node
	tempdirection2 = 0;  //direction from missing node to current node
	temppoint = cv::Point(0, 0);
	linksamount = 0;
	for (int i = 0; i < 4; i++) {
		if (TempConnections[x][y][i] == true) {
			for (int j = 0; j < 4; j++) {
				if (TempLinks[x][y][i][j] != cv::Point(5, 5)) {   //if there is linking  do this
					switch (i)
					{
					case 0: tempdirection2 = 2; break;
					case 1: tempdirection2 = 3; break;
					case 2: tempdirection2 = 0; break;
					case 3: tempdirection2 = 1; break;
					default: break;
					}
					switch (j)
					{
					case 0: tempdirection1 = 2; break;
					case 1: tempdirection1 = 3; break;
					case 2: tempdirection1 = 0; break;
					case 3: tempdirection1 = 1; break;
					default: break;
					}
					/*if (i == 0) {
						tempdirection2 = 2;
					}
					if (i == 1) {
						tempdirection2 = 3;
					}
					if (i == 2) {
						tempdirection2 = 0;
					}
					if (i == 3) {
						tempdirection2 = 1;
					}
					if (j == 0) {
						tempdirection1 = 2; 
					}
					if (j == 1) {
						tempdirection1 = 3;
					}
					if (j == 2) {
						tempdirection1 = 0;
					}
					if (j == 3) {
						tempdirection1 = 1;
					}*/
					std::cout <<"tempdirection1 " << tempdirection1 << "   tempdirection2 " << tempdirection2 << std::endl;
					temppoint = TempLinks[x][y][i][j];
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[TempLinks[x][y][i][j].x][TempLinks[x][y][i][j].y];
					TempConnections[x][y][i] = false;
					TempConnections[TempLinks[x][y][i][j].x][TempLinks[x][y][i][j].y][tempdirection1] = false;
					TempLinks[TempLinks[x][y][i][j].x][TempLinks[x][y][i][j].y][tempdirection1][tempdirection2] = cv::Point(5, 5);	//reset both points
					TempLinks[x][y][i][j] = cv::Point(5, 5);																		//reset both points
					amount++;
					repeat(temppoint.x, temppoint.y);
				}
			}
			for (int j = 0; j < 4; j++) {
				if (Links[x][y][i][j] != cv::Point(5, 5)) {
					linksamount++;
				}
			}
			if (linksamount == 0) {  // if there is no linking  do the usual repeat
				if (i == 0) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x][y - 1].x), 2) + pow((DotsReal[x][y].y - DotsReal[x][y - 1].y), 2)) > linethickness) {  // if the distance (Pythagorean Therom) between the two points is bigger than the linethickness 
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y - 1];
					TempConnections[x][y - 1][2] = false;
					amount++;
					repeat(x, y - 1);
					//}
				}
				if (i == 1) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x + 1][y].x), 2) + pow((DotsReal[x][y].y - DotsReal[x + 1][y].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x + 1][y];
					TempConnections[x + 1][y][3] = false;
					amount++;
					repeat(x + 1, y);
					//}
				}
				if (i == 2) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x][y + 1].x), 2) + pow((DotsReal[x][y].y - DotsReal[x][y + 1].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y + 1];
					TempConnections[x][y + 1][0] = false;
					amount++;
					repeat(x, y + 1);
					//}
				}
				if (i == 3) {
					//if (sqrt(pow((DotsReal[x][y].x - DotsReal[x - 1][y].x), 2) + pow((DotsReal[x][y].y - DotsReal[x - 1][y].y), 2)) > linethickness) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x - 1][y];
					TempConnections[x - 1][y][1] = false;
					amount++;
					repeat(x - 1, y);
					//}
				}
			}
		}
	}
}

void find_greendot() {
	//---Zurücksetzen----------
	for (int a = 0; a < 2; a++) {
		greendots[a] = cv::Point(0, 0);
		index_green[a] = -1;
	}
	//---find-contures---------
	for (int i = 0; i < 2; i++) {
		cv::findContours(img_bin, contours_green[i], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	}
	std::cout << "Hallo9.1 " << std::endl;
	//---größte-Konturen-(2)---
	for (int j = 0; j < 2; j++) {
		double grocont = 0;  //größte Kontur
		for (unsigned int i = 0; i < contours_green[j].size(); i++) {
			cv::Moments m = cv::moments(contours_green[j][i]);
			//cv::circle
			if (m.m00 > grocont && i != index_green[0]) {  // sodass es die größte ist, die aber nicht die erste ist ( 1. grüner Punkt )
				grocont = m.m00;
				std::cout << "Hallo9.2 " << grocont << std::endl;
				greendots[j].x = m.m10 / m.m00;
				greendots[j].y = m.m01 / m.m00;
				index_green[j] = i;
			}
		}
	}
	std::cout << "Hallo9.3 " << std::endl;
	//---Kontrolle-ob-Punkt----
	for (int i = 0; i < 2; i++) {  
		if (greendots[i] != cv::Point(0, 0)) {
			std::cout << "Hallo9.3.1 " << std::endl;
			int count_black = 0;
			cv::Moments m = cv::moments(contours_green[i][index_green[i]]);
			double height = sqrt(m.m00);
			std::cout << "Hallo9.3.2 " << m.m00 << "  " << height << std::endl;
			for (int iy = 0; iy < height; iy++) {
				int x = greendots[i].x;
				int y = greendots[i].y  - (height / 2) - iy;
				std::cout << "Hallo9.3.2.1 " << x << " " << y << std::endl;
				if (y >= 0) {
					if (img_binsw.at<uchar>(y, x) == 255) {
						count_black++;
					}
					cv::circle(img_rgb, cv::Point(x, y), 2, cv::Scalar((iy * 20), (255 - (iy * 20)), 0), 5);
				}
			}
			std::cout << "count_black " << count_black << "  height " << height << std::endl;
			if (count_black < (height * 0.50)) {
				std::cout << "to Small" << std::endl;
				greendots[i] = cv::Point(0, 0);
				index_green[i] = -1;
			}
		}
	}
}

void draw() {
	if (true) {
		int i = 0;
		for (int x = 0; x < ANZAHL; x++) {
			for (int y = 0; y < ANZAHL; y++) {
				if (DotsReal[x][y].x != 0 && DotsReal[x][y].y != 0) {
					cv::drawContours(img_rgb, contours_arr[x][y], index_arr[x][y], cv::Scalar(255 - (i * 10), 0, 0 + (i * 10)), 2, 8, cv::noArray(), INT_MAX, cv::Point((pic_cols / ANZAHL)*x, (pic_rows / ANZAHL)*y));
					cv::circle(img_rgb, DotsReal[x][y], 2, cv::Scalar(0, 255, 0), 5);
				}
				i++;
			}
		}
	}
	for (int a = 0; a < amount; a++) {
		std::cout << "Hallo10.1 " << a << std::endl;
		cv::line(img_rgb, linesstart[a], linesend[a], cv::Scalar(0, 0, 255), 3);
		std::cout << "Hallo10.2 " << a << std::endl;
	}
	for (int b = 0; b < 2; b++) {  // Green-Dot
		if (greendots[b] != cv::Point(0, 0)) {
			cv::drawContours(img_rgb, contours_green[b], index_green[b], cv::Scalar(0, 255, 0), 2, 8, cv::noArray(), INT_MAX);
			cv::circle(img_rgb, greendots[b], 2, cv::Scalar(30, 255, 30), 5);
		}
	}
}

void show() {
	//übergebe Bilder an den Server
	cv::imshow("RGB", img_rgb);
	cv::imshow("GREEN", img_bin);
	cv::waitKey(30000000);
}


int find_linethickness() {
	for (int y = ANZAHL - 1; y >= 0; y--) {
		for (int x = ANZAHL - 1; x >= 0; x--) {
			if (DotsReal[x][y].x != 0 && DotsReal[x][y].y != 0) {
				int a = 0;
				int roiheight = ROIs[x][y].rows;
				int roiwidth = ROIs[x][y].cols;
				cv::imshow("BLUR", ROIs[x][y]);
				std::cout << "roiwidth " << roiwidth << " " << "roiheight " << roiheight << std::endl;
				for (int iy = roiheight - 1; iy >= 0; iy--) {
					for (int ix = roiwidth - 1; ix >= 0; ix--) {
						if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
							a++;
						}
					}
					if (a != 0) {
						std::cout << "linethickness " << a << std::endl;
						return a;
					}
				}
			}
		}
	}
}


int get_av_width(int x, int y) {
	int thickness = 0;
	int wholethickness = 0;
	int roiheight = ROIs[x][y].rows;
	int roiwidth = ROIs[x][y].cols;
	int countedrows = 0;
	for (int iy = roiheight - 1; iy >= 0; iy--) {
		thickness = 0;
		for (int ix = roiwidth - 1; ix >= 0; ix--) {
			if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
				thickness++;
			}
		}
		if (thickness != 0) {
			countedrows++;
			wholethickness += thickness;
		}
	}
	return wholethickness / countedrows;
}

int get_av_height(int x, int y) {
	int thickness = 0;
	int wholethickness = 0;
	int roiheight = ROIs[x][y].rows;
	int roiwidth = ROIs[x][y].cols;
	int countedrows = 0;
	for (int ix = roiwidth - 1; ix >= 0; ix--) {
		thickness = 0;
		for (int iy = roiheight - 1; iy >= 0; iy--) {
			if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
				thickness++;
			}
		}
		if (thickness != 0) {
			countedrows++;
			wholethickness += thickness;
		}
	}
	return wholethickness / countedrows;
}