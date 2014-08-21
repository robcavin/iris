#include "Trackster.h"
#define _USE_MATH_DEFINES // for C
#include <math.h>

DWORD WINAPI DisplayThread(LPVOID param) {
	Trackster* tracker = (Trackster*)param;

	char h_eyeView[256];
	strcpy_s(h_eyeView, "Eye View");

	cvNamedWindow(h_eyeView, 0);  // create the demo window
	cvResizeWindow(h_eyeView, 320, 240);
	cvMoveWindow(h_eyeView, 500, 600);

	IplImage* imageHeader = cvCreateImageHeader(tracker->size, IPL_DEPTH_8U, 1);

	IplImage* tempImage = cvCreateImage(tracker->size, IPL_DEPTH_8U, 1);

	char h_workingView[256];
	strcpy_s(h_workingView, "Working Image");

	cvNamedWindow(h_workingView, 0);  // create the demo window
	cvResizeWindow(h_workingView, 320, 240);
	cvMoveWindow(h_workingView, 800, 600);

	while (true) {
		while (tracker->sync) {};

		tracker->sync = true;
		tracker->DisplayEyeImage(h_eyeView, imageHeader, tempImage);
		tracker->DisplayWorkingImage(h_workingView);
		tracker->sync = false;

		cvWaitKey(33);
	}

	return 0;
}

void Trackster::DisplayEyeImage(char* h_view, IplImage* reusableImageHeader, IplImage* tempImage) {
	INT ret;
	
	char* currentImage = m_pcImageMemory;
	ret = is_LockSeqBuf(m_hCam, IS_IGNORE_PARAMETER, currentImage);
	ret = is_CopyImageMem(m_hCam, currentImage, m_lMemoryId, m_copyImageMemory);
	ret = is_UnlockSeqBuf(m_hCam, IS_IGNORE_PARAMETER, currentImage);

	if (ret == 0) {
		reusableImageHeader->imageData = m_copyImageMemory;
		cvFlip(reusableImageHeader, tempImage, 1);
		cvShowImage(h_view, tempImage);
	}
}

void Trackster::DisplayWorkingImage(char* h_view) {
	cvShowImage(h_view, working_image);
}


Trackster::Trackster() {
	m_hCam = NULL;
	m_hAVI = NULL;
	frameCount = 0;
	sync = false;
}

Trackster::~Trackster() {
	this->Close();
}

void Trackster::Init() {

	if (m_hCam) this->Close();

	m_Ret = is_InitCamera(&m_hCam, NULL);

	if (m_Ret == IS_SUCCESS)
	{
		is_GetSensorInfo(m_hCam, &m_sInfo);

		//GetMaxImageSize(&m_nSizeX, &m_nSizeY);
		m_nSizeX = 320;
		m_nSizeY = 240;

		// setup the color depth to the current windows setting
		m_Ret = is_SetColorMode(m_hCam, IS_CM_MONO8);

		//is_SetSubSampling(m_hCam, IS_SUBSAMPLING_4X_VERTICAL | IS_SUBSAMPLING_4X_HORIZONTAL);

		// memory initialization
		for (int i = 0; i < 10; i++) {
			m_Ret = is_AllocImageMem(m_hCam,
				m_nSizeX,
				m_nSizeY,
				8, //m_nBitsPerPixel,
				&m_pcImageMemory,
				&m_lMemoryId);

			m_Ret = is_SetImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);	// set memory active

			m_Ret = is_AddToSequence(m_hCam, m_pcImageMemory, m_lMemoryId);
		}

		m_Ret = is_InitImageQueue(m_hCam, 0);

		is_AllocImageMem(m_hCam,
			m_nSizeX,
			m_nSizeY,
			8, //m_nBitsPerPixel,
			&m_copyImageMemory,
			&m_lMemoryId);

		//m_Ret = is_SetSubSampling(m_hCam, IS_SUBSAMPLING_2X_HORIZONTAL | IS_SUBSAMPLING_2X_VERTICAL);

		// display initialization
		IS_RECT aoi;
		aoi.s32X = 1000;
		aoi.s32Y = 1000;
		aoi.s32Width = m_nSizeX;
		aoi.s32Height = m_nSizeY;
		m_Ret = is_AOI(m_hCam, IS_AOI_IMAGE_SET_AOI, (void*)&aoi, sizeof(aoi));

		m_Ret = is_SetDisplayMode(m_hCam, IS_SET_DM_DIB);

		INT speed = 256;
		m_Ret = is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_SET,
			(void*)&speed, sizeof(speed));

		m_Ret = is_SetGainBoost(m_hCam, IS_SET_GAINBOOST_ON);
		m_Ret = is_SetHardwareGain(m_hCam, IS_SET_ENABLE_AUTO_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);

		m_Ret = is_SetFrameRate(m_hCam, 500, &m_fps);

		size.width = m_nSizeX;
		size.height = m_nSizeY;

		eye_image = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
		working_image = cvCreateImage(size, IPL_DEPTH_8U, 1);

		mem_storage = cvCreateMemStorage(0);
	}
}

