#include "TracksterAVI.h"
#include <fstream>

DWORD WINAPI AVIDisplayThread(LPVOID param) {
	TracksterAVI* tracker = (TracksterAVI*)param;

	IplImage* trackingDisplayImage = cvCreateImage({ 1000, 1000 }, IPL_DEPTH_32F, 4);

	char h_eyeView[256];
	strcpy_s(h_eyeView, "Eye View");

	cvNamedWindow(h_eyeView, 0);  // create the demo window
	cvResizeWindow(h_eyeView, 320, 240);
	cvMoveWindow(h_eyeView, 500, 600);

	char h_workingView[256];
	strcpy_s(h_workingView, "Working Image");

	cvNamedWindow(h_workingView, 0);  // create the demo window
	cvResizeWindow(h_workingView, 320, 240);
	cvMoveWindow(h_workingView, 800, 600);

	while (true) {
		while (tracker->sync) {};

		tracker->sync = true;
		tracker->DisplayEyeImage(h_eyeView);
		tracker->DisplayWorkingImage(h_workingView);

		if (tracker->overlayView != NULL && tracker->trained) {
			CvPoint2D32f projection = tracker->GetProjection();
			showCrosshair(tracker->overlayView, trackingDisplayImage, projection);
		}

		tracker->sync = false;

		cvWaitKey(33);
	}

	return 0;
}


void TracksterAVI::DisplayEyeImage(char* h_view) {
	cvShowImage(h_view, eye_image);
}

void TracksterAVI::DisplayWorkingImage(char* h_view) {
	cvShowImage(h_view, working_image);
}


TracksterAVI::TracksterAVI() {
	frameCount = 0;
}

TracksterAVI::~TracksterAVI() {
	this->Close();
}

void TracksterAVI::Init() {

	// Annoying but the AVI compression and decompression compresses the color space, so 
	//  the threshold is quite different.
	pupilThreshold = 4;

	HANDLE displayThread = CreateThread(NULL, 0, AVIDisplayThread, this, 0, NULL);

	size.width = 320;
	size.height = 240;

	eye_image = cvCreateImage(size, IPL_DEPTH_8U, 1);
	working_image = cvCreateImage(size, IPL_DEPTH_8U, 1);

	mem_storage = cvCreateMemStorage(0);
}

bool TracksterAVI::StartCapture() {

	//m_video = cvCaptureFromFile("C:\\Users\\Oculus VR Inc\\Documents\\Visual Studio 2013\\Projects\\Project2\\Project2\\tester.avi");
	m_video = cvCaptureFromFile("tester.avi");
	//cvSetCaptureProperty(m_video, CV_CAP_PROP_POS_FRAMES, 9320);

	char buffer[256];
	fopen_s(&trainingFile, "results.txt", "r");

	// Read in first training point
	int frame, x, y;
	fscanf_s(trainingFile, "%d,%d,%d\n", &frame, &x, &y);
	trainingFrames[trainingIndex] = frame;
	trainingPoints[trainingIndex].x = x;
	trainingPoints[trainingIndex].y = y;

	return true;
}

bool TracksterAVI::NextFrame() {

	IplImage* image = cvQueryFrame(m_video);
	if (image) {
		cvCvtColor(image, eye_image, CV_BGR2GRAY);
		/*DoEyeTracking();

		if (!trained && (frameCount == trainingFrames[trainingIndex])) {
			trainingDeltas[trainingIndex].x = delta_x;
			trainingDeltas[trainingIndex].y = delta_y;

			trainingIndex++;

			if (trainingIndex == 5) {
				Train(trainingDeltas, trainingPoints);
				fclose(trainingFile);
			}
			else {
				int frame, x, y;
				fscanf_s(trainingFile, "%d,%d,%d\n", &frame, &x, &y);
				trainingFrames[trainingIndex] = frame;
				trainingPoints[trainingIndex].x = x;
				trainingPoints[trainingIndex].y = y;
			}
		}*/
		frameCount++;
	}
	else {
		droppedFrameCount++;
	}
	
	//cvWaitKey(1);

	return true;
}

void TracksterAVI::Close() {

	cvReleaseCapture(&m_video);
}