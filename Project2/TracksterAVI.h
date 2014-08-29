#include <uEye.h>
#include <uEye_tools.h>

#include <opencv2\opencv.hpp>

#include "Trackster.h"

class TracksterAVI : public Trackster {

private:

	// AVI specific vars
	CvCapture* m_video;

	CvPoint trainingPoints[5];
	CvPoint2D32f trainingDeltas[5];
	int trainingFrames[5];
	int trainingIndex = 0;
	FILE *trainingFile;

public:

	TracksterAVI();
	~TracksterAVI();

	void Init();
	bool StartCapture();
	bool NextFrame();
	void Close();

	void TracksterAVI::DisplayEyeImage(char* h_view);
	void DisplayWorkingImage(char* h_view);

};