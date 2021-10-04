#include <iostream>  
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "digitRecognition.h"
#include "imageInterpreter.h"

#define N 20

using namespace cv;
using namespace std;

struct cmp1 {
	bool operator() (cv::Point pt1, cv::Point pt2) { return (pt1.x < pt2.x); }
} xSort;
struct cmp2 {
	bool operator() (cv::Point pt1, cv::Point pt2) { return (pt1.y < pt2.y); }
} ySort;

//Scales and centres an image onto a new background square.
void scale_and_centre(Mat* img, double size) {
	int margin = 20, background = 0;
	int h = img->rows, w = img->cols;
	int l_pad, r_pad, t_pad, b_pad;
	if (h > w) {
		t_pad = margin / 2;
		b_pad = t_pad;
		double ratio = (size - margin) / h;
		w = ratio * w, h = ratio * h;
		l_pad, r_pad;
		l_pad = int((size - w) / 2);
		r_pad = w % 2 == 0 ? l_pad : l_pad + 1;
	}
	else{
		l_pad = margin / 2;
		r_pad = l_pad;
		double ratio = (size - margin) / w;
		w = ratio * w, h = ratio * h;
		t_pad, b_pad;
		t_pad = int((size - h) / 2);
		b_pad = h % 2 == 0 ? t_pad : t_pad + 1;
	}
			
	resize(*img, *img, Size(w, h));
	copyMakeBorder(*img, *img, t_pad, b_pad, l_pad, r_pad, BORDER_CONSTANT);
	resize(*img, *img, Size(size, size));
}

//predict the handwriting digit
int predict(Mat* img) {
	Mat image;
	img->copyTo(image);
	/*cvtColor(image, image, COLOR_BGR2GRAY);
	threshold(image,image, 140, 255, THRESH_BINARY);*/
	resize(image, image, Size(28, 28));

	return 0;
}

