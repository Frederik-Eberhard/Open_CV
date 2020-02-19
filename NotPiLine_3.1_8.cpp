//============================================================================
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
cv::Mat img_blur;   //unused
cv::Mat img_roi;    //unused
cv::Mat img_bin;
cv::Mat img_hsv;
cv::Mat img_sw;     //unused




					/*
					cv::Mat img_edgy;
					int thrVal = 100;  //starting threshhold value
					int thrVal2 = 100;  //starting threshhold value
					int thrVal3 = 100;  //starting threshhold value
					int thrVal4 = 100;  //starting threshhold value


					cv::Mat img_sat;
					int alpha = 1;
					int beta = 10;
					*/
					/*
					std::vector<int> brightnesses;
					std::vector<cv::Mat> hsv_splitmat;  //Ein Vektor von Matritzen für Hue, Saturation und Value
					std::vector<cv::Point> differences;  //x-pos == difference in Value, y-pos == pos in brightnesses
					int blackvalue;*/
					/*
					std::vector<int> brightnesses;  //stores the brightnesses at current
					std::vector<cv::Mat> hsv_splitmat;  //Ein Vektor von Matritzen für Hue, Saturation und Value
					std::vector<int> differences;  //stores the differences of current to next
					std::vector<int> blacks;  //stores the blacks
					*/
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

cv::Point greendots[2];
std::vector<std::vector<cv::Point> > contours_green[2]; //für grün ( 2 für die möglichen zwei grünen Punkte )
int index_green[2];

cv::Point heading = cv::Point();

cv::Point startPoint;  //starting Point of Line


int linethickness = 0;
//const int extravalue = 5;  //extravalue + linthickness for linescheck()
const float percentage = 1.2;  //linethickness * percentage 7

void color_set();  //name is work in progress
void img_set();
void makeROIs();
void makeConturesandDots();
void lines();
void linescheck();
void find_begining();
bool repeat(int x, int y);
bool repeat(int x, int y, bool loop, cv::Point fork, vector<cv::Point>& loopvec);
void find_greendot();
void link_clean();
void get_startPoint(cv::Point& sP);	//gets the Startpoint of the Line
void loops();
//void get_loop(cv::Point fork, vector<cv::Point>&);
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
//vector<vector<cv::Point>> get_fork_loop_vertecies(vector<cv::Point>(&tempVertexLinks)[ANZAHL][ANZAHL], bool(&save_verts_at_fork)[ANZAHL][ANZAHL]);
//vector<cv::Point> get_save_vertex(cv::Point pos, vector<cv::Point>& vertex, vector<cv::Point>(&tempVertexLinks)[ANZAHL][ANZAHL], bool(&save_verts_at_fork)[ANZAHL][ANZAHL]);
void create_vertecies(vector<vector<cv::Point>>& final_verts, cv::Point lastVertexEnd, cv::Point lastPosition, vector<vector<vector<cv::Point>>>& vertecies);
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
	//	cv::namedWindow("Edges");
	cv::namedWindow("BinSW");

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

		color_set();
		//cv::cvtColor(img_rgb, img_edgy, CV_BGR2GRAY);

		std::cout << "Hallo3.5 " << std::endl;
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
		std::cout << "Hallo10 " << std::endl;
		find_greendot();
		std::cout << "Hallo11 " << std::endl;
		link_clean();
		get_startPoint(startPoint);
		loops();
		find_heading();
		std::cout << "Hallo12 " << std::endl;
		draw();
		std::cout << "Hallo13 " << std::endl;
		show();
		std::cout << "Hallo14 " << std::endl;
	}
	std::cout << "BYYYY " << std::endl;
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

