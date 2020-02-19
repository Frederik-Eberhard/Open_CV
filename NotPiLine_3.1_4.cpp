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

const float mingroß = 0.1;   //prozent von gesamten ROI, die die Linie mindestens aufnehmen muss, bevor sie gezählt wird  (0.1)
const float cerheigth = 0.6; //linethickness value (how high, bevor it is considered to be vertical and is counted)

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

//cv::Point Links[ANZAHL][ANZAHL][4][4];  //first two [][] for current x,y pos  next[] for which connection and last[] for wich connection on the deleted node
std::vector<cv::Point> Links[ANZAHL][ANZAHL];  //first two [][] for current x,y pos and vector for all the Links of that field
cv::Point point;  //temp point storage
int tempdirection2; //tempdirection
int extx = 0;	// extra x value
int exty = 0;	// extra y value
//cv::Point TempLinks[ANZAHL][ANZAHL][4][4];  //first two [][] for current x,y pos  next[] for which connection and last[] for wich connection on the deleted node
std::vector<cv::Point> TempLinks[ANZAHL][ANZAHL];

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
//const int extravalue = 5;  //extravalue + linthickness for linescheck()
const float percentage = 1.2;  //linethickness * percentage 

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
float get_median_thickness(int x, int y, bool direction);
int get_borderpos(int x, int y, int direction);
int get_borderthickness(int x, int y, int direction);
int get_maxdimensions(int x, int y, bool direction);
bool similar(int a, int b);