int** imageToArray(char* path, int r, int c) {
	//load the original image
	Mat image = imread(path);
	int rowNum = r, colNum = c;
	//half the size
	resize(image, image, Size(), 0.5, 0.5, CV_INTER_LINEAR);
	//imshow("Origin_Image", image);
	//discard the color information(grayscale)
	Mat gray;
	cvtColor(image, gray, COLOR_BGR2GRAY);
	//imshow("Gray", gray);
	//apply a Gaussian blur on the image to remove the noise
	Mat blur;
	GaussianBlur(gray, blur, Size(5, 5), 0);
	//imshow("Blur", blur);
	//adaptive threshold(thresholds the image taking an optimum value for a local neighborhood)
	Mat thresh;
	adaptiveThreshold(blur, thresh, 255, 1, 1, 11, 2);
	//imshow("Thresh", thresh);
	//process all the contours to find the one that has the maximum area(the biggest borders)
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(thresh, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//find the biggest contour
	double max_area = 0;
	int best_cnt = 0;
	for (int i = 0;i < contours.size();i++) {
		double area = contourArea(contours[i]);
		if (area > 1000&& area > max_area) {
			max_area = area;
			best_cnt = i;
			drawContours(image, contours, i, Scalar(0, 255, 0), 3);
		}
	}
	//create a mask image according to these borders
	Mat mask = cv::Mat::zeros(gray.size(), CV_8U);
	drawContours(mask, contours, best_cnt, Scalar(255, 255, 255), -1);
	drawContours(mask, contours, best_cnt, Scalar(0, 0, 0), 2);
	//imshow("Mask", mask);
	//clip the image
	Mat out = cv::Mat::zeros(gray.size(), CV_8U);
	gray.copyTo(out, mask);
	//imshow("New_Image", out);
	//blur the image and apply the adaptive threshold on the new image
	GaussianBlur(out, blur, Size(5, 5), 0);
	//imshow("Blur1", blur);
	adaptiveThreshold(blur, thresh, 255, 1, 1, 11, 2);
	//imshow("Thresh1", thresh);
	//compute the vertex of the bounding rectangle
	Rect boundRect = boundingRect(contours[best_cnt]);
	vector<Point> approx;
	double len = arcLength(contours[best_cnt], true)*0.009;
	approxPolyDP(contours[best_cnt], approx, len, true);
	for (int i = 0;i < approx.size();i++) {
		circle(image, approx[i], 5, Scalar(0, 0, 255), -1);
	}
	//sort the points based on their x - coordinates
	sort(approx.begin(), approx.end(), xSort);
	vector<Point> leftMost, rightMost;
	for (int i = 0;i < approx.size();i++) {
		if (i < 2) leftMost.push_back(approx[i]);
		else rightMost.push_back(approx[i]);
	}
	//sort the points based on their y - coordinates
	sort(leftMost.begin(), leftMost.end(), ySort);
	sort(rightMost.begin(), rightMost.end(), ySort);
	/*apply perspective transformation*/
	//Convert points to Point2f format
	Point2f ordered_corners[] = {
		cv::Point2f(leftMost[0]),
		cv::Point2f(rightMost[0]),
		cv::Point2f(rightMost[1]),
		cv::Point2f(leftMost[1]) };
	Point2f dimensions[] = {
		cv::Point2f(0,0),
		cv::Point2f(boundRect.width - 1,0),
		cv::Point2f(boundRect.width - 1, boundRect.height - 1),
		cv::Point2f(0, boundRect.height - 1) };
	
	//calculate the perspective transform matrix and warp the perspective to grab the screen
	Mat grid = getPerspectiveTransform(ordered_corners, dimensions);
	warpPerspective(image, image, grid, Size(boundRect.width, boundRect.height));

	warpPerspective(thresh, grid, grid, Size(boundRect.width, boundRect.height));
	int edge_h = grid.rows, edge_w = grid.cols;
	int celledge_h = edge_h / rowNum,
		celledge_w = edge_w / colNum;
	//cout << edge_h << " " << edge_w << " " << celledge_h << " " << celledge_w << endl;

	//imshow("Final_Gird", grid);

	//find out the contours in the image and draw these contours on the image
	findContours(grid, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	for (int i = 0;i < contours.size();i++) {
		double area = contourArea(contours[i]);
		if (area > 1000/2) {
			drawContours(image, contours, i, Scalar(0, 255, 0), 3);
		}
	}
	//imshow("New_image", image);
	//cut the image according to the grid
	vector<vector<Mat>> finalgrid;
	for (int i = 0;i < edge_h - celledge_h;i += celledge_h) {
		vector<Mat> temp;
		for (int j = 0;j < edge_w - celledge_w;j += celledge_w) {
			Rect rect(j, i, celledge_w, celledge_h);
			Mat roi_img(grid, rect);
			temp.push_back(roi_img);
			/*//imshow("ROI", roi_img);
			waitKey(0);*/
		}
		finalgrid.push_back(temp);
	}
	//digit recognition
	DigitRecognizer *dr = new DigitRecognizer();
	bool b = dr->train("../Grid_recognition/grid_recognition/mnist_dataset/train-images.idx3-ubyte", "../Grid_recognition/grid_recognition/mnist_dataset/train-labels.idx1-ubyte");
	if (b == false) {
		cout << "Unsuccessful training." << endl;
		return NULL;
	}
	int** final_grid = (int**)new int*[rowNum];
	for (int i = 0;i < colNum;i++) *(final_grid + i) = new int[colNum];
	for (int i = 0;i < rowNum;i++)
		for (int j = 0;j < colNum;j++) final_grid[i][j] = 0;
	vector<vector<Point> > cnts;
	for (int i = 0;i < rowNum;i++) {
		for (int j = 0;j < colNum;j++) {
			//define a threshold, 128 is the middle of black and white in grey scale
			int threshold_value = 128;
			//threshold the image
			threshold(finalgrid[i][j], gray, threshold_value, 255, THRESH_BINARY);
			/*//imshow("Threshold_Gray", gray);
			waitKey(0);*/
			findContours(gray, cnts, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
			/*vector<Point> cnt;
			if (cnts.size() == 2) cnt = cnts[0];
			else cnt = cnts[1];*/
			//find the digit by cell contours
			for (int k = 0;k < cnts.size();k++) {
				Rect cell = boundingRect(cnts[k]);
				int x = cell.x, y = cell.y, w = cell.width, h = cell.height;
				int thickness = 6;//filter the blank boxes by thickness
				if (x < thickness || y < thickness
					|| h < thickness || w < thickness)
					continue;
				//filter the noisy digit
				double area = contourArea(cnts[k]);
				//cout << "Number area: " << area << endl;
				if (area < celledge_h*0.5) continue;
				//Rect rect(j, i, celledge_w, celledge_h);
				Mat ROI(gray, cell);
				scale_and_centre(&ROI, 120);
				int number = dr->classify(ROI);
				//cout << "Predicted number: " << number << endl;
				final_grid[i][j] = number;
				/*imshow("ROI", ROI);
				waitKey(0);*/
			}
		}
	}
	/*cout << "[\n";
	for (int i = 0;i < rowNum;i++) {
		if(i>0)cout << ",\n[";
		else cout << "[";
		for (int j = 0;j < colNum;j++) {
			cout << final_grid[i][j];
			if(j < colNum - 1)  cout << ", ";
		}
		cout << "]";
	}
	cout << "\n]";*/
		
	//waitKey(0);
	return final_grid;
}