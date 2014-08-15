#include <uEye.h>
#include <uEye_tools.h>

#include <opencv2\opencv.hpp>

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

protected:
	CvMemStorage* mem_storage;
	CvSeq* contour;
	IplImage* eye_image;
	IplImage* working_image;
	CvBox2D32f findBounds(IplImage* image, CvPoint2D32f nearestTo, float minArea);

public:
	bool sync;

	Trackster();
	~Trackster();

	void Init();
	bool StartCapture();
	bool NextFrame();
	void Close();

	void PrepareAVI();
	void CloseAVI();

	int frameCount;
	bool running;

	CvSize size;

	void DoEyeTracking();

	// Used by bg display thread
	void DisplayEyeImage(char* viewName, IplImage* reusableImageHeader, IplImage* tempImage);
	void DisplayWorkingImage(char* h_view);
};

