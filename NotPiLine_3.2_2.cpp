﻿//============================================================================
// Name        : VideoShow.cpp
// Author      : Frederik Eberhard
// Version     :
// Copyright   : Your copyright notice
// Description : Line-detection-programm (no comms)
//============================================================================

#include <iostream>
#include "opencv2/opencv.hpp"



using namespace std;
using namespace cv;

int pic_rows;
int pic_cols;
const int ANZAHL = 5;

const float mingross = 0.1;   //prozent von gesamten ROI, die die Linie mindestens aufnehmen muss, bevor sie gezählt wird  (0.1)
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
cv::Mat img_bin;
cv::Mat img_hsv;
std::vector<cv::Mat> hsv_splitmat;  //Ein Vektor von Matritzen für Hue, Saturation und Value
int color_values[256];
int blackvalue;

std::vector<cv::Mat> splitmat;  //Ein Vektor von Matritzen für Rot, Gruen und Blau

cv::Mat ROIs[ANZAHL][ANZAHL]; //Vector of Matritzen
std::vector<std::vector<cv::Point> > contours_arr[ANZAHL][ANZAHL]; //für schwarz

cv::Point Dots[ANZAHL][ANZAHL];
cv::Point DotsReal[ANZAHL][ANZAHL];
bool Connections[ANZAHL][ANZAHL][4];  //[x][y]Point on grid 0123 Connections to  top left bottom right

									  //cv::Point Links[ANZAHL][ANZAHL][4][4];  //first two [][] for current x,y pos  next[] for which connection and last[] for wich connection on the deleted node
std::vector<cv::Point> Links[ANZAHL][ANZAHL];  //first two [][] for current x,y pos and vector for all the Links of that field
cv::Point point;  //temp point storage
int tempdirection1; //tempdirection
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

vector<vector<vector<cv::Point>>> Vertecies;  //all final Vertecies sorted by line
vector<cv::Point> end_line;					  //final line of Points

cv::Point greendots[2];
std::vector<std::vector<cv::Point> > contours_green[2]; //für grün ( 2 für die möglichen zwei grünen Punkte )
int index_green[2];

cv::Point heading = cv::Point();

cv::Point startPoint;  //starting Point of Line


int linethickness = 0;
const float percentage = 1.2;  //linethickness * percentage 7

void color_set();  //name is work in progress
void img_set();
void makeROIs();
void makeConturesandDots();
void lines();
void linescheck();
void find_greendot();
void link_clean();
void get_startPoint(cv::Point& sP);	//gets the Startpoint of the Line
void loops();
void line_evaluation();
void find_heading();
void draw();
void show();

int find_linethickness();
int get_av_width(int x, int y);
int get_av_height(int x, int y);
float get_median_thickness(int x, int y, bool direction);
int get_borderpos(int x, int y, int direction);
int get_borderthickness(int x, int y, int direction);
int get_maxdimensions(int x, int y, bool direction);
float get_median(vector<int>&);
float get_average(vector<int>&);
bool similar(int a, int b);
float dist(cv::Point a, cv::Point b);
cv::Point get_fork(bool(&arr)[ANZAHL][ANZAHL]);
vector<cv::Point> get_forks();
vector<vector<cv::Point>> get_fork_verts(vector<cv::Point>& forks, int pos);
void get_fork_vertex(cv::Point pos, cv::Point startingFork, vector<cv::Point>& forks, bool(&TConnections)[ANZAHL][ANZAHL][4], std::vector<cv::Point>(&TLinks)[ANZAHL][ANZAHL], vector<cv::Point>& vertex);
void create_vertecies(vector<vector<cv::Point>>& final_verts, cv::Point lastVertexEnd, cv::Point lastPosition, vector<vector<vector<cv::Point>>>& vertecies);
void get_line_in_Vertecies(vector<vector<vector<cv::Point>>>& vertecies, vector<cv::Point>& line, cv::Point currentPoint);
void get_dest_forks(vector<vector<vector<cv::Point>>>& vertecies, vector<cv::Point>& forks);
void v_sort(vector<int>&);


