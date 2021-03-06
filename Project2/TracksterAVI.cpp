#include "TracksterAVI.h"
#include <fstream>

#define PER_VIDEO_SEGMENT_FRAME_COUNT 5000

#define CM_TO_SCREEN      60  // 23.5 * 2.54

#define CM_DIM_OF_SCREEN  24  // 24" wide * 2.54 * 1000 / 2560
#define CM_PIXELS_PER_DIM 1000

#define CM_PIXELS_CENTER  CM_PIXELS_PER_DIM/2
#define PRECISION_NUM_FRAMES 25

TracksterAVI::TracksterAVI() {
	frameCount = 0;
}

TracksterAVI::~TracksterAVI() {
	this->Close();
}

void TracksterAVI::Init() {

	CommonInit();

	prev_image = cvCreateImage(size, IPL_DEPTH_8U, 1);

	// Annoying but the AVI compression and decompression compresses the color space, so 
	//  the threshold is quite different.
	pupilThreshold = 15;
}

bool TracksterAVI::StartCapture() {

	char buffer[256];
	fopen_s(&trainingFile, "results.txt", "r");

	// Read in first training point
	int frame, x, y;
	float delta_x, delta_y;
	fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);
	trainingFrames[trainingPass][trainingIndex] = frame;
	trainingPoints[trainingPass][trainingIndex] = { x, y };

	// ONLY FOR FALSE EYE TEST
	//trainingDeltas[trainingPass][trainingIndex] = { delta_x, delta_y };

	// Each file has 5000 frames
	trainingInitialFrame = trainingFrames[trainingPass][trainingIndex] - PRECISION_NUM_FRAMES;
	
	m_videoIndex = trainingInitialFrame / PER_VIDEO_SEGMENT_FRAME_COUNT;

	char filename[32];
	sprintf_s(filename, "test_uc%d.avi", m_videoIndex++);

	m_video = cvCaptureFromFile(filename);
	
	cvSetCaptureProperty(m_video, CV_CAP_PROP_POS_FRAMES, trainingInitialFrame % PER_VIDEO_SEGMENT_FRAME_COUNT);

	return true;
}