void color_set() {
	int val = 0;
	int loc[] = { 0,255 };  // 0 = black  1 = white
	bool ready = false;
	for (int i = 0; i < 256; i++) {
		color_values[i] = 0;
	}

	std::cout << "Hallo3.1 " << std::endl;
	//RGB -> HSV
	cv::cvtColor(img_rgb, img_hsv, cv::COLOR_BGR2HSV);
	cv::split(img_hsv, hsv_splitmat);

	for (int y = 0; y < pic_rows; y++) {
		for (int x = 0; x < pic_cols; x++) {
			val = hsv_splitmat[2].at<uchar>(y, x);
			//std::cout << "val " << val << std::endl;
			color_values[val]++;
		}
	}
	for (int i = 0; i < 256; i++) {
		std::cout << i << ": " << color_values[i] << std::endl;
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
				std::cout << "i " << i << " loc " << loc[0] << std::endl;
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
				std::cout << "i " << i << " loc " << loc[1] << std::endl;
				break;
			}
		}
	}

	std::cout << "blackvalue " << (loc[0] + loc[1]) / 2 - 5 << std::endl;
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

			//			std::cout << "xmin " << xmin << std::endl;
			//			std::cout << "xmax " << xmax << std::endl;
			//			std::cout << "ymin " << ymin << std::endl;
			//			std::cout << "ymax " << ymax << std::endl;

			//			  X  ,  Y  ,   WIDTH  ,   HEIGHT
			cv::Rect roi(xmin, ymin, xmax - xmin, ymax - ymin);
			//			std::cout << "Hallo4.1 " << std::endl;
			ROIs[x][y] = img_binsw(roi);
			//			std::cout << "Hallo4.2 " << std::endl;
			cv::GaussianBlur(ROIs[x][y], ROIs[x][y], Size(5, 5), 0, 0);
			//			std::cout << "Hallo4.3 " << std::endl;
			cv::inRange(ROIs[x][y], cv::Scalar(0, 0, 0), cv::Scalar(50, 45, blackvalue), ROIs[x][y]);  //50 , 45 , 45
																									   //			std::cout << "Hallo4.4 " << std::endl;
			cv::bitwise_not(ROIs[x][y], ROIs[x][y]); //schwarz weiß tauschen
													 //			std::cout << "Hallo4.5 " << std::endl;
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
					if (grocont >(roisize * mingross)) { //wie viel des Felds soll die Kontur belegen, bevor sie gez�hlt wird
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
						if (similar(get_borderpos(x, y, 0), get_borderpos(x, y - 1, 2))) {  //check if they are touching
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
}	std::vector<cv::Point> loop;			 //saves all points of loop


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
std::cout << "LINKS" << Links[2][2][1][1] << std::endl;	std::vector<cv::Point> loop;			 //saves all points of loop


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

bool repeat(int x, int y) {  //overloading for linefind
	std::vector<cv::Point> vec;				//just temp vector without use
	repeat(x, y, false, cv::Point(0, 0), vec);
	return true;
}

bool repeat(int x, int y, bool loop, cv::Point fork, vector<cv::Point>& loopvec) {
	std::cout << "Hallo9.1.0 " << TempLinks[x][y].max_size() << " " << x << " " << y << std::endl;
	temppoint = cv::Point(0, 0);
	//--Links----------
	if (TempLinks[x][y].size() != 0) {
		std::cout << "Hallo9.1.1 " << std::endl;
		int lsize = TempLinks[x][y].size();  //linkssize
											 //for (int i = 0; i < lsize; i++) {
		while (TempLinks[x][y].size() > 0) {
			std::cout << "Hallo9.1.2 " << " " << TempLinks[x][y].size() << std::endl;
			linesstart[amount] = DotsReal[x][y];
			std::cout << "Hallo9.1.2.1 " << TempLinks[x][y][0] << std::endl;
			temppoint = TempLinks[x][y][0];  //always the first, since it is deleated later on and there by prevents loops
			std::cout << "Hallo9.1.2.2 " << std::endl;
			linesend[amount] = DotsReal[temppoint.x][temppoint.y];
			std::cout << "Hallo9.1.3 " << std::endl;
			//remove link to current from linked to node--------
			if (!loop) {
				for (int j = 0; j < TempLinks[temppoint.x][temppoint.y].size(); j++) {
					if (TempLinks[temppoint.x][temppoint.y][j] == cv::Point(x, y)) {
						std::cout << "Hallo9.1.4 " << std::endl;
						TempLinks[temppoint.x][temppoint.y].erase(TempLinks[temppoint.x][temppoint.y].begin() + j);
					}
				}
			}

			std::cout << "Hallo9.1.5 " << std::endl;
			amount++;
			std::cout << "Hallo9.1.6 " << std::endl;
			TempLinks[x][y].erase(TempLinks[x][y].begin());  //deleating current
			std::cout << "Hallo9.1.7 " << std::endl;
			if (loop) {
				if (temppoint.x == fork.x && temppoint.y == fork.y) {  //back at beginning -> LOOP
					loopvec.push_back(cv::Point(x, y));				 //push_back current point
					return true;									 //exit recursion
				}
				if (repeat(temppoint.x, temppoint.y, true, fork, loopvec)) {  //still runs recursive
					loopvec.push_back(cv::Point(x, y));				 //push_back current point
					return true;									//exit recursion
				}

			}
			else {
				repeat(temppoint.x, temppoint.y);
			}
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
				amount++;
				if (!loop) {
					TempConnections[x][y - 1][2] = false;
					repeat(x, y - 1);
				}
				else {
					if (temppoint.x == fork.x && temppoint.y == fork.y) {  //back at beginning -> LOOP
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;									 //exit recursion
					}
					if (repeat(x, y - 1, true, fork, loopvec)) {
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;
					}
				}
			}
			if (i == 1) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x + 1][y];
				amount++;
				if (!loop) {
					TempConnections[x + 1][y][3] = false;
					repeat(x + 1, y);
				}
				else {
					if (temppoint.x == fork.x && temppoint.y == fork.y) {  //back at beginning -> LOOP
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;									 //exit recursion
					}
					if (repeat(x + 1, y, true, fork, loopvec)) {
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;
					}
				}
			}
			if (i == 2) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x][y + 1];
				amount++;
				if (!loop) {
					TempConnections[x][y + 1][0] = false;
					repeat(x, y + 1);
				}
				else {
					if (temppoint.x == fork.x && temppoint.y == fork.y) {  //back at beginning -> LOOP
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;									 //exit recursion
					}
					if (repeat(x, y + 1, true, fork, loopvec)) {
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;
					}
				}
			}
			if (i == 3) {
				TempConnections[x][y][i] = false;
				linesstart[amount] = DotsReal[x][y];
				linesend[amount] = DotsReal[x - 1][y];
				amount++;
				if (!loop) {
					TempConnections[x - 1][y][1] = false;
					repeat(x - 1, y);
				}
				else {
					if (temppoint.x == fork.x && temppoint.y == fork.y) {  //back at beginning -> LOOP
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;									 //exit recursion
					}
					if (repeat(x - 1, y, true, fork, loopvec)) {
						loopvec.push_back(cv::Point(x, y));				 //push_back current point
						return true;
					}
				}
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
	std::cout << "Hallo10.1 " << std::endl;
	//---größte-Konturen-(2)---
	double grocont = 0;  //größte Kontur
	for (int j = 0; j < 2; j++) {
		for (unsigned int i = 0; i < contours_green[j].size(); i++) {
			cv::Moments m = cv::moments(contours_green[j][i]);
			//cv::circle
			if (m.m00 > grocont && i != index_green[0]) {  // sodass es die größte ist, die aber nicht die erste ist ( 1. grüner Punkt )
				grocont = m.m00;
				std::cout << "Hallo10.2 " << grocont << std::endl;
				greendots[j].x = m.m10 / m.m00;
				greendots[j].y = m.m01 / m.m00;
				index_green[j] = i;
			}
		}
		if (index_green[j] != -1) {
			std::cout << "Hallo10.2.1 " << grocont << " " << contours_green[j].size() << std::endl;
			cv::Moments a = cv::moments(contours_green[j][index_green[j]]);  //stoping the second point being much smaller than the first
			std::cout << "Hallo10.2.2 " << std::endl;
			grocont = a.m00 / 2;
			std::cout << "Hallo10.2.3 " << grocont << std::endl;
		}
	}
	std::cout << "Hallo10.3 " << std::endl;
	//---Kontrolle-ob-Punkt----
	for (int i = 0; i < 2; i++) {
		if (greendots[i] != cv::Point(0, 0)) {
			std::cout << "Hallo10.3.1 " << std::endl;
			int count_black = 0;
			cv::Moments m = cv::moments(contours_green[i][index_green[i]]);
			double height = sqrt(m.m00);
			std::cout << "Hallo10.3.2 " << m.m00 << "  " << height << std::endl;
			for (int iy = 0; iy < height; iy++) {
				int x = greendots[i].x;
				int y = greendots[i].y - (height / 2) - iy;
				std::cout << "Hallo10.3.2.1 " << x << " " << y << std::endl;
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

/*
void loops() {  //WORK IN PROGRESS -> void for removing loops in Image
	std::cout << "loops" << std::endl;
	bool usedforks[ANZAHL][ANZAHL];  		 //all used forks
	for (int i = 0; i < ANZAHL; i++) {
		for (int j = 0; j < ANZAHL; j++) {
			usedforks[i][j] = false;
		}
	}
											 //repeating starts here
	cv::Point fork = get_fork(usedforks);  	 //gets a fork
	std::cout << "new_fork " << fork << std::endl;
	if (fork != cv::Point(5, 5)) {
		std::vector<cv::Point> loop;			 //saves all points of loop
		get_loop(fork, loop);
	}	//gets the loop belonging to fork
}

void get_loop(cv::Point fork, vector<cv::Point>& loop) {
	repeat(fork.x, fork.y, true, fork, loop);
	std::cout << "LOOP: " << loop << std::endl;
}*/

void link_clean() {												//important for fork_detection     ---POSSIBLE BUG WHEN SORTING VECTOR  -> CRASHING---
	for (int x = 0; x < ANZAHL; x++) {
		for (int y = 0; y < ANZAHL; y++) {
			for (int i = 0; i < Links[x][y].size(); i++) {
				if (Links[x][y][i] == cv::Point(x, y - 1)|| Links[x][y][i] == cv::Point(x + 1, y) || Links[x][y][i] == cv::Point(x, y + 1) || Links[x][y][i] == cv::Point(x - 1, y)) {  //removes links that are also connections
					Links[x][y].erase(Links[x][y].begin() + i);
					i--;
				}
				for (int j = 0; j < Links[x][y].size(); j++) {   //removes links that are double
					if (j != i && Links[x][y][i] == Links[x][y][j]) {
						Links[x][y].erase(Links[x][y].begin() + i);
						i--;
					}
				}
			}
		}
	}
}

void get_startPoint(cv::Point& sP) {
	sP = cv::Point(1, 4);				//TEMPORARY SOLLUTION -> NEEDS TO BE REWORKED
}

void loops() {
	std::cout << "Hallo11: loops" << std::endl;
	vector<cv::Point> forks;							//all pos of forks in img (start is always a fork!)
	forks = get_forks();


	//--removing-loops-part--------------------
	vector<vector<cv::Point>> forks_from_fork;			//all forks that a fork connects to 
	vector<vector<cv::Point>> final_verts;				//all final vertecies

	for (int i = 0; i < forks.size(); i++) {
		vector<vector<cv::Point>> fork_vertecies;		//all vertecies from this fork
		fork_vertecies = get_fork_verts(forks, i);
		vector<cv::Point> t_v;							//temp storage to forks that this fork connects to


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
				std::cout << "f_v.size " << fork_vertecies.size() << std::endl;
				for (int a = 0; a < fork_vertecies.size(); a++) {
					if (a != j && fork_vertecies[a][fork_vertecies[a].size() - 1] == fork_vertecies[j][fork_vertecies[j].size() - 1]) {  //if last point of two vertecies are the same -> simple-loop    -> remove longer connection
						std::cout << "j " << j << "  a " << a << " Simple-Loop" << std::endl;
						int num = 0;
						if (fork_vertecies[a].size() > fork_vertecies[j].size()) {
							num = a;		//a gets deleted
						}
						else {
							num = j;		//j gets deleted
						}
						std::cout << "longer one: " << num << "  " << fork_vertecies[num].size() << std::endl;
						for (int b = 1; b < fork_vertecies[num].size() - 1; b++) {  // first of vertex is fork and last is other fork, so dont need to deleat them
							std::cout << "remove b: " << b << std::endl;
							int x = fork_vertecies[num][b].x;
							int y = fork_vertecies[num][b].y;
							std::cout << "x " << x << " y " << y << std::endl;
							DotsReal[x][y] = cv::Point(0, 0);
							std::cout << "removed DotsReal" << std::endl;
							for (int c = 0; c < 4; c++) {
								if (Connections[x][y][c] == true) {
									Connections[x][y][c] = false;																							//delete Connection
									Connections[x + (c % 2 == 0 ? 0 : (2 - c))][y + (c % 2 == 0 ? (c - 1) : 0)][(c + 2) % 4] = false;						//delete Connection of other point to current Point   (i % 2 == 0 ? (i - 1) : 0) -> -1,0,1,0 for 0,1,2,3
								}
							}
							std::cout << "removed Connections" << std::endl;
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
							std::cout << "removed Links  " << Links[x][y].size() << " " << Connections[x][y][1] << std::endl;
						}
						fork_vertecies.erase(fork_vertecies.begin() + num);
						std::cout << "f_v.size " << fork_vertecies.size() << std::endl;

						if (num == j) {
							j -= 1;			//to not skipp a vertex 
						}
						done = true;
					}
					if (done) { break; }
				}

				//--complex-loop---------------------------    -> Data storage Cleanup for end Data storage neccessary!


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


				//--standard-vertex------------------------
				done = true;
			}

		}

		forks_from_fork.push_back(t_v);														 //push back all new forks that are being connected to	


		for (int j = 0; j < fork_vertecies.size(); j++) {  //saving final Vertecies
			final_verts.push_back(fork_vertecies[j]);
		}

		std::cout << "Vertecies of fork " << i << std::endl;
		for (int j = 0; j < fork_vertecies.size(); j++) {
			std::cout << j << "  " << fork_vertecies[j] << std::endl;
		}

		std::cout << "forks_from_fork" << std::endl;
		for (int j = 0; j < forks_from_fork.size(); j++) {
			std::cout << j << "  " << forks_from_fork[j] << std::endl;
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
	std::cout << "Vertecies done" << std::endl;

	//--printing-----------------------------
	std::cout << "forks: " << std::endl;
	for (int i = 0; i < forks.size(); i++) {
		std::cout << i << "  " << forks[i] << std::endl;
	}
	std::cout << "final_verts: " << std::endl;
	for (int i = 0; i < final_verts.size(); i++) {
		std::cout << i << "  " << final_verts[i] << std::endl;
	}
	std::cout << "Vertecies" << std::endl;
	for (int i = 0; i < Vertecies.size(); i++) {
		std::cout << "i " << i << std::endl;
		for (int j = 0; j < Vertecies[i].size(); j++) {
			std::cout << "j " << j << "  " << Vertecies[i][j] << std::endl;
		}
	}
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

	std::cout << "Hallo11.1.1" << std::endl;
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

	//printing
	/*
	for (int i = 0; i < ANZAHL; i++) {
		for (int j = 0; j < ANZAHL; j++) {
			for (int ij = 0; ij < 4; ij++) {
				std::cout << "x " << i << " y " << j << " TC: " << TConnections[i][j][ij] << std::endl;
			}
			for (int ij = 0; ij < TLinks[i][j].size(); ij++) {
				std::cout << "x " << i << " y " << j << " Links: " << TLinks[i][j][ij] << std::endl;
			}
		}
	}*/

	std::cout << "Hallo11.1.2" << std::endl;
	bool a = true;
	while (a) {
		vector<cv::Point> v;	//vertex
		std::cout << "Hallo11.1.3" << std::endl;
		get_fork_vertex(forks[pos], forks[pos], forks, TConnections, TLinks, v);
		std::cout << "Hallo11.1.4" << std::endl;
		if (v.size() > 1) {  //exit sign -> only one point (startingpoint / fork[i])
			f_v.push_back(v);
			std::cout << "Hallo11.1.5" << std::endl;
		}
		else {
			a = false;  //exit loop and go to next fork
			std::cout << "Hallo11.1.6" << std::endl;
		}
	}
	std::cout << "Hallo11.1.7" << std::endl;

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

void create_vertecies(vector<vector<cv::Point>>& final_verts, cv::Point lastVertexEnd, cv::Point lastPosition, vector<vector<vector<cv::Point>>>& vertecies) {  //WORK IN PROGRESS!!
	std::cout << "Hallo11.5.1" << std::endl;
	for (int i = 0; i < final_verts.size(); i++) {
		std::cout << "Hallo11.5.2" << std::endl;
		cv::Point newlastPosition;
		if (final_verts[i][0] == lastVertexEnd || final_verts[i][final_verts[i].size() -1] == lastVertexEnd) {   //if first or last point of vertex connects
			std::cout << "Hallo11.5.3" << std::endl;
			if (final_verts[i][final_verts[i].size() - 1] == lastVertexEnd) {	//flip order if last one is the connecting one
				std::cout << "reverse" << std::endl;
				std::reverse(final_verts[i].begin(), final_verts[i].end());
			}
			std::cout << "Hallo11.5.4" << std::endl;
			if (lastPosition == cv::Point(-1, -1)) {  //if starting Point
				vector<vector<cv::Point>> t_v;  //temp Vector of Vector of Points (gets added to vertecies as new Vector in x-direction
				t_v.push_back(final_verts[i]);
				vertecies.push_back(t_v);
				newlastPosition = cv::Point(vertecies.size() - 1, lastPosition.y + 1);
			}
			else {
				std::cout << "Hallo11.5.4.1" << lastPosition << "  " << vertecies.size() << std::endl;
				if (vertecies[lastPosition.x].size() > lastPosition.y + 1) {  //push ->
					std::cout << "Hallo11.5.4.2" << std::endl;
					vector<vector<cv::Point>> t_v;  //temp Vector of Vector of Points (gets added to vertecies as new Vector in x-direction
					vector<cv::Point> empty;		//empty directory for spaces in Vertecies   if crashing try adding a value
					std::cout << "Hallo11.5.4.3" << std::endl;
					for (int j = 0; j < lastPosition.y; j++) {  //fill with empties until reacing current hight
						t_v.push_back(empty);
					}
					std::cout << "Hallo11.5.4.4" << std::endl;
					t_v.push_back(final_verts[i]);
					std::cout << "Hallo11.5.4.5" << std::endl;
					vertecies.push_back(t_v);
					newlastPosition = cv::Point(vertecies.size() - 1, lastPosition.y + 1);
					std::cout << "Hallo11.5.4.6" << std::endl;
				}
				else {  //push  \/ 
					std::cout << "Hallo11.5.4.7" << std::endl;
					vertecies[lastPosition.x].push_back(final_verts[i]);
					std::cout << "Hallo11.5.4.8" << std::endl;
					newlastPosition = cv::Point(lastPosition.x, lastPosition.y + 1);
					std::cout << "Hallo11.5.4.9" << std::endl;
				}
			}
			std::cout << "Hallo11.5.5" << std::endl;
			std::cout << "Hallo11.5.6" << std::endl;
			cv::Point newLastVertexEnd;
			newLastVertexEnd = final_verts[i][final_verts[i].size() - 1];
			std::cout << "Hallo11.5.7" << std::endl;
			final_verts.erase(final_verts.begin() + i);
			i -= 1; //
			std::cout << "new vert" << std::endl;
			create_vertecies(final_verts, newLastVertexEnd, newlastPosition, vertecies);
			std::cout << "Hallo11.5.8" << std::endl;

		}
	}
}

/*
void loops() {
	std::cout << "Hallo11: loops" << std::endl;
	vector<cv::Point> forks;							 //all pos of forks in img
	forks = get_forks();
	std::cout << "Hallo11.1" << std::endl;
	vector<vector<cv::Point>> fork_verts;				 //all verts(routes from verts), all points of vert, point    (save the entire route, but only account for start and end)
	fork_verts = get_fork_verts(forks);
	std::cout << "Hallo11.2" << std::endl;

	bool  save_verts_at_fork[ANZAHL][ANZAHL];			 //save vertecies that are connected to a fork / point
	vector<vector<cv::Point>> final_verts;				 //vertecies that will stay / are safe
	vector<cv::Point> points_to_remove;				     //points that need to be removed
	vector<cv::Point> tempVertexLinks[ANZAHL][ANZAHL];	 //vertecies transverted to links for mazesolving-algorithm


	cout << "fork_verts.size " << fork_verts.size() << endl;

	//remove self loops
	std::cout << "fork_vertecies: " << std::endl;
	for (int i = 0; i < fork_verts.size(); i++) {
		std::cout << "vertex " << i << ": " << fork_verts[i] << std::endl;
	}

	for (int i = 0; i < fork_verts.size(); i++) {
		if (fork_verts[i][0] == fork_verts[i][fork_verts[i].size() - 1]) {  //if first and last Point of the vertex are the same (loop from fork to same fork)
			//float dist1 = dist(fork_verts[i][0], fork_verts[i][1]);
			//float dist2 = dist(fork_verts[i][fork_verts[i].size() - 1], fork_verts[i][fork_verts[i].size() - 2]);
			//if (dist1 > dist2) {  //remove last point (remove furthest point)
			//	points_to_remove.push_back(fork_verts[i][1]);
			//	fork_verts[i].erase(fork_verts[i].begin() + 1);
			//}
			//else {
			//	points_to_remove.push_back(fork_verts[i][fork_verts[i].size() - 2]);
			//	fork_verts[i].erase(fork_verts[i].begin() + fork_verts[i].size() - 2);
			//}
			fork_verts[i].erase(fork_verts[i].begin() + fork_verts[i].size() - 1);  //remove double fork, since loop has been removed
		}
	}

	
	//remove save verts (maybe marking the ends of the verts as destinations) -> probably best to keep the verts for later use (e.g. the final route analysation with the green points)
	
	for (int i = 0; i < fork_verts.size(); i++) {
		bool notsafe = false;
		for (int j = 0; j < forks.size(); j++) {
			if (fork_verts[i][fork_verts[i].size() - 1] == forks[j]) {
				notsafe = true;
				break;
			}
		}
		if (!notsafe) {  //in other words if safe
			final_verts.push_back(fork_verts[i]);
			fork_verts.erase(fork_verts.begin() + i);  //deleat safe vertex form fork_vertex
		}
	}

	cout << "fork_verts.size " << fork_verts.size() << endl;


	//mark amount of save vetecies per fork / point
	for (int i = 0; i < final_verts.size(); i++) {
		save_verts_at_fork[final_verts[i][0].x][final_verts[i][0].y] = true;  //add 1 to the point, where the vertex starts
	}

	//transvert vertecies to links for mazesolving
	for (int i = 0; i < fork_verts.size(); i++) {
		tempVertexLinks[fork_verts[i][0].x][fork_verts[i][0].y].push_back(fork_verts[i][fork_verts[i].size() - 1]);  //adds a link from current poit to last point of vertex
		tempVertexLinks[fork_verts[i][fork_verts[i].size() - 1].x][fork_verts[i][fork_verts[i].size() - 1].y].push_back(fork_verts[i][0]);  //adds a link from last point of vertex to curent point
	}

	//remove loops between forks
	vector<vector<cv::Point>> temp_verts;  //stores the vertecies that connect the exits / start to each other
	temp_verts = get_fork_loop_vertecies(tempVertexLinks, save_verts_at_fork);
	
	cout << "temp_verts.size" << temp_verts.size() << endl;
	
	for (int i = 0; i < temp_verts.size(); i++) {  //add temp_verts to final_verts
		final_verts.push_back(temp_verts[i]);
	}

	/*
	cout << "final_verts.size" << final_verts.size() << endl;
	cout << "Vertecies.size()" << Vertecies.size() << endl;

	sort_vertecies(final_verts, startPoint, cv::Point(-1, -1), Vertecies);  //sort Verteies and stores it in "Vertecies"  | final_verts, LastVertexEnd, LastPosition, Vertecies

	cout << "Vertecies.size()" << Vertecies.size() << endl;

	
	
	//vector<vector<cv::Point>> temp_final_verts;  //stores the vertecies in correct order
	
	//sorted adding of temp_verts into final_verts
	//for (int i = 0; i < final_verts.size(); i++) {
	//	if (final_verts[i][final_verts[i].size() - 1] == startPoint) {  // 1. find startPoint of the Line
	//		temp_final_verts.push_back(final_verts[i]);
	//		final_verts.erase(final_verts.begin() + i);
	//	}
	//}
	


	//printing
	std::cout << "forks: " << forks << std::endl;
	for (int i = 0; i < forks.size(); i++) {
		std::cout << "point: " << forks[i] << std::endl;
		std::cout << "all links" << std::endl;
		for (int j = 0; j < Links[forks[i].x][forks[i].y].size(); j++) {
			std::cout << Links[forks[i].x][forks[i].y][j] << std::endl;
		}
		std::cout << "all connections" << std::endl;
		for(int j = 0; j < 4; j++){
			std::cout << Connections[forks[i].x][forks[i].y][j] << std::endl;
		}
	}
	std::cout << "fork_vertecies: " << std::endl;
	for (int i = 0; i < fork_verts.size(); i++) {
		std::cout << "vertex " << i << ": " << fork_verts[i] << std::endl;
	}
	std::cout << "Vertecies" << std::endl;
	for (int i = 0; i < Vertecies.size(); i++) {
		for (int j = 0; j < Vertecies[i].size(); j++) {
			std::cout << "x " << i << "  y " << j << std::endl;
			std::cout << "Vstart " << Vertecies[i][j][0] << std::endl;
			std::cout << "Vend   " << Vertecies[i][j][Vertecies[i][j].size() - 1] << std::endl;
		}
	}


}

vector<cv::Point> get_forks() {
	vector<cv::Point> f;
	cv::Point sP = startPoint;
	for (int y = 0; y < ANZAHL; y++) {
		for (int x = 0; x < ANZAHL; x++) {
			int num = 0;
			for (int i = 0; i < 4; i++) {
				num += Connections[x][y][i] ? 1 : 0;
			}
			num += Links[x][y].size();
			if (num > 2) {  //if fork
				f.push_back(cv::Point(x, y));
			}
		}
	}
	return f;
}

vector<vector<cv::Point>> get_fork_verts(vector<cv::Point>& forks) {
	vector<vector<cv::Point>> f_v;
	bool TConnections[ANZAHL][ANZAHL][4];   //temporary Connections
	std::vector<cv::Point> TLinks[ANZAHL][ANZAHL];  //temporary Links

	cv::Point sP = startPoint;

	std::cout << "Hallo11.1.1" << std::endl;
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

	for (int i = 0; i < ANZAHL; i++) {
		for (int j = 0; j < ANZAHL; j++) {
			for (int ij = 0; ij < 4; ij++) {
				std::cout << "x " << i << " y " << j << " TC: " << TConnections[i][j][ij] << std::endl;
			}
			for (int ij = 0; ij < TLinks[i][j].size(); ij++) {
				std::cout << "x " << i << " y " << j << " Links: " << TLinks[i][j][ij] << std::endl;
			}
		}
	}

	std::cout << "Hallo11.1.2" << std::endl;

	for (int i = 0; i < forks.size(); i++) {
		bool a = true;
		while (a) {
			vector<cv::Point> v;	//vertex
			std::cout << "Hallo11.1.3" << std::endl;
		    get_fork_vertex(forks[i],forks[i],forks,sP,TConnections,TLinks,v);
			std::cout << "Hallo11.1.4" << std::endl;
			if (v.size() > 1) {  //exit sign -> only one point (startingpoint / fork[i])
				f_v.push_back(v);
				std::cout << "Hallo11.1.5" << std::endl;
			}
			else {
				a = false;  //exit loop and go to next fork
				std::cout << "Hallo11.1.6" << std::endl;
			}
		}
		std::cout << "Hallo11.1.7" << std::endl;
	}

	return f_v;
}

void get_fork_vertex(cv::Point pos, cv::Point startingFork, vector<cv::Point>& forks, cv::Point sP, bool(&TConnections)[ANZAHL][ANZAHL][4], std::vector<cv::Point>(&TLinks)[ANZAHL][ANZAHL], vector<cv::Point>& vertex) {
	bool vertexfin = false;									//vertex is finished
	vertex.push_back(cv::Point(pos.x, pos.y));				//save point

	if (vertex.size() > 1) {						//if you have already moved  (1 since there is always on vertx already part of the vector)
		for (int i = 0; i < forks.size(); i++) {
			if (forks[i] == pos) {					//if you reached a fork/ your StartingFork
				vertexfin = true;
			}
		}
		if (sP == pos) {					//if you reached the Start (incase start is a fork and you start on Startpoint)
			vertexfin = true;
		}
	}


	if (!vertexfin) {
		for (int i = 0; i < 4; i++) { 
			if (TConnections[pos.x][pos.y][i] == true) {																		
				TConnections[pos.x][pos.y][i] = false;																			//deleat Connection
				//TConnections[pos.x + ((3 - i - 2) % 2) - 1][pos.y + ((i - 2) % 2)- 1][(i + 2) % 4] = false;									//deleat Connection of other point to current Point   ((i - 2) % 2)- 1 -> -1,0,1,0 for 0,1,2,3  <- doesn't Work
				TConnections[pos.x + (i % 2 == 0 ? 0 : (2 - i))][pos.y + (i % 2 == 0 ? (i - 1) : 0)][(i + 2) % 4] = false;						//deleat Connection of other point to current Point   (i % 2 == 0 ? (i - 1) : 0) -> -1,0,1,0 for 0,1,2,3
				get_fork_vertex(cv::Point(pos.x + (i % 2 == 0 ? 0 : (2 - i)), pos.y + (i % 2 == 0 ? (i - 1) : 0)), startingFork, forks, sP, TConnections, TLinks, vertex);  //Modulo + inbuilt if-statement sollution to replace switch method
				vertexfin = true;		//if there is already a path to and from this tile (in other words you have reached this point) this point doesn't need to be coputed further, hence aborting vertex now, since it will be already finished when the programm returns to this point
				break;
			}
		}
	}
	if (!vertexfin && TLinks[pos.x][pos.y].size() != 0) {  //if it still isn't true  (no Connections) and there are still Links
		cv::Point TPoint = TLinks[pos.x][pos.y][0];
		TLinks[pos.x][pos.y].erase(TLinks[pos.x][pos.y].begin());							//erase Point [0]
		for (int i = 0; i < TLinks[TPoint.x][TPoint.y].size(); i++) {
			if (TLinks[TPoint.x][TPoint.y][i] == cv::Point(pos.x,pos.y)) {
				TLinks[TPoint.x][TPoint.y].erase(TLinks[TPoint.x][TPoint.y].begin() + i);	 //erase point at linked to tile linking back to current point
				break;
			}
		}
		get_fork_vertex(TPoint, startingFork, forks, sP, TConnections, TLinks, vertex);  //go to linked to tile
		vertexfin = true;
	}
	//if there are no connections/links and you haven't moved -> abort fork  (vertex.size() == 1)
	//if there are no connections or links and one hasn't reached the startPoint, another fork or the startingFork -> end of line	(since point has already been saved nothing needs to be done	
}

vector<vector<cv::Point>> get_fork_loop_vertecies(vector<cv::Point>(&tempVertexLinks)[ANZAHL][ANZAHL] , bool(&save_verts_at_fork)[ANZAHL][ANZAHL]) {
	vector<vector<cv::Point>> t_v;
	for (int i = 0; i < ANZAHL; i++) {
		for (int j = 0; j < ANZAHL; j++) {
			if (tempVertexLinks[i][j].size() > 0 && save_verts_at_fork[i][j] == true) {  //if there are links from this point and a connection to a save vertex
				vector<cv::Point> t_v2;
				t_v.push_back(get_save_vertex(cv::Point(i,j),t_v2,tempVertexLinks,save_verts_at_fork));
				save_verts_at_fork[i][j] == false;
			}
		}
	}
	return  t_v;
}

vector<cv::Point> get_save_vertex(cv::Point pos, vector<cv::Point>& vertex, vector<cv::Point>(&tempVertexLinks)[ANZAHL][ANZAHL], bool(&save_verts_at_fork)[ANZAHL][ANZAHL]) {
	vertex.push_back(cv::Point(pos.x, pos.y));				//save point
	bool done = false;
	//if this is a fork with save points and you have moved
	if (save_verts_at_fork[pos.x][pos.y] == true && vertex.size() > 1) {
		done = true;
	}
	
	if (!done) {
		cv::Point tp = tempVertexLinks[pos.x][pos.y][0];		//temp save linkedto point
		tempVertexLinks[pos.x][pos.y].erase(tempVertexLinks[pos.x][pos.y].begin());  //remove link from current point to linkedto point
		for (int j = 0; j < tempVertexLinks[tp.x][tp.y].size(); j++) {				 //remove link back to current point
			if (tempVertexLinks[tp.x][tp.y][j] == cv::Point(pos.x, pos.y)) {
				tempVertexLinks[tp.x][tp.y].erase(tempVertexLinks[tp.x][tp.y].begin() + j);
			}
		}
		get_save_vertex(tp, vertex, tempVertexLinks, save_verts_at_fork);
	}

	return vertex;
}


//vector<vector<cv::Point>> sort_vertecies(vector<vector<cv::Point>>& final_verts, vector<vector<cv::Point>>& temp_verts, int num) {  //WORK IN PROGRESS!!
//	vector<cv::Point> temp_vertex;
//	if (num == 0) {
//		for (int i = 0; i < final_verts.size(); i++) {
//			if (final_verts[i][final_verts[i].size() - 1] == startPoint) {  //1. if last point of vetrex is the Start, this is the first vertex of final_verts
//				temp_vertex = final_verts[0];
//				final_verts[0] = final_verts[i];
//				final_verts[i] = temp_vertex;
//			}
//		}
//	}
//	else {
//		for (int i = num; i < final_verts.size(); i++) {
//			if (final_verts[num - 1][final_verts[i].size() - 1] == final_verts[i][0]) {  //2. if the first point of vertex is the same as last of previous
//				temp_vertex = final_verts[num];
//				final_verts[num] = final_verts[i];
//				final_verts[i] = temp_vertex;
//			}
//		}
//	}
//}

void sort_vertecies(vector<vector<cv::Point>>& final_verts, cv::Point lastVertexEnd, cv::Point lastPosition, vector<vector<vector<cv::Point>>>& vertecies) {  //WORK IN PROGRESS!!
	for (int i = 0; i < final_verts.size(); i++) {
		cv::Point newlastPosition;
		if (final_verts[i][0] == lastVertexEnd) {
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
					for (int j = 0; j < lastPosition.y; j++) {  //fill with empties until reacing current hight
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
		}
		cv::Point newLastVertexEnd;
		newLastVertexEnd = final_verts[i][final_verts[i].size() - 1];
		final_verts.erase(final_verts.begin() + i);
		std::cout << "new vert" << std::endl;
		sort_vertecies(final_verts, newLastVertexEnd, newlastPosition, vertecies);
	}
}*/

void find_heading() {
	std::cout << "HEADING" << linesstart[0] << " " << linesend[0] << std::endl;
	heading = cv::Point((linesstart[0].x - linesend[0].x) / 2 + linesend[0].x, (linesstart[0].y - linesend[0].y) / 2 + linesend[0].y);
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
	cv::circle(img_rgb, heading, 2, cv::Scalar(0, 255, 255), 5);
}

void show() {
	//übergebe Bilder an den Server
	cv::imshow("RGB", img_rgb);
	cv::imshow("GREEN", img_bin);
	//	cv::imshow("Edges", img_edgy);
	cv::imshow("BinSW", img_binsw);
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
			if (DotsReal[x][y] != cv::Point(0, 0)) {
				int roiheight = ROIs[x][y].rows;
				int roiwidth = ROIs[x][y].cols;

				h = get_maxdimensions(x, y, true);  //true = height
				if (similar(h, maxh)) {  // if height is similar to max height
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

	std::cout << "holdvector : ";
	for (int i = 0; i < holdvector.size(); i++) {
		std::cout << " " << holdvector[i];
	}
	std::cout << std::endl;

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

float dist(cv::Point a, cv::Point b) {  //distance betwwen the DotsReal in image
	cv::Point da = DotsReal[a.x][a.y];
	cv::Point db = DotsReal[b.x][b.y];
	std::cout << "a " << a << "da " << da << " b " << b << " db " << db << " dist " << sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2)) << std::endl;
	return sqrt(pow(da.x - db.x, 2) + pow(da.y - db.y, 2));
}

cv::Point get_fork(bool(&arr)[ANZAHL][ANZAHL]) {
	bool t = false;
	std::cout << "usedforks " << t << std::endl;
	for (int i = 0; i < ANZAHL; i++) {
		for (int j = 0; j < ANZAHL; j++) {
			std::cout << i << " " << j << "  " << arr[i][j] << std::endl;
		}
	}

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
					std::cout << "x " << x << " y " << y << std::endl;
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