int main() {

	//Erstelle Objekt von Klasse cv::VideoCapture und �ffne das Video "line.avi"
	cv::VideoCapture cap("line2.avi");

	//Pr�fe ob das Video geöffnet werden konnte
	if (!cap.isOpened()) {
		std::cout << "Problem beim �ffnen der Datei" << std::endl;
		return -1;
	}

	//Setze Start-frame fest 
	//double count = cap.get(CV_CAP_PROP_FRAME_COUNT);
	//cap.set(CV_CAP_PROP_POS_FRAMES, (int)count / 5 * 3);
	
	//Setze Framerate und die Auflösung
	cap.set(cv::CAP_PROP_FPS, 30);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	//Füge Fenster zum Server hinzu
	cv::namedWindow("RGB");
	cv::namedWindow("BLUR");
	cv::namedWindow("GREEN");
	//	cv::namedWindow("Edges");
	cv::namedWindow("BinSW");

	while (1) {
		cap.read(img_rgb);
		if (img_rgb.empty()) {
			std::cout << "Fehler beim holen des Bildes" << std::endl;
			return -2;
		}

		pic_rows = img_rgb.rows;
		pic_cols = img_rgb.cols;

		color_set();
		img_set();
		makeROIs();
		makeConturesandDots();
		lines();
		linethickness = find_linethickness();
		linescheck();
		find_greendot();
		link_clean();
		get_startPoint(startPoint);
		loops();
		line_evaluation();
		find_heading();
		draw();
		show();		
	}
	return 0;
}	//Oben wurde zuweit eingeschoben
	/*
	void color_set() {
	cv::cvtColor(img_rgb, img_edgy, CV_BGR2GRAY);
	img_sat = Mat::zeros(img_rgb.size(), img_rgb.type());
	createTrackbar("Trackbar", "Edges", &alpha, 500);
	createTrackbar("Trackbar2", "Edges", &beta, 1000);
	img_edgy.convertTo(img_sat, -1, alpha, -beta);
	}

	void color_set() {
	createTrackbar("Trackbar", "Edges", &thrVal, 500);
	cv::cvtColor(img_rgb, img_edgy, CV_BGR2GRAY);
	Canny(img_edgy, img_edgy, thrVal, thrVal*3, 3);
	vector<Vec4i> lines;
	createTrackbar("Trackbar2", "RGB", &thrVal2, 500);
	createTrackbar("Trackbar3", "RGB", &thrVal3, 500);
	createTrackbar("Trackbar4", "RGB", &thrVal4, 500);
	HoughLinesP(img_edgy, lines, 1, CV_PI / 180, thrVal2, thrVal3, thrVal4);
	for (size_t i = 0; i < lines.size(); i++)
	{
	line(img_rgb, Point(lines[i][0], lines[i][1]),
	Point(lines[i][2], lines[i][3]), Scalar(0, 255, 255), 3, 8);
	}
	}
	*/
	/*
	void color_set() {
	brightnesses.clear();
	differences.clear();

	std::cout << "Hallo3.1 " << std::endl;
	//RGB -> HSV
	cv::cvtColor(img_rgb, img_hsv, cv::COLOR_BGR2HSV);
	cv::split(img_hsv, hsv_splitmat);

	for (int y = pic_rows - 1; y > 0 + ((pic_rows / 5) * 2); y -= (pic_rows / 5)) {
	for (int x = 0; x < pic_cols; x += 2) {
	brightnesses.push_back(hsv_splitmat[2].at<uchar>(y, x));  //using only the value (brightness of pixel)
	}
	}
	v_sort(brightnesses);
	std::cout << "Hallo3.4 " << std::endl;
	std::cout << "Brightnesses : ";
	for (int i = 0; i < brightnesses.size(); i++) {
	std::cout << " " << brightnesses[i];
	}
	std::cout << std::endl;

	for (int j = 0; j < brightnesses.size() - 1; j++) {
	differences.push_back(cv::Point((brightnesses[j + 1] - brightnesses[j]), j));
	}
	vector<int> holdvector;
	for (int i = 0; i < differences.size(); i++) {
	holdvector.push_back(differences[i].x);
	}
	v_sort(holdvector);
	std::cout << "Hallo3.5 " << std::endl;
	std::cout << "Differences : ";
	for (int i = 0; i < holdvector.size(); i++) {
	std::cout << " " << holdvector[i];
	}
	std::cout << std::endl;

	int v = holdvector[holdvector.size() - 1];
	for (int i = 0; i < differences.size(); i++) {
	if (differences[i].x == v) {
	blackvalue = brightnesses[differences[i].y];
	}
	}
	std::cout << "Hallo3.6 " << std::endl;
	std::cout << "Blackvalue " << blackvalue << std::endl;

	}*/
	/*
	void color_set() {
	blacks.clear();
	blackvalue = 0;

	std::cout << "Hallo3.1 " << std::endl;
	cv::cvtColor(img_rgb, img_hsv, cv::COLOR_BGR2HSV);
	cv::split(img_hsv, hsv_splitmat);

	int mediandiffernce = 0;
	int beginning = 0;
	int end = pic_cols;

	for (int i = 0; i < 3; i++) {
	brightnesses.clear();
	differences.clear();

	mediandiffernce = 0;
	beginning = 0;
	end = pic_cols;

	int y = static_cast<int>((pic_rows / 5) * (5 - i)) - 1;  //static_cast<int> converts to data type ( in this case int )
	std::cout << "Hallo3.2 " << " y " << y << std::endl;
	for (int x = 0; x < pic_cols; x += 2) {
	brightnesses.push_back(hsv_splitmat[2].at<uchar>(y, x));  //using only the value (brightness of pixel)
	}

	std::cout << "brightnesses : ";
	for (int i = 0; i < brightnesses.size(); i++) {
	std::cout << " " << brightnesses[i];
	}
	std::cout << std::endl;

	for (int j = 0; j < brightnesses.size() - 1; j++) {
	differences.push_back(std::abs(brightnesses[j + 1] - brightnesses[j]));  //absolute difference
	}

	std::cout << "differences : ";
	for (int i = 0; i < differences.size(); i++) {
	std::cout << " " << differences[i];
	}
	std::cout << std::endl;

	mediandiffernce = get_average(differences);
	std::cout << "Hallo3.3 " << " mediandiffernce " << mediandiffernce << std::endl;
	for (int j = 0; j < differences.size(); j++) {
	if (differences[j] > (mediandiffernce + 1) * 2) {
	std::cout << "Hallo3.4 " << std::endl;
	if (brightnesses[j] > brightnesses[j + 1]) {  //if current is bigger than next ( white -> black )
	beginning = j + 1;
	}
	else {
	end = j;
	}
	}
	}

	std::cout << "Hallo3.5 " << "beginning : " << beginning << " end : " << end << std::endl;

	if (beginning != 0 || end != pic_cols) {
	for (int j = beginning; j < end; j++) {
	blacks.push_back(brightnesses[j]);
	}
	}
	}
	std::cout << "blacks : ";
	for (int i = 0; i < blacks.size(); i++) {
	std::cout << " " << blacks[i];
	}
	std::cout << std::endl;

	blackvalue = get_average(blacks);
	}*/