bool TracksterAVI::NextFrame() {

	while (waitForUpdateRequest && !pendingSnapshot) {};

	IplImage* image = cvQueryFrame(m_video);

	bool success = true;

	if (!image) {
		cvReleaseCapture(&m_video);

		char filename[32];
		sprintf_s(filename, "test_uc%d.avi", m_videoIndex++);
		m_video = cvCaptureFromFile(filename);

		image = cvQueryFrame(m_video);

		if (!image) {
			success = false;
		}
	}

	if (image) {

		cvCvtColor(image, eye_image, CV_BGR2GRAY);

		/*if (frameCount > 0) {
			CvMat* result = cvCreateMat(2, 3, CV_64FC1);
			cvEstimateRigidTransform(eye_image, prev_image, result, TRUE);
			cvCopy(eye_image, prev_image);
			cvWarpAffine(eye_image, eye_image, result);
		}
		else {
			cvCopy(eye_image, prev_image);
		}*/
		
		DoEyeTracking();

		rollingDeltas[rollingDeltaIndex] = cvPoint2D32f(delta_x, delta_y);
		rollingDeltaIndex++;
		if (rollingDeltaIndex >= NUM_ROLLING_PROJECTIONS) rollingDeltaIndex = 0;

		if (trained) {
			rollingProjections[rollingProjectionIndex] = GetProjection();
			rollingProjectionIndex++;
			if (rollingProjectionIndex >= NUM_ROLLING_PROJECTIONS) rollingProjectionIndex = 0;
		}

		// We keep track of the past second or so's worth of projections for visualization, accuracy, precision calcs
		if ((trainingIndex < 5) && (frameCount + trainingInitialFrame == trainingFrames[trainingPass][trainingIndex])) {
			
			// COMMENT OUT FOR FALSE EYE TEST
			int curDeltaIndex = (rollingDeltaIndex - PRECISION_NUM_FRAMES + NUM_ROLLING_DELTAS) % NUM_ROLLING_DELTAS;
			CvPoint2D32f averageDelta = { 0, 0 };
			for (curDeltaIndex; curDeltaIndex != rollingDeltaIndex; curDeltaIndex = (curDeltaIndex + 1) % NUM_ROLLING_DELTAS) {
				averageDelta.x += delta_x;
				averageDelta.y += delta_y;
			}
			averageDelta.x /= PRECISION_NUM_FRAMES;
			averageDelta.y /= PRECISION_NUM_FRAMES;

			trainingDeltas[trainingPass][trainingIndex] = averageDelta;
			//trainingDeltas[trainingPass][trainingIndex] = { delta_x, delta_y };

			trainingIndex++;

			if (trainingIndex < 5) {
				int frame, x, y;
				float delta_x, delta_y;
				fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);
				trainingFrames[trainingPass][trainingIndex] = frame;
				trainingPoints[trainingPass][trainingIndex] = { x, y };

				displayStaticCrosshair = true;
				staticCrosshairCoord = cvPointTo32f(trainingPoints[trainingPass][trainingIndex]);

				// ONLY FOR FALSE EYE TEST
				//trainingDeltas[trainingPass][trainingIndex] = { delta_x, delta_y };
			}
			else {

				trainingPass++;
				if (trainingPass < 3) {
					trainingIndex = 0;
					int frame, x, y;
					float delta_x, delta_y;
					fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);
					trainingFrames[trainingPass][trainingIndex] = frame;
					trainingPoints[trainingPass][trainingIndex] = { x, y };

					displayStaticCrosshair = true;
					staticCrosshairCoord = cvPointTo32f(trainingPoints[trainingPass][trainingIndex]);

					// ONLY FOR FALSE EYE TEST
					//trainingDeltas[trainingPass][trainingIndex] = { delta_x, delta_y };

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

					Train(averageDeltas, trainingPoints[0]);

					// FIXUP FOR FALSE EYE TEST ONLY
					//xParams.offsetX = delta_x;
					//xParams.offsetY = delta_y;

					//yParams.offsetX = delta_x;
					//yParams.offsetY = delta_y;

					// Read first test point
					int frame, x, y;
					float delta_x, delta_y;
					fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);

					testIndex = 0;
					testFrames[testIndex] = frame;
					testPoints[testIndex] = { x, y };
					testDeltas[testIndex] = { delta_x, delta_y };

					displayStaticCrosshair = true;
					staticCrosshairCoord = cvPointTo32f(testPoints[testIndex]);
				}
			}
		}

		else if (trained && (frameCount + trainingInitialFrame == testFrames[testIndex])) {

			// Compute precision using last 100 samples
			int curProjectionIndex = (rollingProjectionIndex - PRECISION_NUM_FRAMES + NUM_ROLLING_PROJECTIONS) % NUM_ROLLING_PROJECTIONS;
			
			double x_precision = 0;
			double y_precision = 0;
			double angular_precision = 0;
			double angular_accuracy = 0;

			double disp_angle, x_angle, y_angle, x_delta, y_delta;

			double pixel_size = 1.0 * CM_DIM_OF_SCREEN / CM_PIXELS_PER_DIM;

			// Calculate the precision and accuracy
			CvPoint2D32f curPoint = rollingProjections[curProjectionIndex];
			CvPoint2D32f prevPoint = curPoint;

			double previous_x_angle = atan2((curPoint.x - CM_PIXELS_CENTER) * pixel_size, CM_TO_SCREEN);
			double previous_y_angle = atan2((curPoint.y - CM_PIXELS_CENTER) * pixel_size, CM_TO_SCREEN);

			curProjectionIndex = (curProjectionIndex + 1) % NUM_ROLLING_PROJECTIONS;

			CvPoint accuracyTarget = testPoints[testIndex];

			for (curProjectionIndex; curProjectionIndex != rollingProjectionIndex; curProjectionIndex = (curProjectionIndex + 1) % NUM_ROLLING_PROJECTIONS) {

				// Method 1 - calculating the angle in x and y separately
				curPoint = rollingProjections[curProjectionIndex];
				x_angle = atan2((curPoint.x - CM_PIXELS_CENTER) * pixel_size, CM_TO_SCREEN);
				y_angle = atan2((curPoint.y - CM_PIXELS_CENTER) * pixel_size, CM_TO_SCREEN);

				x_delta = x_angle - previous_x_angle;
				y_delta = y_angle - previous_y_angle;

				x_precision += x_delta * x_delta;
				y_precision += y_delta * y_delta;

				previous_x_angle = x_angle;
				previous_y_angle = y_angle;


				// Method 2 - calculate approx visual angle delta = 2 * atan( (displacement/2) / distance to screen)
				x_delta = ((curPoint.x - CM_PIXELS_CENTER) - (prevPoint.x - CM_PIXELS_CENTER)) * pixel_size;
				y_delta = ((curPoint.y - CM_PIXELS_CENTER) - (prevPoint.y - CM_PIXELS_CENTER)) * pixel_size;

				// TODO - NOT ACCURATE ENOUGH - this assumes all deltas are on unit circle
				disp_angle = 2*atan2(sqrt(x_delta * x_delta + y_delta * y_delta)/2, CM_TO_SCREEN);

				// Try to filter out saccades that happened right before or after the theoretical window
				if (disp_angle < 0.1) // 0.1 radians is a bit over 5 degrees
					angular_precision += disp_angle * disp_angle;

				// Accuracy, using average visual angle delta
				x_delta = ((curPoint.x - CM_PIXELS_CENTER) - (accuracyTarget.x - CM_PIXELS_CENTER)) * pixel_size;
				y_delta = ((curPoint.y - CM_PIXELS_CENTER) - (accuracyTarget.y - CM_PIXELS_CENTER)) * pixel_size;

				disp_angle = 2 * atan2(sqrt(x_delta * x_delta + y_delta * y_delta) / 2, CM_TO_SCREEN);

				angular_accuracy += disp_angle;

			}

			x_precision = sqrt(x_precision / (PRECISION_NUM_FRAMES - 1)) * 180 / M_PI;
			y_precision = sqrt(y_precision / (PRECISION_NUM_FRAMES - 1)) * 180 / M_PI;

			angular_precision = sqrt(angular_precision / (PRECISION_NUM_FRAMES - 1)) * 180 / M_PI;

			angular_accuracy = (angular_accuracy / (PRECISION_NUM_FRAMES - 1)) * 180 / M_PI;

			testPrecisions[testIndex] = angular_precision;
			testAccuracies[testIndex] = angular_accuracy;

			// Compute accuracy using last 100 samples with test point as target
			int frame, x, y;
			float delta_x, delta_y;
			
			int matchedItems = fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);
			if (matchedItems < 0) {

				// Reset training
				trainingIndex = 0;
				testIndex = 0;
				trainingPass = 0;
				frameCount = -1;

				fclose(trainingFile);
				fopen_s(&trainingFile, "results.txt", "r");

				int frame, x, y;
				float delta_x, delta_y;
				fscanf_s(trainingFile, "%d,%d,%d,%f,%f\n", &frame, &x, &y, &delta_x, &delta_y);
				trainingFrames[trainingPass][trainingIndex] = frame;
				trainingPoints[trainingPass][trainingIndex] = { x, y };

				// Reset video
				cvReleaseCapture(&m_video);

				m_videoIndex = trainingInitialFrame / PER_VIDEO_SEGMENT_FRAME_COUNT;

				char filename[32];
				sprintf_s(filename, "test_uc%d.avi", m_videoIndex++);

				m_video = cvCaptureFromFile(filename);

				cvSetCaptureProperty(m_video, CV_CAP_PROP_POS_FRAMES, trainingInitialFrame % PER_VIDEO_SEGMENT_FRAME_COUNT);

				//success = false;
			}
			else {

				testIndex++;
				testFrames[testIndex] = frame;
				testPoints[testIndex] = { x, y };
				testDeltas[testIndex] = { delta_x, delta_y };

				displayStaticCrosshair = true;
				staticCrosshairCoord = cvPointTo32f(testPoints[testIndex]);
			}
		}

		frameCount++;
	}
	else {
		droppedFrameCount++;
	}

	return success;
}

float TracksterAVI::GetAveragePrecision() {
	float average_precision = 0;
	for (int i = 0; i < testIndex; i++) {
		average_precision += testPrecisions[i];
	}
	return average_precision / testIndex;
}

float TracksterAVI::GetAverageAccuracy() {
	float average_accuracy = 0;
	for (int i = 0; i < testIndex; i++) {
		average_accuracy += testAccuracies[i];
	}
	return average_accuracy / testIndex;
}

void TracksterAVI::Close() {

	if (m_video) {
		cvReleaseCapture(&m_video);
		m_video = NULL;
	}

}