bool Trackster::StartCapture() {

	if (!m_hCam) return false;

	if (is_CaptureVideo(m_hCam, IS_WAIT) == IS_SUCCESS) {
		//m_Ret = is_GetActiveImageMem(m_hCam, &pLast, &dummy);
		//m_Ret = is_GetImageMem(m_hCam, (void**)&pLast);
	}

	HANDLE displayThread = CreateThread(NULL, 0, DisplayThread, this, 0, NULL);

	return true;
}

bool Trackster::NextFrame() {

	if (!m_hCam) return false;

	m_Ret = is_WaitForNextImage(m_hCam, 1000, &m_pcImageMemory, &m_lMemoryId);
	if (m_Ret == IS_SUCCESS) {
		
		if (m_hAVI) {
			m_Ret = isavi_AddFrame(m_hAVI, m_pcImageMemory);
			if (m_Ret != IS_AVI_NO_ERR) {
				m_Ret = m_Ret;
			}
		}

		eye_image->imageData = m_pcImageMemory;
		DoEyeTracking();
		is_UnlockSeqBuf(m_hCam, IS_IGNORE_PARAMETER, m_pcImageMemory);
	}
	else {
		frameCount++;
	}

	return true;
}

void Trackster::Close() {
	if (m_hCam) {
		is_FreeImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);
		is_ExitCamera(m_hCam);
	}
	m_hCam = NULL;
}

void Trackster::DoEyeTracking() {

	while (sync) {};

	sync = true;

	int scale = 1024;
	int log_scale = 10;

	cvThreshold(eye_image, working_image, 25, 255, CV_THRESH_BINARY_INV);
	cvSmooth(working_image, working_image, CV_GAUSSIAN, 5);

	//cvDilate(working_image, working_image);
	
	IplImage* tempImage = cvCloneImage(working_image);
	CvBox2D32f pupil_box = this->findBounds(tempImage, { size.width/2, size.height/2 }, 2500);
	printf("%f %f\n", pupil_box.center.x, pupil_box.center.y);

	/*cvThreshold(eye_image, working_image, 254, 255, CV_THRESH_BINARY);
	CvBox2D32f corneal_ref_box = this->findBounds(working_image, pupil_box.center, 100);
	*/
	pupil_box.center.x *= scale;
	pupil_box.center.y *= scale;
	pupil_box.size.width *= scale;
	pupil_box.size.height *= scale;
	
	/*
	corneal_ref_box.center.x *= scale;
	corneal_ref_box.center.y *= scale;
	corneal_ref_box.size.width *= scale;
	corneal_ref_box.size.height *= scale;

	if (corneal_ref_box.size.width > 0) {
		cvEllipseBox(working_image, corneal_ref_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);
		cvEllipseBox(eye_image, corneal_ref_box, CV_RGB(0, 0, 0), 1, CV_AA, log_scale);
	}
	*/
	
	cvEllipseBox(eye_image, pupil_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);
	
	CvPoint start = { pupil_box.center.x, 0 };
	CvPoint end = { pupil_box.center.x, size.height * scale };
	cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	start = { 0, pupil_box.center.y };
	end = { size.width * scale, pupil_box.center.y };
	cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	//cvLine(eye_image, cvPointFrom32f(pupil_box.center), cvPointFrom32f(corneal_ref_box.center), CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	sync = false;
}