void color_set() {			//ready for an overhaul!!!
	int val = 0;
	int loc[] = { 0,255 };  // 0 = black  1 = white
	bool ready = false;
	for (int i = 0; i < 256; i++) {
		color_values[i] = 0;
	}

	//RGB -> HSV
	cv::cvtColor(img_rgb, img_hsv, cv::COLOR_BGR2HSV);
	cv::split(img_hsv, hsv_splitmat);

	for (int y = 0; y < pic_rows; y++) {
		for (int x = 0; x < pic_cols; x++) {
			val = hsv_splitmat[2].at<uchar>(y, x);
			color_values[val]++;
		}
	}

	for (int i = 0; i < 256; i++) {
		if (color_values[i] > 3000) {
			ready = true;
		}
		if (ready == true) {
			if (color_values[i] >= (color_values[loc[0]] * 0.3)) {
				if (color_values[i] > color_values[loc[0]]) {
					loc[0] = i;
				}
			}
			else {
				break;
			}
		}
	}
	ready = false;
	for (int i = 255; i >= 0; i--) {
		if (color_values[i] > 3000) {
			ready = true;
		}
		if (ready == true) {
			if (color_values[i] >= (color_values[loc[1]] * 0.3)) {
				if (color_values[i] > color_values[loc[1]]) {
					loc[1] = i;
				}
			}
			else {
				break;
			}
		}
	}

	blackvalue = (loc[0] + loc[1]) / 2 - 5;
}

/*
void color_set() {  //medium value of image
	int a = 0;
	for (int x = 0; x < pic_cols; x++) {
		for (int y = 0; y < pic_rows; y++) {
			a += hsv_splitmat[2].at<uchar>(y, x);
		}
	}
	a /= pic_cols*pic_rows;  //average image value
	blackvalue = a;
}
*/

void img_set() {

	//Binarisierung + Green Image
	cv::inRange(img_hsv, cv::Scalar(60, 100, 25), cv::Scalar(90, 255, 150), img_bin);
	cv::inRange(img_hsv, cv::Scalar(0, 0, 0), cv::Scalar(180, 255, blackvalue), img_binhell); //letzet Wert ist Helligkeit   (100)

																							  //White Image
	cv::split(img_rgb, splitmat);
	img_gminb = cv::abs(splitmat[0] - splitmat[1]);		// gruen - blau
	img_gminr = cv::abs(splitmat[1] - splitmat[2]);		// gruen - rot
	img_unterschied = cv::abs(img_gminb - img_gminr);	// Unterschied ausrechenen; klein=schwarz oder weiß
	cv::inRange(img_unterschied, cv::Scalar(0), cv::Scalar(8), img_binunt);

	cv::bitwise_and(img_binunt, img_binhell, img_binsw);
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

			//			  X  ,  Y  ,   WIDTH  ,   HEIGHT
			cv::Rect roi(xmin, ymin, xmax - xmin, ymax - ymin);
			ROIs[x][y] = img_binsw(roi);
			cv::GaussianBlur(ROIs[x][y], ROIs[x][y], Size(5, 5), 0, 0);
			cv::inRange(ROIs[x][y], cv::Scalar(0, 0, 0), cv::Scalar(50, 45, blackvalue), ROIs[x][y]);  //50 , 45 , 45
			cv::bitwise_not(ROIs[x][y], ROIs[x][y]); //schwarz weiß tauschen
		}
	}
}

