#include <uEye.h>
#include <uEye_tools.h>

#include <opencv2\opencv.hpp>

#include "Trackster.h"

class TracksterAVI : public Trackster {

private:

	// AVI specific vars
	CvCapture* m_video;
	int m_videoIndex = 0;

	CvPoint trainingPoints[3][5];
	CvPoint testPoints[20];

	CvPoint2D32f trainingDeltas[3][5];
	CvPoint2D32f testDeltas[20];
	
	float testPrecisions[20];
	float testAccuracies[20];

	int trainingFrames[3][5];
	int testFrames[20];

	int trainingPass = 0;
	int trainingIndex = 0;
	int testIndex = 0;

	FILE *trainingFile;

	IplImage* prev_image;

public:

	TracksterAVI();
	~TracksterAVI();

	void Init();
	bool StartCapture();
	bool NextFrame();
	void Close();

	float GetAveragePrecision();
	float GetAverageAccuracy();
};