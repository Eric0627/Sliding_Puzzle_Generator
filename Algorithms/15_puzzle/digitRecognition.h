#include <iostream>  
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml.hpp>

using namespace cv;
using namespace ml;

#define MAX_NUM_IMAGES 60000
class DigitRecognizer {
public:
	DigitRecognizer();

	~DigitRecognizer();

	bool train(const char* trainPath, const char* labelsPath);

	int classify(Mat img);//takes an image and returns what digit it is

private:
	Mat preprocessImage(Mat img);

	int readFlippedInteger(FILE *fp);

private:
	Ptr<KNearest> knn;
	int numRows, numCols, numImages;

};