void makeConturesandDots() {
	//-----all-contures----
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			cv::findContours(ROIs[x][y], contours_arr[x][y], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		}
	}

	//-----sorting-of-biggest-contour-and-getting-the-dots----
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
				if (m.m00 > grocont) {
					grocont = m.m00;
					if (grocont >(roisize * mingross)) { //wie viel des Felds soll die Kontur belegen, bevor sie gez�hlt wird
						Dots[ix][iy].x = m.m10 / m.m00;
						Dots[ix][iy].y = m.m01 / m.m00;
					}
					index_arr[ix][iy] = i;
				}
			}
		}
	}
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
			}
		}
	}
	for (int x = 0; x < ANZAHL; x++) {  //go through all and save neighbor field
		for (int y = 0; y < ANZAHL; y++) {
			if (DotsReal[x][y].x != 0 && DotsReal[x][y].y != 0) {

				if (y != 0) {
					if (DotsReal[x][y - 1] != cv::Point(0, 0)) {  //top
						if (similar(get_borderpos(x, y, 0), get_borderpos(x, y - 1, 2))) {  //check if they are touching
							Connections[x][y][0] = true;
						}
					}
				}
				if (x < ANZAHL - 1) {		//ANZAHL - 1, da der Array von 0 - 4 geht und ich wil, dass er schon bei 4 dieses nicht macht
					if (DotsReal[x + 1][y] != cv::Point(0, 0)) {  //right
						if (similar(get_borderpos(x, y, 1), get_borderpos(x + 1, y, 3))) {
							Connections[x][y][1] = true;
						}
					}
				}
				if (y < ANZAHL - 1) {
					if (DotsReal[x][y + 1] != cv::Point(0, 0)) {  //bottom
						if (similar(get_borderpos(x, y, 2), get_borderpos(x, y + 1, 0))) {
							Connections[x][y][2] = true;
						}
					}
				}
				if (x != 0) {
					if (DotsReal[x - 1][y] != cv::Point(0, 0)) {  //left
						if (similar(get_borderpos(x, y, 3), get_borderpos(x - 1, y, 1))) {
							Connections[x][y][3] = true;
						}
					}
				}
			}
		}
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

	if (true) {  // to isolate the variables
		int a, b;
		bool otherbigger, morelinks;
		for (int y = 0; y < ANZAHL; y++) {
			for (int x = 0; x < ANZAHL; x++) {
				if (DotsReal[x][y] != cv::Point(0, 0)) {
					for (int i = 0; i < 4; i++) {
						otherbigger = false;
						morelinks = false;
						if (Connections[x][y][i] == true) {
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
							if (DotsReal[x + extx][y + exty] != cv::Point(0, 0)) {
								if (a + b < linethickness * percentage) {
									int avx = (DotsReal[x][y].x + DotsReal[x + extx][y + exty].x) / 2;
									int avy = (DotsReal[x][y].y + DotsReal[x + extx][y + exty].y) / 2;
									if (a < b) {	//other bigger than x,y
										x += extx;  //making the bigger one the current one
										y += exty;
										extx *= -1;
										exty *= -1;
										otherbigger = true;
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
										if (Connections[x + extx][y + exty][j] == true) {
											if (j != tempdirection2) {
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
	//---größte-Konturen-(2)---
	double grocont = 0;  //größte Kontur
	for (int j = 0; j < 2; j++) {
		for (unsigned int i = 0; i < contours_green[j].size(); i++) {
			cv::Moments m = cv::moments(contours_green[j][i]);
			//cv::circle
			if (m.m00 > grocont && i != index_green[0]) {  // sodass es die größte ist, die aber nicht die erste ist ( 1. grüner Punkt )
				grocont = m.m00;
				greendots[j].x = m.m10 / m.m00;
				greendots[j].y = m.m01 / m.m00;
				index_green[j] = i;
			}
		}
		if (index_green[j] != -1) {
			cv::Moments a = cv::moments(contours_green[j][index_green[j]]);  //stoping the second point being much smaller than the first
			grocont = a.m00 / 2;
		}
	}
	//---Kontrolle-ob-Punkt----
	for (int i = 0; i < 2; i++) {
		if (greendots[i] != cv::Point(0, 0)) {
			int count_black = 0;
			cv::Moments m = cv::moments(contours_green[i][index_green[i]]);
			double height = sqrt(m.m00);
			for (int iy = 0; iy < height; iy++) {
				int x = greendots[i].x;
				int y = greendots[i].y - (height / 2) - iy;
				if (y >= 0) {
					if (img_binsw.at<uchar>(y, x) == 255) {
						count_black++;
					}
					cv::circle(img_rgb, cv::Point(x, y), 2, cv::Scalar((iy * 20), (255 - (iy * 20)), 0), 5);
				}
			}
			if (count_black < (height * 0.50)) {
				greendots[i] = cv::Point(0, 0);
				index_green[i] = -1;
			}
		}
	}
}

void link_clean() {												//important for fork_detection     
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < Links[x][y].size(); i++) {
				if (Links[x][y][i] == cv::Point(x, y - 1)|| Links[x][y][i] == cv::Point(x + 1, y) || Links[x][y][i] == cv::Point(x, y + 1) || Links[x][y][i] == cv::Point(x - 1, y)) {  //removes links that are also connections
					Links[x][y].erase(Links[x][y].begin() + i);
					i--;
				}
				else {
					for (int j = 0; j < Links[x][y].size(); j++) {   //removes links that are double
						if (j != i && Links[x][y][i] == Links[x][y][j]) {
							Links[x][y].erase(Links[x][y].begin() + j);
							j--;
						}
					}
				}
			}
		}
	}
}

void get_startPoint(cv::Point& sP) {
	int arr[ANZAHL*ANZAHL][2] = { { 2,4 },{ 1,4 },{ 3,4 },{ 2,3 },{ 1,3 },{ 3,3 },{ 0,4 },{ 4,4 },{ 2,2 },{ 1,2 },{ 3,2 },{ 0,3 },{ 4,3 },{ 2,1 },{ 1,1 },{ 1,3 },{ 0,2 },{ 4,2 },{ 2,0 },{ 1,0 },{ 3,0 },{ 0,1 },{ 4,1 },{ 0,0 },{ 4,0 } };
	for (int i = 0; i < ANZAHL*ANZAHL; i++) {
		if (DotsReal[arr[i][0]][arr[i][1]] != cv::Point(0, 0)) {
			sP = cv::Point(arr[i][0], arr[i][1]);				
			break;
		}
	}
}

void loops() {
	vector<cv::Point> forks;							//all pos of forks in img (start is always a fork!)
	forks = get_forks();


	//--removing-loops-part--------------------
	vector<vector<cv::Point>> forks_from_fork;			//all forks that a fork connects to 
	vector<vector<cv::Point>> final_verts;				//all final vertecies

	for (int i = 0; i < forks.size(); i++) {
		vector<vector<cv::Point>> fork_vertecies;		//all vertecies from this fork
		fork_vertecies = get_fork_verts(forks, i);
		vector<cv::Point> t_v;							//temp storage to forks that this fork connects to
		if (fork_vertecies.size() == 0 && forks[i] == startPoint) {  //add to startPoint if current one has no connections
			t_v.push_back(startPoint);
			t_v.push_back(startPoint);
			fork_vertecies.push_back(t_v);
			t_v.clear();
		}

		for (int j = 0; j < fork_vertecies.size(); j++) {
			int done = false;
			while (done == false) {		//to be able to  exit if vertecie has been looked at
				//--self-loop----------------------------
				if (fork_vertecies[j][fork_vertecies[j].size() - 1] == forks[i]) {  //if the last point is the same as the fork that I'm at -> self-loop
					float dist1 = dist(forks[i], fork_vertecies[j][1]);
					float dist2 = dist(forks[i], fork_vertecies[j][fork_vertecies[j].size() - 2]);
					if (dist1 < dist2) {		 //remove last point (remove furthest point)
						fork_vertecies[j].erase(fork_vertecies[j].begin() + fork_vertecies[j].size() - 1);		//remove last point of vertex
					}
					else {
						fork_vertecies[j].erase(fork_vertecies[j].begin());										//remove first point of vertex
						std::reverse(fork_vertecies[j].begin(), fork_vertecies[j].end());								//flip vertex
					}
					done = true;
				}
				if (done) { break; }

				//--simple-loop----------------------------
				if (!done) {
					for (int a = 0; a < fork_vertecies.size(); a++) {
						if (a != j && fork_vertecies[a][fork_vertecies[a].size() - 1] == fork_vertecies[j][fork_vertecies[j].size() - 1]) {  //if last point of two vertecies are the same -> simple-loop    -> remove longer connection
							int num = 0;
							if (fork_vertecies[a].size() > fork_vertecies[j].size()) {
								num = a;		//a gets deleted
							}
							else {
								num = j;		//j gets deleted
							}
							for (int b = 1; b < fork_vertecies[num].size() - 1; b++) {  // first of vertex is fork and last is other fork, so dont need to deleat them
								int x = fork_vertecies[num][b].x;
								int y = fork_vertecies[num][b].y;
								DotsReal[x][y] = cv::Point(0, 0);
								for (int c = 0; c < 4; c++) {
									if (Connections[x][y][c] == true) {
										Connections[x][y][c] = false;																							//delete Connection
										Connections[x + (c % 2 == 0 ? 0 : (2 - c))][y + (c % 2 == 0 ? (c - 1) : 0)][(c + 2) % 4] = false;						//delete Connection of other point to current Point   (i % 2 == 0 ? (i - 1) : 0) -> -1,0,1,0 for 0,1,2,3
									}
								}
								for (int c = 0; c < Links[x][y].size(); c++) {
									cv::Point TPoint = Links[x][y][c];
									for (int d = 0; d < Links[TPoint.x][TPoint.y].size(); d++) {
										if (Links[TPoint.x][TPoint.y][d] == cv::Point(x, y)) {
											Links[TPoint.x][TPoint.y].erase(Links[TPoint.x][TPoint.y].begin() + d);	 //erase point at linked to tile linking back to current point
											break;
										}
									}

								}
								Links[x][y].clear();						//clear vector
							}
							fork_vertecies.erase(fork_vertecies.begin() + num);

							if (num == j) {
								j -= 1;			//to not skipp a vertex 
							}
							done = true;
						}
						if (done) { break; }
					}
				}

				//--complex-loop---------------------------    -> Data storage Cleanup for end Data storage neccessary!

				if (!done) {
					for (int b = 0; b < forks.size(); b++) {
						if (fork_vertecies[j][fork_vertecies[j].size() - 1] == forks[b]) {  //if connected to other fork
							for (int c = 0; c < forks_from_fork.size(); c++) {
								for (int d = 0; d < forks_from_fork[c].size(); d++) {
									if (forks_from_fork[c][d] == fork_vertecies[j][fork_vertecies[j].size() - 1]) {  //if this fork is already connected to
										fork_vertecies.erase(fork_vertecies.begin() + j);							 //delete this vertex
										j -= 1;			//to not skipp a vertex		
										done = true;
										break;
									}
								}
								if (done) { break; }
							}
							if (done) { break; }
							else {
								t_v.push_back(forks[b]);													 //if not jet there add this fork to being connected to
								done = true;
							}
						}
						if (done) { break; }
					}
				}

				//--standard-vertex------------------------
				done = true;
			}

		}

		forks_from_fork.push_back(t_v);														 //push back all new forks that are being connected to	


		for (int j = 0; j < fork_vertecies.size(); j++) {  //saving final Vertecies
			final_verts.push_back(fork_vertecies[j]);
		}
	}

	//--cleaning-final_verts-----------------
	for (int i = 0; i < final_verts.size(); i++) {
		for (int j = 0; j < final_verts.size(); j++) {
			if (i != j && (final_verts[i][0] == final_verts[j][final_verts[j].size() - 1] && final_verts[j][0] == final_verts[i][final_verts[i].size() - 1])) {  //if there are two vertecies that are the same, just flipped -> remove one
				final_verts.erase(final_verts.begin() + j);
			}
		}
	}

	//--creating-Vertecies-------------------
	Vertecies.clear();
	create_vertecies(final_verts,startPoint,cv::Point(-1,-1),Vertecies);
}

vector<cv::Point> get_forks() {  //gets all forks + startPoint in Image
	vector<cv::Point> f;
	cv::Point sP = startPoint;
	for (int y = 0; y < ANZAHL; y++) {
		for (int x = 0; x < ANZAHL; x++) {
			int num = 0;
			for (int i = 0; i < 4; i++) {
				num += Connections[x][y][i] ? 1 : 0;
			}
			num += Links[x][y].size();
			if (num > 2 || cv::Point(x,y) == sP) {  //if fork
				f.push_back(cv::Point(x, y));
			}
		}
	}
	return f;
}

vector<vector<cv::Point>> get_fork_verts(vector<cv::Point>& forks, int pos) {		 //all vertecies from a fork
	vector<vector<cv::Point>> f_v;
	bool TConnections[ANZAHL][ANZAHL][4];   //temporary Connections
	std::vector<cv::Point> TLinks[ANZAHL][ANZAHL];  //temporary Links

	//temp prep
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < 4; i++) {
				TConnections[x][y][i] = Connections[x][y][i];
			}
			for (int i = 0; i < Links[x][y].size(); i++) {
				TLinks[x][y].push_back(Links[x][y][i]);
			}
		}
	}

	bool a = true;
	while (a) {
		vector<cv::Point> v;	//vertex
		get_fork_vertex(forks[pos], forks[pos], forks, TConnections, TLinks, v);
		if (v.size() > 1) {  //exit sign -> only one point (startingpoint / fork[i])
			f_v.push_back(v);
		}
		else {
			a = false;  //exit loop and go to next fork
		}
	}
	return f_v;
}

void get_fork_vertex(cv::Point pos, cv::Point startingFork, vector<cv::Point>& forks, bool(&TConnections)[ANZAHL][ANZAHL][4], std::vector<cv::Point>(&TLinks)[ANZAHL][ANZAHL], vector<cv::Point>& vertex) {
	bool vertexfin = false;									//vertex is finished
	vertex.push_back(cv::Point(pos.x, pos.y));				//save point

	if (vertex.size() > 1) {						//if you have already moved  (1 since there is always on vertx already part of the vector)
		for (int i = 0; i < forks.size(); i++) {
			if (forks[i] == pos) {					//if you reached a fork / your StartingFork / startingPoint
				vertexfin = true;
			}
		}
	}


	if (!vertexfin) {																					//WORK IN PROGRESS
		for (int i = 0; i < 4; i++) {
			if (TConnections[pos.x][pos.y][i] == true) {
				TConnections[pos.x][pos.y][i] = false;																							//deleat Connection
				TConnections[pos.x + (i % 2 == 0 ? 0 : (2 - i))][pos.y + (i % 2 == 0 ? (i - 1) : 0)][(i + 2) % 4] = false;						//deleat Connection of other point to current Point   (i % 2 == 0 ? (i - 1) : 0) -> -1,0,1,0 for 0,1,2,3
				get_fork_vertex(cv::Point(pos.x + (i % 2 == 0 ? 0 : (2 - i)), pos.y + (i % 2 == 0 ? (i - 1) : 0)), startingFork, forks, TConnections, TLinks, vertex);  //Modulo + inbuilt if-statement sollution to replace switch method
				vertexfin = true;		//if there is already a path to and from this tile (in other words you have reached this point) this point doesn't need to be computed further, hence aborting vertex now, since it will be already finished when the programm returns to this point
				break;
			}
		}
	}
	if (!vertexfin && TLinks[pos.x][pos.y].size() != 0) {  //if it still isn't true  (no Connections) and there are still Links
		cv::Point TPoint = TLinks[pos.x][pos.y][0];
		TLinks[pos.x][pos.y].erase(TLinks[pos.x][pos.y].begin());							//erase Point [0]
		for (int i = 0; i < TLinks[TPoint.x][TPoint.y].size(); i++) {
			if (TLinks[TPoint.x][TPoint.y][i] == cv::Point(pos.x, pos.y)) {
				TLinks[TPoint.x][TPoint.y].erase(TLinks[TPoint.x][TPoint.y].begin() + i);	 //erase point at linked to tile linking back to current point
				break;
			}
		}
		get_fork_vertex(TPoint, startingFork, forks, TConnections, TLinks, vertex);  //go to linked to tile
		vertexfin = true;
	}
	//if there are no connections/links and you haven't moved -> abort fork  (vertex.size() == 1)
	//if there are no connections or links and one hasn't reached the startPoint, another fork or the startingFork -> end of line	(since point has already been saved nothing needs to be done	
}

void create_vertecies(vector<vector<cv::Point>>& final_verts, cv::Point lastVertexEnd, cv::Point lastPosition, vector<vector<vector<cv::Point>>>& vertecies) {  //W
	for (int i = 0; i < final_verts.size(); i++) {
		cv::Point newlastPosition;
		if (final_verts[i][0] == lastVertexEnd || final_verts[i][final_verts[i].size() -1] == lastVertexEnd) {   //if first or last point of vertex connects
			if (final_verts[i][final_verts[i].size() - 1] == lastVertexEnd) {	//flip order if last one is the connecting one
				std::reverse(final_verts[i].begin(), final_verts[i].end());
			}
			if (lastPosition == cv::Point(-1, -1)) {  //if starting Point
				vector<vector<cv::Point>> t_v;  //temp Vector of Vector of Points (gets added to vertecies as new Vector in x-direction
				t_v.push_back(final_verts[i]);
				vertecies.push_back(t_v);
				newlastPosition = cv::Point(vertecies.size() - 1, lastPosition.y + 1);
			}
			else {
				if (vertecies[lastPosition.x].size() > lastPosition.y + 1) {  //push ->
					vector<vector<cv::Point>> t_v;  //temp Vector of Vector of Points (gets added to vertecies as new Vector in x-direction
					vector<cv::Point> empty;		//empty directory for spaces in Vertecies   if crashing try adding a value
					for (int j = 0; j < lastPosition.y + 1; j++) {  //fill with empties until reacing current hight
						t_v.push_back(empty);
					}
					t_v.push_back(final_verts[i]);
					vertecies.push_back(t_v);
					newlastPosition = cv::Point(vertecies.size() - 1, lastPosition.y + 1);
				}
				else {  //push  \/ 
					vertecies[lastPosition.x].push_back(final_verts[i]);
					newlastPosition = cv::Point(lastPosition.x, lastPosition.y + 1);
				}
			}
			cv::Point newLastVertexEnd;
			newLastVertexEnd = final_verts[i][final_verts[i].size() - 1];
			final_verts.erase(final_verts.begin() + i);
			i -= 1; //
			create_vertecies(final_verts, newLastVertexEnd, newlastPosition, vertecies);
		}
	}
}

void line_evaluation() {
	vector<cv::Point> empty;
	vector<cv::Point> destinations;
	end_line.clear();

	//--get-the-end-positions-------
	for (int i = 0; i < Vertecies.size(); i++) {
		destinations.push_back(Vertecies[i][Vertecies[i].size() - 1][Vertecies[i][Vertecies[i].size() - 1].size() - 1]);  //last point of each line
	}

	//--get-ideal-end-position------
	if (greendots[0] == cv::Point(0, 0) && greendots[1] == cv::Point(0, 0)) {  //drive straight
		vector<int> dists;
		for (int i = 0; i < destinations.size(); i++) {  //get distances between ideal endpoint and actual endpoints
			cv::Point da = cv::Point(2, 0);
			cv::Point db = destinations[i];
			dists.push_back(sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2)));
		}
		int minpos = 0;
		int mindist = dists[0];
		for (int i = 1; i < dists.size(); i++) {		 //get endpoint with smalest distance
			if (dists[i] < mindist) {
				mindist = dists[i];
				minpos = i;
			}
		}
		end_line.push_back(destinations[minpos]);
		get_line_in_Vertecies(Vertecies, end_line, cv::Point(minpos,Vertecies[minpos].size() - 1));

		//get line with designated end
	}
	if (greendots[0] != cv::Point(0, 0) || greendots[1] != cv::Point(0, 0)) {  //turn to that point
		vector<cv::Point> forks;			//number of fork					 //, [forkpos][forkexit1][forkexit2]...
		get_dest_forks(Vertecies,forks);							
		if (forks.size() > 0) {
			cv::Point da = (greendots[1] != cv::Point(0, 0)) ? greendots[1] : greendots[0];
			cv::Point db = DotsReal[forks[0].x][forks[0].y];
			int mindist = sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2));
			int fork = 0;
			for (int i = 1; i < forks.size(); i++) {
				cv::Point db = DotsReal[forks[i].x][forks[i].y];
				mindist = (mindist < sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2))) ? mindist : sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2));
				fork = i;
			}

			//destinations 
			float w_1 = 1.2;	//weight  1 & 2  1 => how important is x-dir   2 => how important is y-dir 
			float w_2 = 2;
			int likely = 0;
			if (DotsReal[forks[fork].x][forks[fork].y].x < (greendots[1] != cv::Point(0, 0)) ? greendots[1].x : greendots[0].x) {
				//left
		 		for (int i = 0; i < destinations.size(); i++) {
					cv::Point da = (greendots[1] != cv::Point(0, 0)) ? greendots[1] : greendots[0]; //entsprechende grüner punkt
					cv::Point db = DotsReal[destinations[i].x][destinations[i].y];
					cv::Point dc = DotsReal[destinations[likely].x][destinations[likely].y];

					if (destinations[i].x < destinations[likely].x * w_1 && sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2)) < sqrt(pow(da.x - dc.x, 2) + pow(da.y - dc.y, 2))) {  //if closer and maybe further left
						likely = i;
					}
				}
			}
			else {
				//right

				int likely = 0;
				for (int i = 0; i < destinations.size(); i++) {
					cv::Point da = (greendots[1] != cv::Point(0, 0)) ? greendots[1] : greendots[0]; //entsprechende grüner punkt
					cv::Point db = DotsReal[destinations[i].x][destinations[i].y];
					cv::Point dc = DotsReal[destinations[likely].x][destinations[likely].y];

					if (destinations[i].x > destinations[likely].x * w_1 && sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2)) < sqrt(pow(da.x - dc.x, 2) + pow(da.y - dc.y, 2))) {  //if closer and maybe further right
						likely = i;
					}
				}
			}
			end_line.push_back(destinations[likely]);
			get_line_in_Vertecies(Vertecies, end_line, cv::Point(likely, Vertecies[likely].size() - 1));
		}
		else {  // if greendot but no fork
			end_line.push_back(destinations[0]);
			get_line_in_Vertecies(Vertecies, end_line, cv::Point(0, Vertecies[0].size() - 1));


		}
		//get forks in Vertecies and endpoints of there vertecies
		//get dists between green points and verts
		//get spacial relation to closest fork
	}
	if (greendots[0] != cv::Point(0, 0) && greendots[1] != cv::Point(0, 0)) {  //180°
		//TURN
		end_line.push_back(cv::Point(-1, -1));  //-1,-1 => TURN
	}

	std::reverse(end_line.begin(), end_line.end());

	if (end_line.size() == 1) {  //incase there is only one point of the line (gap), double the only point to work with rest of programm
		end_line.push_back(end_line[0]);
	}
}

