#include "Helper.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "opencv2/core/ocl.hpp"

using namespace cv;

bool roiIsValid(Rect2d roi, Mat frame) {
	if (0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= frame.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= frame.rows) return true;
	else return false;
}

Helper::Helper()
{
	redKoopaImage = imread(kImageRedKoopa, CV_LOAD_IMAGE_ANYCOLOR);
}

Helper::~Helper()
{

}