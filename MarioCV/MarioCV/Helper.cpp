#include "Helper.hpp"

using namespace cv;

bool roiIsValid(Rect2d roi, Mat frame) {
	if (0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= frame.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= frame.rows) return true;
	else return false;
}