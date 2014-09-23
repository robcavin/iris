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

	while (runTracking && trackster->NextFrame()) {
		frame++;
	}

	float precision = trackster->GetAveragePrecision();
	float accuracy = trackster->GetAverageAccuracy();

	std::cout << "Average Precision = " << precision << "\n";
	std::cout << "Average Accuracy = " << accuracy << "\n";

	DWORD end_time = GetTickCount();

	if (captureAVI) {
		trackster->CloseAVI();
	}

	trackster->Close();

	std::cout << " FPS = " << frame * 1000.0 / (end_time - start_time) << "\n";;

	std::cout << " Attempted frames = " << frame << "\n";
	std::cout << " Successful frames = " << trackster->frameCount << "\n";
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
	int trainingPass = 0;

	CvPoint2D32f trainingDeltas[3][5];
	int trainingFrames[3][5];

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

					for (int j = 0; j < 3; j++) {
						for (int i = 0; i < 5; i++) {
							outFile << trainingFrames[j][i] << ","
								<< trainingPoints[i].x << "," << trainingPoints[i].y << ","
								<< trainingDeltas[j][i].x << "," << trainingDeltas[j][i].y;
							outFile << "\n";
						}
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
					trackster.displayStaticCrosshair = true;
					trackster.staticCrosshairCoord = cvPointTo32f(trainingPoints[++trainingIndex]);
				}

				else if (trainingIndex < 4) {
					// Capture points 0, 1, 2, and 3
					trainingDeltas[trainingPass][trainingIndex].x = trackster.delta_x;
					trainingDeltas[trainingPass][trainingIndex].y = trackster.delta_y;
					trainingFrames[trainingPass][trainingIndex] = trackster.frameCount;

					//showCrosshair(h_trainingView, image, cvPointTo32f(trainingPoints[++trainingIndex]));
					trackster.displayStaticCrosshair = true;
					trackster.staticCrosshairCoord = cvPointTo32f(trainingPoints[++trainingIndex]);
				}

				else if (trainingIndex == 4) {
					trackster.displayStaticCrosshair = false;

					// Capture point 4
					trainingDeltas[trainingPass][trainingIndex].x = trackster.delta_x;
					trainingDeltas[trainingPass][trainingIndex].y = trackster.delta_y;
					trainingFrames[trainingPass][trainingIndex] = trackster.frameCount;

					// Train
					trainingPass++;
 					if (trainingPass < 3) {
						trainingIndex = -1;
						trackster.displayStaticCrosshair = true;
						trackster.staticCrosshairCoord = cvPointTo32f(trainingPoints[++trainingIndex]);
					}
					else {
						CvPoint2D32f averageDeltas[5] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
						for (int i = 0; i < 5; i++) {
							for (int j = 0; j < trainingPass; j++) {
								averageDeltas[i].x += trainingDeltas[j][i].x;
								averageDeltas[i].y += trainingDeltas[j][i].y;
							}

							averageDeltas[i].x /= 3;
							averageDeltas[i].y /= 3;
						}

  						trackster.Train(averageDeltas, trainingPoints);
						// Reset in case we want to retrain
						trainingIndex = -1;
						trainingPass = 0;
					}
				}
				break;
			}


			if ((event.type == SDL_MOUSEBUTTONDOWN) && trackster.trained && testPointIndex < 10) {

				testFrames[testPointIndex] = trackster.frameCount;

				CvPoint2D32f projeciton = trackster.GetProjection();
				testProjections[testPointIndex] = projeciton;

				testPoints[testPointIndex].x = event.button.x;
				testPoints[testPointIndex].y = event.button.y;

				testPointIndex++;
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