void get_line_in_Vertecies(vector<vector<vector<cv::Point>>>& vertecies, vector<cv::Point>& line, cv::Point currentPoint) {
	vector<cv::Point> empty;
	for (int i = vertecies[currentPoint.x][currentPoint.y].size() - 2; i >= 0; i--) {
		line.push_back(vertecies[currentPoint.x][currentPoint.y][i]);
	}
	if (currentPoint.y != 0) {  //if not jet reached end
		if (vertecies[currentPoint.x][currentPoint.y - 1] == empty) {  //if line is done
			get_line_in_Vertecies(vertecies, line, cv::Point(currentPoint.x - 1, currentPoint.y - 1));
		}
		else {
			get_line_in_Vertecies(vertecies, line, cv::Point(currentPoint.x, currentPoint.y - 1));
		}
	}
}

void get_dest_forks(vector<vector<vector<cv::Point>>>& vertecies, vector<cv::Point>& forks) {   
	vector<cv::Point> empty;
	bool next = false;
	for (int i = vertecies.size() - 1; i > 0; i--) {
		for (int j = vertecies[i].size() - 1; j > 0; j--) {
			if (vertecies[i][j] != empty) {
				next = false;
				if (j > 0) {
					if (vertecies[i][j - 1] == empty) {  //part of fork
						for (int a = 0; a < forks.size(); a++) {
							if (vertecies[i][j][0] == forks[a]) {
								next = true;
							}
						}
						if (!next) {
							forks.push_back(vertecies[i][j][0]);
						}
					}
				}
				else {		//if fork from startPoint
					if (i > 0) {
						for (int a = 0; a < forks.size(); a++) {
							if (vertecies[i][j][0] == forks[a]) {
								next = true;
							}
						}
						if (!next) {
							forks.push_back(vertecies[i][j][0]);
						}
					}
				}
			}
		}
	}
}

