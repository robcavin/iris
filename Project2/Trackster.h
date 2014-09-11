#ifndef __TRACKSTER_H__
#define __TRACKSTER_H__

#include <uEye.h>
#include <uEye_tools.h>
#include <opencv2\opencv.hpp>
#include <SDL.h>

#define NUM_TEST_IMAGES 10
#define NUM_ROLLING_PROJECTIONS 500

typedef struct {
	float offsetX;
	float offsetY;
	float a;
	float b;
	float c;
	float d;
	float e;
} BQParams;

void showCrosshair(char* windowName, IplImage* image, CvPoint2D32f center);


class Trackster {

private:

	// uEye varibles
	HIDS	m_hCam;			// handle to camera
	INT		m_Ret;			// return value of uEye SDK functions
	INT		m_nColorMode;	// Y8/RGB16/RGB24/REG32
	INT		m_nBitsPerPixel;// number of bits needed store one pixel
	INT		m_nSizeX;		// width of video 
	INT		m_nSizeY;		// height of video
	char*	m_pcImageMemory;// grabber memory - pointer to buffer
	INT		m_lMemoryId;	// grabber memory - buffer ID
	SENSORINFO m_sInfo;	    // sensor information struct
	
	INT		m_hAVI;
	
	double  m_fps;

	char*	m_copyImageMemory;// grabber memory - pointer to buffer

	void setAOI(IS_RECT rect);

	SDL_mutex *lock;
	SDL_cond *cond;

	CvPoint2D32f prevPupilBoxCenter;
	CvBox2D prevPupilBoxes[10];

protected:
	CvMemStorage* mem_storage;
	CvSeq* contour;

	CvBox2D findBounds(IplImage* image, CvPoint2D32f nearestTo, float minArea, CvBox2D* maskArea);

	BQParams xParams;
	BQParams yParams;

	IplImage* eye_image;
	IplImage* working_image;
	
	IplImage* eye_snapshot_image;
	IplImage* working_snapshot_image;
	
	IplImage* test_snapshot_image[NUM_TEST_IMAGES];
	int test_snapshot_image_index;

	bool pendingSnapshot = false;

	void CommonInit();

public:

	bool trained;

	int frameCount;
	int droppedFrameCount;
	bool running;

	int pupilThreshold;
	int glintThreshold;

	float delta_x;
	float delta_y;

	CvSize size;
	char* overlayView;

	Trackster();
	
	virtual ~Trackster();

	virtual void Init();
	virtual bool StartCapture();
	virtual bool NextFrame();
	virtual void Close();

	virtual void PrepareAVI();
	virtual void CloseAVI();

	virtual bool Train(CvPoint2D32f deltas[5], CvPoint calibPoints[5]);
	virtual CvPoint2D32f GetProjection();

	virtual void DoEyeTracking();

	bool waitForUpdateRequest = false;

	virtual void UpdateImageSnapshots();
	virtual IplImage* GetEyeImage();
	virtual IplImage* GetWorkingImage();

	virtual IplImage* GetTestImage(int index);

	virtual float GetAveragePrecision();
	virtual float GetAverageAccuracy();

	CvPoint2D32f rollingProjections[NUM_ROLLING_PROJECTIONS];
	int rollingProjectionIndex = 0;
};

#endif