void v_sort(vector<int>&);


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


		//---TEMPORARY---------
		/*
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
		}*/
		//-----------------




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
					if (grocont > (roisize * mingroß)) { //wie viel des Felds soll die Kontur belegen, bevor sie gez�hlt wird
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
				std::cout << "x  " << x << "  y  " << y << std::endl;

				if (y != 0) {
					if (DotsReal[x][y - 1] != cv::Point(0, 0)) {  //top
						std::cout << "1111  " << std::endl;
						if (similar(get_borderpos(x, y, 0),get_borderpos(x, y - 1, 2))) {  //check if they are touching
							Connections[x][y][0] = true;
						}
					}
				}
				if (x < ANZAHL - 1) {		//ANZAHL - 1, da der Array von 0 - 4 geht und ich wil, dass er schon bei 4 dieses nicht macht
					if (DotsReal[x + 1][y] != cv::Point(0, 0)) {  //right
						std::cout << "2222  " << std::endl;
						if (similar(get_borderpos(x, y, 1), get_borderpos(x + 1, y, 3))) {
							Connections[x][y][1] = true;
						}
					}
				}
				if (y < ANZAHL - 1) {
					if (DotsReal[x][y + 1] != cv::Point(0, 0)) {  //bottom
						std::cout << "3333  " << std::endl;
						if (similar(get_borderpos(x, y, 2), get_borderpos(x, y + 1, 0))) {
							Connections[x][y][2] = true;
						}
					}
				}
				if (x != 0) {
					if (DotsReal[x - 1][y] != cv::Point(0, 0)) {  //left
						std::cout << "4444  " << std::endl;
						if (similar(get_borderpos(x, y, 3), get_borderpos(x - 1, y, 1))) {
							Connections[x][y][3] = true;
						}
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
	int  tempdirection2 = 0;
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			Links[x][y].clear();  // leert den Vector
		}
	}

	std::cout << "   0   1   2   3   4 " << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		std::cout << "   " << Connections[0][i][0] << "   " << Connections[1][i][0] << "   " << Connections[2][i][0] << "   " << Connections[3][i][0] << "   " << Connections[4][i][0] << std::endl;
		std::cout << i << " " << Connections[0][i][3] << Links[0][i].size() << Connections[0][i][1] << " " << Connections[1][i][3] << Links[1][i].size() << Connections[1][i][1] << " " << Connections[2][i][3] << Links[2][i].size() << Connections[2][i][1] << " " << Connections[3][i][3] << Links[3][i].size() << Connections[3][i][1] << " " << Connections[4][i][3] << Links[4][i].size() << Connections[4][i][1] << std::endl;
		std::cout << "   " << Connections[0][i][2] << "   " << Connections[1][i][2] << "   " << Connections[2][i][2] << "   " << Connections[3][i][2] << "   " << Connections[4][i][2] << std::endl;

	}

	if (true) {  // to isolate the variables
		int a, b;
		bool otherbigger, morelinks;
		for (int y = 0; y < ANZAHL; y++) {
			for (int x = 0; x < ANZAHL; x++) {
				if (DotsReal[x][y] != cv::Point(0, 0)) {
					std::cout << "Hallo8.1 " << x << "  " << y << std::endl;
					for (int i = 0; i < 4; i++) {
						std::cout << "Hallo8.2 " << i << "  " << x << "  " << y << std::endl;
						otherbigger = false;
						morelinks = false;
						if (Connections[x][y][i] == true) {
							std::cout << "Hallo8.3 " << std::endl;
							if (i == 0 || i == 2) {  // top || bottom
								extx = 0;
								if (i == 0) {
									exty = -1;	//für top -> x + 0, y - 1
								}
								if (i == 2) {
									exty = 1;	 //für bottom -> x + 0, y + 1
								}
								a = get_median_thickness(x, y, true);  //true = height  false = width
								b = get_median_thickness(x + extx, y + exty, true);
							}
							if (i == 1 || i == 3) {  // right || left
								exty = 0;
								if (i == 1) {
									extx = 1;	//für right -> x + 1, y + 0
								}
								if (i == 3) {
									extx = -1;	 //für bottom -> x - 1, y + 0
								}
								a = get_median_thickness(x, y, false);  //true = height  false = width
								b = get_median_thickness(x + extx, y + exty, false);
							}
							std::cout << "a " << a << " b " << b << std::endl;
							if (DotsReal[x + extx][y + exty] != cv::Point(0, 0)) {
								if (a + b < linethickness * percentage) {
									std::cout << "Hallo8.4 " << std::endl;
									int avx = (DotsReal[x][y].x + DotsReal[x + extx][y + exty].x) / 2;
									int avy = (DotsReal[x][y].y + DotsReal[x + extx][y + exty].y) / 2;
									if (a < b) {	//other bigger than x,y
										x += extx;  //making the bigger one the current one
										y += exty;
										extx *= -1;
										exty *= -1;
										otherbigger = true;
										//goto icount;
										std::cout << " x " << x << " y " << y << std::endl;
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

									if (otherbigger == true) {
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
									//----Check-for-Links----
									if (Links[x + extx][y + exty].size() > 0) { // if there are links
										for (int i = 0; i < Links[x + extx][y + exty].size(); i++) {
											Links[x][y].push_back(Links[x + extx][y + exty][i]);  
											for (int j = 0; j < Links[Links[x][y].back().x][Links[x][y].back().y].size(); j++) {  //go through vector ar linked to field
												if (Links[Links[x][y].back().x][Links[x][y].back().y][j] == cv::Point(x + extx, y + exty)) {  //find old link
													Links[Links[x][y].back().x][Links[x][y].back().y][j] = cv::Point(x, y);  // link it to current 
												}
											}
										}
										Links[x + extx][y + exty].clear();  // seting back of now unused links
									}
									//Check-for-Connections--
									for (int j = 0; j < 4; j++) {
										std::cout << "j " << j << " tempdirection2 " << tempdirection2 << " i " << i << std::endl;
										if (Connections[x + extx][y + exty][j] == true) {
											if (j != tempdirection2) {
												std::cout << "HALSKDJASLDKJASDLAKDJ" << std::endl;
												point = cv::Point(0, 0);
												if (j == 0) {  //top
													point = cv::Point(x + extx, y + exty - 1);
													morelinks = true;
												}
												if (j == 1) {  //right
													point = cv::Point(x + extx + 1, y + exty);
													morelinks = true;
												}
												if (j == 2) {  //bottom
													point = cv::Point(x + extx, y + exty + 1);
													morelinks = true;
												}
												if (j == 3) {  //left
													point = cv::Point(x + extx - 1, y + exty);
													morelinks = true;
												}
												Links[x][y].push_back(point);
												Links[point.x][point.y].push_back(cv::Point(x, y));

												std::cout << " x " << x << " y " << y << " i " << i << " j " << j << std::endl;
												std::cout << "LINKS" << Links[x][y].back() << std::endl;
												std::cout << " point.x " << point.x << " point.y " << point.y << " tempdirection2 " << tempdirection2 << std::endl;
												std::cout << "LINKS" << Links[point.x][point.y].back() << std::endl;
											}
											switch (j)  //seting back connected connections
											{
											case 0: Connections[x + extx][y + exty - 1][2] = false; break;  //top
											case 1: Connections[x + extx + 1][y + exty][3] = false; break;  //right
											case 2: Connections[x + extx][y + exty + 1][0] = false; break;  //bottom
											case 3: Connections[x + extx - 1][y + exty][1] = false; break;  //left
											default: break;
											}

										}
										Connections[x + extx][y + exty][j] = false;
									}
									Connections[x][y][i] = false;  // no linking to oneself  (-> REWORK!!! (if only link is to oneself))
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
			TempLinks[x][y].clear();
		}
	}

	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < Links[x][y].size(); i++) {
					TempLinks[x][y].push_back(Links[x][y][i]);
			}
		}
	}
	std::cout << "   0   1   2   3   4 " << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		std::cout << "   " << Connections[0][i][0] << "   " << Connections[1][i][0] << "   " << Connections[2][i][0] << "   " << Connections[3][i][0] << "   " << Connections[4][i][0] << std::endl;
		std::cout << i << " " << Connections[0][i][3] << Links[0][i].size() << Connections[0][i][1] << " " << Connections[1][i][3] << Links[1][i].size() << Connections[1][i][1] << " " << Connections[2][i][3] << Links[2][i].size() << Connections[2][i][1] << " " << Connections[3][i][3] << Links[3][i].size() << Connections[3][i][1] << " " << Connections[4][i][3] << Links[4][i].size() << Connections[4][i][1] << std::endl;
		std::cout << "   " << Connections[0][i][2] << "   " << Connections[1][i][2] << "   " << Connections[2][i][2] << "   " << Connections[3][i][2] << "   " << Connections[4][i][2] << std::endl;

	}
}
/*
void linescheck() {
	point = cv::Point(0, 0);
	tempdirection1 = 0;
	tempdirection2 = 0;
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < Links[x][y][i].size(); j++) {
					Links[x][y][i].pop_back();  // leert den Vector
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
						std::cout << "Hallo8.2 " << i << "  " << x << "  " << y << std::endl;
						otherbigger = false;
						morelinks = false;
						if (Connections[x][y][i] == true) {
							std::cout << "Hallo8.3 " << std::endl;
							if (i == 0 || i == 2) {  // top || bottom
								extx = 0;
								if (i == 0) {
									exty = -1;	//für top -> x + 0, y - 1
								}
								if (i == 2) {
									exty = 1;	 //für bottom -> x + 0, y + 1
								}
								a = get_median_thickness(x, y , true);  //true = height  false = width
								b = get_median_thickness(x + extx, y + exty, true);
							}
							if (i == 1 || i == 3) {  // right || left
								exty = 0;
								if (i == 1) {
									extx = 1;	//für right -> x + 1, y + 0
								}
								if (i == 3) {
									extx = -1;	 //für bottom -> x - 1, y + 0
								}
								a = get_median_thickness(x, y, false);  //true = height  false = width
								b = get_median_thickness(x + extx, y + exty,false);
							}
							if (DotsReal[x + extx][y + exty] != cv::Point(0, 0)) {
								if (a + b < linethickness + extravalue) {
									std::cout << "Hallo8.4 " << std::endl;
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
				std::cout << "Hallo9.1 " << x << " " << y << std::endl;
				std::cout << "Hallo9.1 dotsReal " << DotsReal[x][y].x << " " << DotsReal[x][y].y << std::endl;
				repeat(x, y);
				for (int a = 0; a < amount; a++) {
					std::cout << "Hallo9.1 lines " << linesstart[a].x << " " << linesstart[a].y << " , " << linesend[a].x << " " << linesend[a].y << std::endl;
				}
				return;
			}
			std::cout << "Hallo9.1.5 " << x << " " << y << std::endl;
		}
	}
}

void repeat(int x, int y) {
	std::cout << "Hallo9.1.0 " << TempLinks[x][y].max_size() << " " << x << " " << y << std::endl;
	temppoint = cv::Point(0, 0);
	//--Links----------
	if (TempLinks[x][y].size() != 0) {
		std::cout << "Hallo9.1.1 " << std::endl;
		int lsize = TempLinks[x][y].size();  //linkssize
		//for (int i = 0; i < lsize; i++) {
		while(TempLinks[x][y].size() > 0){
			std::cout << "Hallo9.1.2 "  << " " << TempLinks[x][y].size() << std::endl;
			linesstart[amount] = DotsReal[x][y];
			std::cout << "Hallo9.1.2.1 " << TempLinks[x][y][0] << std::endl;
			temppoint = TempLinks[x][y][0];  //always the first, since it is deleated later on and there by prevents loops 
			std::cout << "Hallo9.1.2.2 " << std::endl;
			linesend[amount] = DotsReal[temppoint.x][temppoint.y];
			std::cout << "Hallo9.1.3 " << std::endl;
			for (int j = 0; j < TempLinks[temppoint.x][temppoint.y].size(); j++) {
				if (TempLinks[temppoint.x][temppoint.y][j] == cv::Point(x, y)) {
					std::cout << "Hallo9.1.4 " << std::endl;
					TempLinks[temppoint.x][temppoint.y].erase(TempLinks[temppoint.x][temppoint.y].begin() + j);
				}
			}
			std::cout << "Hallo9.1.5 " << std::endl;
			amount++;
			std::cout << "Hallo9.1.6 " << std::endl;
			TempLinks[x][y].erase(TempLinks[x][y].begin());  //deleating current 
			std::cout << "Hallo9.1.7 " << std::endl;
			repeat(temppoint.x, temppoint.y);
		}
		//TempLinks[x][y].clear();
	}
	//--Connections----
	std::cout << "Hallo9.1.8 " << std::endl;
	for (int i = 0; i < 4; i++) {
		if (TempConnections[x][y][i] == true) {
			if (i == 0) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x][y - 1];
				TempConnections[x][y - 1][2] = false;
				amount++;
				repeat(x, y - 1);
			}
			if (i == 1) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x + 1][y];
				TempConnections[x + 1][y][3] = false;
				amount++;
				repeat(x + 1, y);
			}
			if (i == 2) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x][y + 1];
				TempConnections[x][y + 1][0] = false;
				amount++;
				repeat(x, y + 1);
			}
			if (i == 3) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x - 1][y];
				TempConnections[x - 1][y][1] = false;
				amount++;
				repeat(x - 1, y);
			}
		}
	}
}
/*
void repeat(int x, int y) {
	tempdirection1 = 0;  //direction from linked-to-node to missing node
	tempdirection2 = 0;  //direction from missing node to current node
	temppoint = cv::Point(0, 0);
	linksamount = 0;
	for (int i = 0; i < 4; i++) {
		linksamount = 0;
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
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y - 1];
					TempConnections[x][y - 1][2] = false;
					amount++;
					repeat(x, y - 1);
				}
				if (i == 1) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x + 1][y];
					TempConnections[x + 1][y][3] = false;
					amount++;
					repeat(x + 1, y);
				}
				if (i == 2) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x][y + 1];
					TempConnections[x][y + 1][0] = false;
					amount++;
					repeat(x, y + 1);
				}
				if (i == 3) {
					TempConnections[x][y][i] = false;
					linesstart[amount] = DotsReal[x][y];
					linesend[amount] = DotsReal[x - 1][y];
					TempConnections[x - 1][y][1] = false;
					amount++;
					repeat(x - 1, y);
				}
			}
		}
	}
}*/

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

/*
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
*/

int find_linethickness() {
	vector<int> widths;
	bool first = true;
	bool line = false;
	int h = 0; //height
	int a = 0;
	int th = 0; //temp height
	int maxh = 0; //max height
	for (int y = ANZAHL - 1; y >= 0; y--) {
		for (int x = ANZAHL - 1; x >= 0; x--) {
			if (DotsReal[x][y] != cv::Point(0, 0)) {
				th = 0;
				th = get_maxdimensions(x, y, true);  //true = height
				if (th > maxh) {
					maxh = th;
				}
			}
		}
	}

	for (int y = ANZAHL - 1; y >= 0; y--) {
		for (int x = ANZAHL - 1; x >= 0; x--) {
			if (DotsReal[x][y] != cv::Point(0,0)) {
				int roiheight = ROIs[x][y].rows;
				int roiwidth = ROIs[x][y].cols;
				
				h = get_maxdimensions(x, y, true);  //true = height
				if (similar(h,maxh)) {  // if height is similar to max height
					cv::imshow("BLUR", ROIs[x][y]);
					std::cout << "roiwidth " << roiwidth << " " << "roiheight " << roiheight << std::endl;
					for (int iy = roiheight - 1; iy >= 0; iy--) {
						a = 0;
						for (int ix = roiwidth - 1; ix >= 0; ix--) {
							if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
								a++;
							}
						}
						std::cout << "x " << x << "  a " << a << std::endl;
						if (a != 0) {
							if (first == true || roiheight - iy > widths.size() - 1) {
								widths.push_back(a);
							}
							else {
								widths[roiheight - iy] += a;
							}
						}
					}
					first = false;
				}
			}
		}
		if (widths.size() > 0) {
			v_sort(widths);
			std::cout << "Widths:  ";
			for (int i = 0; i < widths.size(); i++) {
				std::cout << widths[i] << "  ";
			}
			std::cout << std::endl;
			if (widths.size() % 2 == 0) {  // if even
				std::cout << "linethickness even " << (widths[widths.size() / 2] + widths[(widths.size() / 2) + 1]) / 2 << std::endl;
				return (widths[widths.size() / 2] + widths[(widths.size() / 2) + 1]) / 2;
			}
			else {  //if odd
				std::cout << "linethickness odd " << widths[(widths.size() + 1) / 2] << std::endl;
				return widths[(widths.size() + 1) / 2];
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

float get_median_thickness(int x, int y, bool direction) {  //true = height  false = width   
	int width = ROIs[x][y].cols;
	int height = ROIs[x][y].rows;
	int thickness = 0;
	int wholethickness = 0;
	int countedrows = 0;
	vector<int> values;
	if (direction == false) {  //width check
		for (int iy = height - 1; iy >= 0; iy--) {
			thickness = 0;
			for (int ix = width - 1; ix >= 0; ix--) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
						thickness++;
					}
				}
			}
			if (thickness != 0) {
				countedrows++;
				values.push_back(thickness);
			}
		}
	}
	else {  //height check
		for (int ix = width - 1; ix >= 0; ix--) {
			thickness = 0;
			for (int iy = height - 1; iy >= 0; iy--) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
						thickness++;
					}
				}
			}
			if (thickness != 0) {
				countedrows++;
				values.push_back(thickness);
			}
		}
	}

	v_sort(values);

	std::cout << "Vector:  ";
	for (int i = 0; i < values.size(); i++) {
		std::cout << values[i] << "  ";
	}
	std::cout << std::endl;

	if (values.size() % 2 == 0) {  // if even
		return (values[values.size() / 2] + values[(values.size() / 2) + 1]) / 2;  
	}
	else {  //if odd
		return values[(values.size() + 1) / 2];  
	}
}

