#include "Player.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "src/kcf/kcftracker.hpp"
#include "Helper.hpp"

using namespace cv;

Player::Player(String trackerType, Rect2d roi, Mat frame)
{
	// List of tracker types in OpenCV 3.2
	// NOTE : GOTURN implementation is buggy and does not work.
	String trackerTypes[6] = { "BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN" };

	// Create a tracker
	//String trackerType = trackerTypes[2];

	if (trackerType == "BOOSTING")
		trackerOther = TrackerBoosting::create();
	if (trackerType == "MIL")
		trackerOther = TrackerMIL::create();
	if (trackerType == "KCF")
		trackerKCF = new KCFTracker(true, true, true, true);
	if (trackerType == "TLD")
		trackerOther = TrackerTLD::create();
	if (trackerType == "MEDIANFLOW")
		trackerOther = TrackerMedianFlow::create();
	if (trackerType == "GOTURN")
		trackerOther = TrackerGOTURN::create();

	// initialize the tracker
	if (trackerType == "KCF")
		trackerKCF->init(roi, frame);
	else
		trackerOther->init(frame, roi);

	initialPlayerImg = frame(roi);
	//imwrite(kImagePlayer, initialPlayerImg);
}

Player::~Player()
{
	delete trackerKCF;
	delete trackerOther;
}

void Player::initTracker(Rect2d roi, Mat frame) {
	if (roiIsValid(roi, frame)) {
		if (trackerOther != 0)
		{
			trackerOther->init(frame, roi);
		}
		else
		{
			trackerKCF->init(roi, frame);
		}
	}
}

Rect2d Player::GetPlayerLocation(Mat frame)
{
	Rect2d roi;
	// update the tracking result
	if (trackerOther != 0)
	{
		trackerOther->update(frame, roi);
	}
	else
	{
		roi = trackerKCF->update(frame);
	}

	//if (0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= frame.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= frame.rows) playerImg = frame(roi);

	return roi;
}

// detect player and reinitialize tracker
//https://docs.opencv.org/3.2.0/de/da9/tutorial_template_matching.html
 Rect2d Player::DetectPlayer(Mat frame, Mat playerImage)
{
	Mat result;
	int result_cols = frame.cols - playerImage.cols + 1;
	int result_rows = frame.rows - playerImage.rows + 1;
	
	// Available template matching: TM_SQDIFF, c, TM_CCORR, TM_CCORR_NORMED, TM_CCOEFF, TM_CCOEFF_NORMED 
	int match_method = TM_SQDIFF_NORMED;

	result.create(result_rows, result_cols, CV_32FC1);
	matchTemplate(frame, playerImage, result, match_method);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	bool playerFound = false;
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		if (minVal < 100) playerFound = true;
		printf(std::to_string(minVal).c_str());
	}
	else
	{
		matchLoc = maxLoc;
		if (maxVal > 1000) playerFound = true;
	}

	Rect2d roi(matchLoc, Point(matchLoc.x + playerImage.cols, matchLoc.y + playerImage.rows));
	
	// reinitialize thr tracker
	if (!playerFound) {
		roi.x = -1;
	}

	return roi;
}