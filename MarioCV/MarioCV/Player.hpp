#pragma once

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "src/kcf/kcftracker.hpp"

using namespace cv;

class Player
{
public:
	Player(String trackertype, Rect2d roi, Mat frame);
	~Player();

	void initTracker(Rect2d roi, Mat frame);

	Rect2d GetPlayerLocation(Mat frame);

	// maybe first compare roi to template to see if it should be recalculated?

	static Rect2d DetectPlayer(Mat frame, Mat playerImage);

	Mat initialPlayerImg;

	Mat playerImg;
	KCFTracker* trackerKCF;
	Ptr<cv::Tracker> trackerOther;
};
