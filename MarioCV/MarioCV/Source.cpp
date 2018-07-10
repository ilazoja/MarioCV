// Next steps: implement detection after awhile to recalibrate

// Will binding like LuaBridge be needed? Maybe not since almost everything will be handled in C++, just need Lua for emulator

// New method: http://www.troubleshooters.com/codecorn/lua/lua_lua_calls_c.htm


#include <windows.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "opencv2/core/ocl.hpp"
#include "src/kcf/kcftracker.hpp"
#include "Player.hpp"
#include "Helper.hpp";

extern "C" // must wrap it around extern "C" as it is c code, also must get rid of luac.c since it has main function
{
	#include "src/lua/lua.h"                               /* Always include this */
	#include "src/lua/lauxlib.h"                           /* Always include this */
	#include "src/lua/lualib.h"                            /* Always include this */
}

using namespace std;
using namespace cv;

// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

HWND hwndDesktop;

int frameCount;
int framesBeforeDetecting = 1000; // number of frames before detecting again
Player* mario;
Helper* helper;

// Setup desktop capture
Mat hwnd2mat(HWND hwnd)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
	width = windowsize.right / 1;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	Mat frame; 
	// reduce from 4 channel to 3 channel
	cvtColor(src, frame, CV_BGRA2BGR);

	return frame;
}

static void detectEnemies(Mat frame, vector<Player*>& enemies) {

	float pixelThreshold = 0.8;
	Rect r(frame.cols - helper->redKoopaImage.cols - (frame.cols - frame.rows)/2, 0, helper->redKoopaImage.cols, frame.rows);
	Mat rightEdge = frame(r);

	Mat res;
	Mat thresholdedImage;
	matchTemplate(rightEdge, helper->redKoopaImage, res, TM_CCOEFF_NORMED);
	threshold(res, thresholdedImage, pixelThreshold, 255, CV_THRESH_BINARY);
	for (int r = 0; r < thresholdedImage.rows; ++r) {
		for (int c = 0; c < thresholdedImage.cols; ++c) {
			if (thresholdedImage.at<float>(r, c)) {// = thresholdedImage(r,c) == 0
				//cout << thresholdedImage.at<float>(r, c) << endl;
				Rect roi(frame.cols - helper->redKoopaImage.cols - (frame.cols - frame.rows) / 2, r, helper->redKoopaImage.cols, helper->redKoopaImage.rows);
				Player* enemy = new Player("KCF", roi, frame);
				enemies.push_back(enemy);
			}
		}
	}

}

static int initPlayer() {
	hwndDesktop = GetDesktopWindow();
	namedWindow("output", WINDOW_NORMAL);
	namedWindow("KCF", WINDOW_NORMAL);

	// get desktop capture
	Mat frame = hwnd2mat(hwndDesktop);

	helper = new Helper();

	Rect2d roi;
	Mat playerImage = imread(kImagePlayer, CV_LOAD_IMAGE_ANYCOLOR);

	if (!playerImage.data) {
		// get bounding box
		roi = selectROI("KCF", frame);
	}
	else {
		roi = mario->DetectPlayer(frame, playerImage);
	}

	mario = new Player("KCF", roi, frame);

	return 1;
}

static int processScreen() {
	static vector<Player*> enemies = {};
	//float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	//printf("Top of cube(), number=%f\n", rtrn);
	//lua_pushnumber(L, rtrn*rtrn*rtrn);      /* Push the return */
	int key = 0;

	Mat frame = hwnd2mat(hwndDesktop);

	// you can do some image processing here
	imshow("output", frame);

	Rect2d roi;
	if (frameCount < framesBeforeDetecting) {
		roi = mario->GetPlayerLocation(frame);
		if (!roiIsValid(roi, frame)) {
			roi = mario->DetectPlayer(frame, mario->initialPlayerImg);
			frameCount = 0;
			mario->initTracker(roi, frame);
		}
		else mario->playerImg = frame(roi);

	}
	else
	{
		roi = mario->DetectPlayer(frame,mario->initialPlayerImg);
		frameCount = 0;
		mario->initTracker(roi, frame);
	}

	if (roiIsValid(roi, frame)) rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);

	detectEnemies(frame, enemies);
	for (int i = 0; i < enemies.size(); ++i) {
		roi = enemies[i]->GetPlayerLocation(frame);
		if (roiIsValid(roi, frame)) rectangle(frame, roi, Scalar(70, 25, 80), 2, 1);
	}
	// draw the tracked object
	//cout << roi.x << endl;
	//cout << roi.y << endl;

	// show image with the tracked object
	imshow("KCF", frame);

	frameCount++;
	//cout << frameCount << endl;

	return 1;                              /* One return value */
}

static int isquare(lua_State *L) {              /* Internal name of func */
	float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	printf("Top of square(), nbr=%f\n", rtrn);
	lua_pushnumber(L, rtrn*rtrn);           /* Push the return */
	int result = MessageBox(NULL, "TEST", "TEST", MB_OK); // THIS WORKS
	return 1;                              /* One return value */
}
static int icube(lua_State *L) {                /* Internal name of func */
	float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	printf("Top of cube(), number=%f\n", rtrn);
	lua_pushnumber(L, rtrn*rtrn*rtrn);      /* Push the return */
	return 1;                              /* One return value */
}

static int iReadScreen(lua_State *L) {
	
	//float rtrn = lua_tonumber(L, -1);      /* Get the single number arg */
	//printf("Top of cube(), number=%f\n", rtrn);
	//lua_pushnumber(L, rtrn*rtrn*rtrn);      /* Push the return */
	int key = 0;
	
	processScreen();

	return 1;                              /* One return value */
}


/* Register this file's functions with the
* luaopen_libraryname() function, where libraryname
* is the name of the compiled .so (or any library like .dll) output. In other words
* it's the filename (but not extension) after the -o
* in the cc command.
*
* So for instance, if your cc command has -o power.so then
* this function would be called luaopen_power().
*
* This function should contain lua_register() commands for
* each function you want available from Lua.
*
*/

// extern "C" needed to make sure function does not get mangled, 
// __declspec(dllexport) needed to expose function
#ifdef _DEBUG
int main()
{
	initPlayer();

	while (true)
	{
		//quit on ESC button
		if (waitKey(1) == 27)break;

		processScreen();
		
	}

	return 0;
}

#else
extern "C" int __declspec(dllexport) luaopen_MarioCV(lua_State *L) { // IMPORTANT THEY ARE NAMED THE SAME
	lua_register(
		L,               /* Lua state variable */
		"square",        /* func name as known in Lua */
		isquare          /* func name in this file */
	);
	lua_register(L, "cube", icube);
	lua_register(L, "readScreen", iReadScreen);

	initPlayer();

	return 0;
}

#endif

/*
// Old method that didn't work: http://lua-users.org/wiki/CreatingBinaryExtensionModules
#include <windows.h>

extern "C" // must wrap it around extern "C" as it is c code, also must get rid of lua.c since it has main function
{
#include "src/lauxlib.h"
}

//Pop-up a Windows message box with your choice of message and caption 
int lua_msgbox(lua_State* L)
{
	const char* message = luaL_checkstring(L, 1);
	const char* caption = luaL_optstring(L, 2, "");
	int result = MessageBox(NULL, message, caption, MB_OK);
	lua_pushnumber(L, result);
	return 1;
}

extern "C" int __declspec(dllexport) libinit(lua_State* L)
{
	lua_register(L, "msgbox", lua_msgbox);
	return 0;
}*/

// Extend to Mario Party?

// Instead write to text used by lua to determine control?