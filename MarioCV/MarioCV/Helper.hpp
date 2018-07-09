#include "opencv2/imgproc.hpp"

using namespace cv;

const String kImagePlayer = "images/player.jpg";

bool roiIsValid(Rect2d roi, Mat frame);