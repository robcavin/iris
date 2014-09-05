#include "TracksterAVI.h"
#include <iostream>
#include <fstream>
#include "TracksterRenderer.h"

boolean runTracking = true;

CvPoint testPoints[10];
CvPoint2D32f testProjections[10];
int testFrames[10];
int testPointIndex = 0;

void mouseClickCallback(int event, int x, int y, int flags, void* param) {

	Trackster* trackster = (Trackster*)param;

	if ((event == CV_EVENT_LBUTTONDOWN) && trackster->trained && testPointIndex < 10) {

		testFrames[testPointIndex] = trackster->frameCount;
		
		CvPoint2D32f projeciton = trackster->GetProjection();
		testProjections[testPointIndex] = projeciton;

		testPoints[testPointIndex].x = x;
		testPoints[testPointIndex].y = y;

		testPointIndex++;
	}
}


DWORD WINAPI TrackingThread(LPVOID param) {

	Trackster* trackster = (Trackster*)param;

	trackster->Init();

	int frame = 0;

	bool captureAVI = false;

	if (captureAVI) {
		trackster->PrepareAVI();
	}

	trackster->StartCapture();

	DWORD start_time = GetTickCount();

	while (runTracking) {
		trackster->NextFrame();
		frame++;
	}

	DWORD end_time = GetTickCount();

	if (captureAVI) {
		trackster->CloseAVI();
	}

	trackster->Close();

	std::cout << " FPS = " << frame * 1000.0 / (end_time - start_time);
	std::cout << " Dropped frames = " << trackster->droppedFrameCount << "\n";

	return 0;
}

int main(int argc, char* args[])
{
	printf("Hello world\n");

	TracksterRenderer renderer;
	TracksterAVI trackster;

	renderer.hTrackster = &trackster;
	
	renderer.init();

	HANDLE tackingThread = CreateThread(NULL, 0, TrackingThread, &trackster, 0, NULL);

	IplImage* image = cvCreateImage({ 1000, 1000 }, IPL_DEPTH_32F, 4);
	CvPoint trainingPoints[5] = { { 500, 500 }, { 100, 100 }, { 900, 900 }, { 100, 900 }, { 900, 100 } };

	int trainingIndex = -1;

	CvPoint2D32f trainingDeltas[5];
	int trainingFrames[5];

	//cvSetMouseCallback(h_trainingView, mouseClickCallback, &trackster);

	SDL_Event event;

	while (runTracking) {
		
		while (SDL_PollEvent(&event)) {
			
			char key = (event.type == SDL_KEYDOWN) ? event.key.keysym.sym : -1;
			
			switch (key) {
			case('x') :
				runTracking = false;

				if (testPointIndex > 0) {

					std::ofstream outFile;
					outFile.open("results.txt");

					for (int i = 0; i < 5; i++) {
						outFile << trainingFrames[i] << "," << trainingPoints[i].x << "," << trainingPoints[i].y;
						outFile << "\n";
					}

					for (int i = 0; i < testPointIndex; i++) {
						outFile << testFrames[i] << "," << testPoints[i].x << "," << testPoints[i].y;
						outFile << "," << testProjections[i].x << "," << testProjections[i].y;
						outFile << "\n";
					}

					outFile.close();
				}

				break;

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
					//showCrosshair(h_trainingView, image, cvPointTo32f(trainingPoints[++trainingIndex]));
					renderer.displayStaticCrosshair = true;
					renderer.staticCrosshairCoord = cvPointTo32f(trainingPoints[++trainingIndex]);
				}

				else if (trainingIndex < 4) {
					// Capture points 0, 1, 2, and 3
					trainingDeltas[trainingIndex].x = trackster.delta_x;
					trainingDeltas[trainingIndex].y = trackster.delta_y;
					trainingFrames[trainingIndex] = trackster.frameCount;

					//showCrosshair(h_trainingView, image, cvPointTo32f(trainingPoints[++trainingIndex]));
					renderer.displayStaticCrosshair = true;
					renderer.staticCrosshairCoord = cvPointTo32f(trainingPoints[++trainingIndex]);
				}

				else if (trainingIndex == 4) {
					renderer.displayStaticCrosshair = false;

					// Capture point 4
					trainingDeltas[trainingIndex].x = trackster.delta_x;
					trainingDeltas[trainingIndex].y = trackster.delta_y;
					trainingFrames[trainingIndex] = trackster.frameCount;

					// Train
					trackster.Train(trainingDeltas, trainingPoints);

					// Reset in case we want to retrain
					trainingIndex = -1;
				}
				break;
			}
		}

		renderer.render();

	}
	
	WaitForSingleObject(TrackingThread, INFINITE);

	std::cout << "done!\n";

	cvWaitKey();

	SDL_Quit();

	return 0;
}
