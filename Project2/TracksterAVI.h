#include <uEye.h>
#include <uEye_tools.h>

#include <opencv2\opencv.hpp>

#include "Trackster.h"

class TracksterAVI : public Trackster {

private:

	// AVI specific vars
	CvCapture* m_video;

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