int get_borderpos(int x, int y, int direction) {
	std::cout << "get_borderpos" << std::endl;
	if (direction == 0) {//top
		for (int iy = 0; iy < ROIs[x][y].rows; iy++) {
			for (int ix = 0; ix < ROIs[x][y].cols; ix++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy),false) > 0) {  //checks if point is in Contour
						return iy;		//check how far the first x_pos/y_pos is from the border
					}
				}
			}
		}
	}
	if (direction == 1) {  //right
		for (int ix = ROIs[x][y].cols - 1; ix >= 0; ix--) {
			for (int iy = 0; iy < ROIs[x][y].rows; iy++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
						return ROIs[x][y].cols - 1 - ix;		//check how far the first x_pos/y_pos is from the border 
					}
				}
			}
		}
	}
	if (direction == 2) { //bottom
		for (int iy = ROIs[x][y].rows - 1; iy >= 0; iy--) {
			for (int ix = 0; ix < ROIs[x][y].cols; ix++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
						return ROIs[x][y].rows - 1 - iy;		//check how far the first x_pos/y_pos is from the border
					}
				}
			}
		}
	}
	if (direction == 3) { //left
		for (int ix = 0; ix < ROIs[x][y].cols; ix++) {
			for (int iy = 0; iy < ROIs[x][y].rows; iy++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
						return ix;		//check how far the first x_pos/y_pos is from the border
					}
				}
			}
		}
	}
}

