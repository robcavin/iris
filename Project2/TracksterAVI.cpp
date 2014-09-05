#include "TracksterAVI.h"
#include <fstream>

TracksterAVI::TracksterAVI() {
	frameCount = 0;
}

TracksterAVI::~TracksterAVI() {
	this->Close();
}

void TracksterAVI::Init() {

	CommonInit();

	// Annoying but the AVI compression and decompression compresses the color space, so 
	//  the threshold is quite different.
	pupilThreshold = 4;
}

bool TracksterAVI::StartCapture() {

	char filename[32];
	sprintf_s(filename, "tester%d.avi", m_videoIndex++);

	m_video = cvCaptureFromFile(filename);
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

	while (waitForUpdateRequest && !pendingSnapshot) {};

	IplImage* image = cvQueryFrame(m_video);

	if (!image) {
		cvReleaseCapture(&m_video);

		char filename[32];
		sprintf_s(filename, "tester%d.avi", m_videoIndex++);
		m_video = cvCaptureFromFile(filename);

		image = cvQueryFrame(m_video);

		if (!image) {
			m_videoIndex = 0;
			return false;
		}
	}

	if (image) {
		cvCvtColor(image, eye_image, CV_BGR2GRAY);
		DoEyeTracking();

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
		}
		frameCount++;
	}
	else {
		droppedFrameCount++;
	}
	
	//cvWaitKey(1);

	return true;
}

void TracksterAVI::Close() {

	if (m_video) {
		cvReleaseCapture(&m_video);
		m_video = NULL;
	}

}