CvBox2D32f Trackster::findBounds(IplImage* image, CvPoint2D32f nearestTo, float targetArea) {
	CvPoint offset = cvPoint(0, 0);

	cvFindContours(image,
		mem_storage,
		&contour,
		sizeof(CvContour),
		CV_RETR_EXTERNAL,          // external contour only
		CV_CHAIN_APPROX_SIMPLE,       // no vert or hor segment contraction
		offset);

	CvContour* currentContour = (CvContour*) contour;

	CvSeq* best_contour = NULL;
	int max_count = 5;
	float best_score = 99999;

	float max_delta = (size.width * size.width / 4) + (size.height * size.height / 4);

	if (currentContour) {
		do {

			float aspect = 1.0 * 
				MAX(currentContour->rect.width, currentContour->rect.height) /
				MIN(currentContour->rect.width, currentContour->rect.height);

			float area = currentContour->rect.width * currentContour->rect.height;

			float score = aspect;
			
			if (nearestTo.x != 0) {
				int x_delta = nearestTo.x - (currentContour->rect.x + currentContour->rect.width / 2);
				int y_delta = nearestTo.y - (currentContour->rect.y + currentContour->rect.height / 2);
				
				float distance = (x_delta * x_delta + y_delta * y_delta);
				float distance_additive = distance / max_delta;
				score += 5 * distance_additive;
			}

			if (targetArea != 0) {
				score += fabs(area - targetArea) / targetArea;
			}

			//if (contour->total > max_count) {
			if (currentContour->total > 5 && score < best_score) {
				best_contour = (CvSeq*) currentContour;
				max_count = currentContour->total;
				best_score = score;
				//cvRectangleR(image, currentContour->rect, CV_RGB(255, 255, 255));
			}
		} while (currentContour = (CvContour*) currentContour->h_next);
	}

	CvBox2D32f best_box;
	best_box.size = { 0.0, 0.0 };


	if (best_score < 10) {
		best_contour->h_next = NULL;

		//CvMat* covMatrix = cvCreateMat(n, n, CV_32FC1);;
		//CvMat* mean = NULL;
		//CvMat* best_cont_array = cvCreateMat(best_contour->total, 2, CV_32S);
		//CvPoint* PointArray = (CvPoint *)malloc(best_contour->total * sizeof(CvPoint));
		//cvCvtSeqToArray(best_contour, best_cont_array->data.ptr);
		//cvCvtSeqToArray(best_contour, PointArray);
		//cvCalcCovarMatrix(best_cont_array->data, 1, covMatrix, mean, 0);

		CvSeq* hull = cvConvexHull2(best_contour, mem_storage, CV_CLOCKWISE, 1);

		cvDrawContours(working_image, hull, CV_RGB(50, 50, 50), CV_RGB(128, 128, 128), 0);
		
		if (hull->total > 5) best_box = cvFitEllipse2(hull);
	}

	if (best_box.size.width > 0 && best_box.size.height > 0) {
		
		CvPoint topLeft = { best_box.center.x - best_box.size.width / 2 - 5, best_box.center.y - best_box.size.height / 2 - 5 };
		int width = MIN(best_box.size.width + 10, 320 - topLeft.x);
		int height = MIN(best_box.size.height + 10, 240 - topLeft.y);

		CvRect rect = { topLeft.x, topLeft.y, width, height };

		cvSetImageROI(eye_image, rect);
		
		CvSize actualSize = { eye_image->roi->width, eye_image->roi->height };

		IplImage* sobelImageX = cvCreateImage(actualSize, IPL_DEPTH_16S, 1);
		IplImage* sobelImageY = cvCreateImage(actualSize, IPL_DEPTH_16S, 1);
		//IplImage* displayImageX = cvCreateImage(actualSize, IPL_DEPTH_8U, 1);
		//IplImage* displayImageY = cvCreateImage(actualSize, IPL_DEPTH_8U, 1);

		cvSobel(eye_image, sobelImageX, 1, 0, 7);
		cvSobel(eye_image, sobelImageY, 0, 1, 7);

		// For display only
		//cvConvertScaleAbs(sobelImageX, displayImageX, 0.5, 0.5);
		//cvConvertScaleAbs(sobelImageY, displayImageY, 0.5, 0.5);
		//cvAddWeighted(displayImageX, 0.5, displayImageY, 0.5, 0, displayImageY);
		cvSetImageROI(working_image, rect);
		//cvCopy(displayImageY, working_image);

		float grad_x, grad_y, grad_mag;

		float delta_x, delta_y, delta_mag;
		float dot_product;

		float sum;
		float max_sum = -1;

		CvPoint center;

		double sin_theta = sin(best_box.angle / 180 * M_PI);
		double cos_theta = cos(best_box.angle / 180 * M_PI);

		double ellipse_center_x = actualSize.width / 2;
		double ellipse_center_y = actualSize.height / 2;

		double ellipse_a_squared = (best_box.size.width / 2) * (best_box.size.width / 2);
		double ellipse_b_squared = (best_box.size.height / 2) * (best_box.size.height / 2);

		for (int i = rect.height/2 - 10; i < rect.height/2 + 10; i++) {
			for (int j = rect.width/2 - 10; j < rect.width/2 + 10; j++) {

				sum = 0;
				for (int ii = 0; ii < rect.height; ii++) {
					for (int jj = 0; jj < rect.width; jj++) {

						float rotated_jj = (jj - ellipse_center_x)*cos_theta + (ii - ellipse_center_y)*sin_theta;
						float rotated_ii = (jj - ellipse_center_x)*-sin_theta + (ii - ellipse_center_y)*cos_theta;

						float result = (rotated_jj*rotated_jj) / ellipse_a_squared + (rotated_ii*rotated_ii) / ellipse_b_squared - 1;

						if (result > -0.2 && result < 0.2 && !((i == ii) && (j == jj))) {

							grad_x = cvGet2D(sobelImageX, ii, jj).val[0];
							grad_y = cvGet2D(sobelImageY, ii, jj).val[0];

							grad_mag = sqrt(grad_x*grad_x + grad_y*grad_y);

							delta_x = jj - j;
							delta_y = ii - i;

							delta_mag = sqrt(delta_x*delta_x + delta_y*delta_y);

							if (grad_mag > 1000 && delta_mag > 0) {
								grad_x /= grad_mag;
								grad_y /= grad_mag;

								delta_x /= delta_mag;
								delta_y /= delta_mag;

								dot_product = grad_x*delta_x + grad_y*delta_y;

								sum += dot_product  * dot_product;

								cvSet2D(working_image, ii, jj, cvScalar(255));
							}
						}
					}
				}

				if (sum > max_sum) {
					max_sum = sum;
					center.x = i;
					center.y = j;
				}
			}
		}

		int scale = 1024;
		int log_scale = 10;

		center.x *= scale;
		center.y *= scale;

		CvPoint start = { center.x, 0 };
		CvPoint end = { center.x, actualSize.height * scale };
		cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

		start = { 0, center.y };
		end = { actualSize.width * scale, center.y };
		cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

		cvResetImageROI(eye_image);
		cvResetImageROI(working_image);
	}

	return best_box;
}

void Trackster::PrepareAVI() {

	if (!m_hCam) return;

	INT ret;

	ret = isavi_InitAVI(&m_hAVI, m_hCam);

	ret = isavi_SetImageSize(m_hAVI, IS_AVI_CM_Y8, m_nSizeX, m_nSizeY, 0, 0, 0);
	ret = isavi_OpenAVI(m_hAVI, "test.avi");
	ret = isavi_StartAVI(m_hAVI);
}

void Trackster::CloseAVI() {
	
	if (!m_hAVI) return;

	INT ret;

	ret = isavi_StopAVI(m_hAVI);
	ret = isavi_CloseAVI(m_hAVI);
	ret = isavi_ExitAVI(m_hAVI);
}

