#include "TracksterAVI.h"
#include <iostream>

boolean runTracking = true;

DWORD WINAPI TrackingThread(LPVOID param) {

	Trackster* trackster = (Trackster*)param;

	trackster->Init();

	int frame = 0;

	int nFrames = 10000;

	bool captureAVI = false;

	if (captureAVI) {
		trackster->PrepareAVI();
	}

	trackster->StartCapture();

	DWORD start_time = GetTickCount();

	while (runTracking && !captureAVI || frame < nFrames) {
		trackster->NextFrame();
		frame++;
	}

	DWORD end_time = GetTickCount();

	if (captureAVI) {
		trackster->CloseAVI();
	}

	trackster->Close();

	std::cout << " FPS = " << nFrames * 1000.0 / (end_time - start_time);
	std::cout << " Dropped frames = " << trackster->frameCount;

	return 0;
}


int main(int argc, char** argv)
{
	Trackster trackster;

	HANDLE tackingThread = CreateThread(NULL, 0, TrackingThread, &trackster, 0, NULL);
	
	char h_trainingView[256];
	strcpy_s(h_trainingView, "Training image");
	cvNamedWindow(h_trainingView, 0);  // create the demo window
	cvResizeWindow(h_trainingView, 1000, 1000);
	cvMoveWindow(h_trainingView, 1200, 600);

	trackster.overlayView = h_trainingView;

	IplImage* image = cvCreateImage({ 1000, 1000 }, IPL_DEPTH_32F, 4);
	CvPoint trainingPoints[5] = { { 500, 500 }, { 100, 100 }, { 900, 900 }, { 100, 900 }, { 900, 100 } };

	int trainingIndex = -1;

	CvPoint2D32f trainingDeltas[5];

	while (1) {
		int key = cvWaitKey();
		if (key >= 0) {
				switch (key) {
				case('q') :
					trackster.pupilThreshold = MIN(255, MAX(0, trackster.pupilThreshold + 1));
					break;
				case('a') :
					trackster.pupilThreshold = MIN(255, MAX(0, trackster.pupilThreshold - 1));
					break;

				case('w') :
					trackster.glintThreshold = MIN(255, MAX(0, trackster.glintThreshold + 1));
					break;
				case('s') :
					trackster.glintThreshold = MIN(255, MAX(0, trackster.glintThreshold - 1));
					break;

				case(' ') :
					
					// Clear this to let us take back our training image screen
  					trackster.trained = false;

					if (trainingIndex < 0) {
						showCrosshair(h_trainingView, image, cvPointTo32f(trainingPoints[++trainingIndex]));
					}

					else if (trainingIndex < 4) {
						// Capture points 0, 1, 2, and 3
						trainingDeltas[trainingIndex].x = trackster.delta_x;
						trainingDeltas[trainingIndex].y = trackster.delta_y;

						showCrosshair(h_trainingView, image, cvPointTo32f(trainingPoints[++trainingIndex]));
					}

					else if (trainingIndex == 4) {
						// Capture point 4
						trainingDeltas[trainingIndex].x = trackster.delta_x;
						trainingDeltas[trainingIndex].y = trackster.delta_y;
						
						// Train
						trackster.Train(trainingDeltas, trainingPoints);

						// Reset in case we want to retrain
						trainingIndex = -1;
					}
					break;
			}
		}
	}
	
	WaitForSingleObject(TrackingThread, INFINITE);
}
