#include "TracksterAVI.h"
#include <iostream>

void showCrosshair(char* windowName, IplImage* image, CvPoint center) {
	cvSet(image, CV_RGB(255, 255, 255));
	cvRectangle(image, { center.x - 2, center.y - 10 }, { center.x + 2, center.y + 10 }, CV_RGB(255, 0, 0), CV_FILLED);
	cvRectangle(image, { center.x - 10, center.y - 2 }, { center.x + 10, center.y + 2 }, CV_RGB(0, 255, 0), CV_FILLED);
	cvShowImage(windowName, image);
}

int main(int argc, char** argv)
{
	char h_trainingImage[256];
	strcpy_s(h_trainingImage, "Training image");
	cvNamedWindow(h_trainingImage, 0);  // create the demo window
	cvResizeWindow(h_trainingImage, 1000, 1000);
	cvMoveWindow(h_trainingImage, 1200, 600);

	IplImage* image = cvCreateImage({ 1000, 1000 }, IPL_DEPTH_32F, 4);
	CvPoint trainingPoints[5] = { { 500, 500 }, { 100, 100 }, { 900, 900 }, { 100, 900 }, { 900, 100 } };

	TracksterAVI trackster;

	trackster.Init();

	int frame = 0;

	int nFrames = 10000;

	bool captureAVI = false;
	
	if (captureAVI) {
		trackster.PrepareAVI();
	}

	trackster.StartCapture();

	DWORD start_time = GetTickCount();

	int trainingIndex = 0;
	while (frame < nFrames) {
		trackster.NextFrame();
		frame++;
		
		/*
		int key = cvWaitKey(1);
		if (key >= 0) {
			showCrosshair(h_trainingImage, image, trainingPoints[trainingIndex]);
			trainingIndex = ++trainingIndex % 5;
		}*/
	}

	DWORD end_time = GetTickCount();

	if (captureAVI) {
		trackster.CloseAVI();
	}

	trackster.Close();

	std::cout << " FPS = " << nFrames * 1000.0 / (end_time - start_time);
	std::cout << " Dropped frames = " << trackster.frameCount;
	int i;
	std::cin >> i;

	return 0;
}