int get_borderthickness(int x, int y, int direction) {   //WORK IN PROGRESS
	int thickness = 0;
	int ix = 0;
	int iy = 0;
	int width = 0;   //means both width and height in this case
	switch (direction)
	{
	case 0:  //top
		iy = 0;
		ix = -1; 
		width = ROIs[x][y].cols;
		break;
	case 1:  //right
		iy = -1;
		ix = ROIs[x][y].cols - 1;
		width = ROIs[x][y].rows; 
		break;
	case 2: //bottom
		iy = ROIs[x][y].rows - 1;
		ix = -1;
		width = ROIs[x][y].cols; 
		break;
	case 3: //left
		iy = -1;
		ix = 0;
		width = ROIs[x][y].rows; 
		break;
	default: break;
	}
	if (ix == -1) {
		for (int ix = 0; ix < width; ix++) {
			if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
				thickness++;
			}
		}
	}
	if (iy == -1) {
		for (int iy = 0; iy < width; iy++) {
			if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
				thickness++;
			}
		}
	}
	return thickness;
}

int get_maxdimensions(int x, int y, bool direction) {  //true = height  false = width 
	int w = ROIs[x][y].cols;
	int h = ROIs[x][y].rows;
	int max = 0;
	int thickness = 0;
	if (direction == true) {  //height
		for (int ix = 0; ix < w; ix++) {
			thickness = 0;
			for (int iy = 0; iy < h; iy++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {
					thickness++;
				}
			}
			if (thickness > max) {
				max = thickness;
			}
		}
	}
	else {  //width
		for (int iy = 0; iy < h; iy++) {
			thickness = 0;
			for (int ix = 0; ix < w; ix++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {
					thickness++;
				}
			}
			if (thickness > max) {
				max = thickness;
			}
		}
	}
	return max;
}

bool similar(int a, int b) {
	std::cout << "a " << a << "   b " << b << std::endl;
	int v = 2;
	if (a < b + v && b < a + v) {
		std::cout << "true" << std::endl;
		return true;
	}
	else {
		std::cout << "false" << std::endl;
		return false;
	}
}

void v_sort(vector<int>& v) {
	int c = 0; //c -> count
	for (int i = 1; i < v.size(); i++) {
		if (v[i-1] > v[i]) {
			iter_swap(v.begin() + i-1, v.begin() + i);
			c++;
		}
	}
	if(c == 0){
		return;
	}
	else{
		v_sort(v);
	}
}