void find_heading() {
	if (end_line[0] != cv::Point(-1, -1)) {
		heading = DotsReal[end_line[1].x][end_line[1].y];
		if (heading.x > pic_cols / 2 + pic_cols / 6) { //right
			std::cout << "right" << std::endl;
		}
		else if (heading.x < pic_cols / 2 - pic_cols / 6) { //left
			std::cout << "left" << std::endl;
		}
		else {
			std::cout << "straight" << std::endl;
		}
	}
	else {
		std::cout << "180° Túrn" << std::endl;
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
	for (int i = 1; i < end_line.size(); i++) {
		cv::line(img_rgb, DotsReal[end_line[i-1].x][end_line[i - 1].y], DotsReal[end_line[i].x][end_line[i].y], cv::Scalar(0, 0, 255), 3);
	}

	for (int b = 0; b < 2; b++) {  // Green-Dot
		if (greendots[b] != cv::Point(0, 0)) {
			cv::drawContours(img_rgb, contours_green[b], index_green[b], cv::Scalar(0, 255, 0), 2, 8, cv::noArray(), INT_MAX);
			cv::circle(img_rgb, greendots[b], 2, cv::Scalar(30, 255, 30), 5);
		}
	}
	cv::circle(img_rgb, heading, 2, cv::Scalar(0, 255, 255), 5);
}

void show() {
	//übergebe Bilder an den Server
	cv::imshow("RGB", img_rgb);
	cv::imshow("GREEN", img_bin);
	cv::imshow("BinSW", img_binsw);
	cv::waitKey(30000000);
}

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
			if (DotsReal[x][y] != cv::Point(0, 0)) {
				int roiheight = ROIs[x][y].rows;
				int roiwidth = ROIs[x][y].cols;

				h = get_maxdimensions(x, y, true);  //true = height
				if (similar(h, maxh)) {  // if height is similar to max height
					for (int iy = roiheight - 1; iy >= 0; iy--) {
						a = 0;
						for (int ix = roiwidth - 1; ix >= 0; ix--) {
							if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
								a++;
							}
						}
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
			if (widths.size() % 2 == 0) {  // if even
				return (widths[widths.size() / 2] + widths[(widths.size() / 2) + 1]) / 2;
			}
			else {  //if odd
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

	if (values.size() % 2 == 0) {  // if even
		return (values[values.size() / 2] + values[(values.size() / 2) + 1]) / 2;
	}
	else {  //if odd
		return values[(values.size() + 1) / 2];
	}
}

int get_borderpos(int x, int y, int direction) {
	if (direction == 0) {//top
		for (int iy = 0; iy < ROIs[x][y].rows; iy++) {
			for (int ix = 0; ix < ROIs[x][y].cols; ix++) {
				if (ROIs[x][y].at<uchar>(iy, ix) == 255) {  // if black
					if (pointPolygonTest(contours_arr[x][y][index_arr[x][y]], cv::Point(ix, iy), false) > 0) {  //checks if point is in Contour
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

float get_median(vector<int>& v) {  //includes sort
	vector<int> holdvector;
	for (int i = 0; i < v.size(); i++) {
		holdvector.push_back(v[i]);
	}
	v_sort(holdvector);

	if (holdvector.size() % 2 == 0) {  // if even
		return (holdvector[holdvector.size() / 2] + holdvector[(holdvector.size() / 2) + 1]) / 2;
	}
	else {  //if odd
		return holdvector[(holdvector.size() + 1) / 2];
	}
}

float get_average(vector<int>& v) {
	int all = 0;
	for (int i = 0; i < v.size(); i++) {
		all += v[i];
	}
	return all / v.size();
}

bool similar(int a, int b) {
	int v = 2;
	if (a < b + v && b < a + v) {
		return true;
	}
	else {
		return false;
	}
}

float dist(cv::Point a, cv::Point b) {  //distance betwwen the DotsReal in image
	cv::Point da = DotsReal[a.x][a.y];
	cv::Point db = DotsReal[b.x][b.y];
	return sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2));
}

cv::Point get_fork(bool(&arr)[ANZAHL][ANZAHL]) {
	bool t = false;

	for (int y = 0; y < ANZAHL; y++) {
		for (int x = 0; x < ANZAHL; x++) {
			if (DotsReal[x][y] != cv::Point(0, 0) && arr[x][y] != 1) {	//if there is a point and it isn't already used
				int con = 0; 				 //all connections from Point
				for (int i = 0; i < 4; i++) {  //all Connections
					if (Connections[x][y][i]) { con++; }
				}
				con += Links[x][y].size();	 //all Links
				if (con > 2) {
					arr[x][y] = true;
					return cv::Point(x, y);
				}  //if fork -> return point
			}
		}
	}
	return cv::Point(5, 5); //no fork
}

void v_sort(vector<int>& v) {
	int c = 0; //c -> count
	for (int i = 1; i < v.size(); i++) {
		if (v[i - 1] > v[i]) {
			iter_swap(v.begin() + i - 1, v.begin() + i);
			c++;
		}
	}
	if (c == 0) {
		return;
	}
	else {
		v_sort(v);
	}
}