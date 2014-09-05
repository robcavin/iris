#include "Trackster.h"

//-----------------------------------------------------------------
// HELPERS
//

//---------- calibration  coefficient calculation ---------------//
// biquadratic equation fitter               
// x, y are coordinates of eye tracker point 
// X is x or y coordinate of screen point    
// computes a, b, c, d, and e in the biquadratic 
// X = a + b*(x-inx) + c*(y-iny) + d*(x-inx)*(x-inx) + e*(y-iny)*(y-iny) 
// where inx = x1, y1 = y1 to reduce the solution to a 4x4 matrix        

BQParams dqfit(float x1, float y1,
	float x2, float y2,
	float x3, float y3,
	float x4, float y4,
	float x5, float y5,
	float X1, float X2, float X3, float X4, float X5)
{
	float den;
	float x22, x32, x42, x52;    // squared terms 
	float y22, y32, y42, y52;

	BQParams bqParams;

	bqParams.offsetX = (int)x1;            // record eye tracker centering constants 
	bqParams.offsetY = (int)y1;
	bqParams.a = X1;                    // first coefficient 
	X2 -= X1;  X3 -= X1;       // center screen points 
	X4 -= X1;  X5 -= X1;
	x2 -= x1;  x3 -= x1;       // center eye tracker points 
	x4 -= x1;  x5 -= x1;
	y2 -= y1;  y3 -= y1;
	y4 -= y1;  y5 -= y1;
	x22 = x2*x2; x32 = x3*x3;   // squared terms of biquadratic 
	x42 = x4*x4; x52 = x5*x5;
	y22 = y2*y2; y32 = y3*y3;
	y42 = y4*y4; y52 = y5*y5;

	//Cramer's rule solution of 4x4 matrix */
	den = -x2*y3*x52*y42 - x22*y3*x4*y52 + x22*y5*x4*y32 - y22*x42*y3*x5 -
		x32*y22*x4*y5 - x42*x2*y5*y32 + x32*x2*y5*y42 - y2*x52*x4*y32 +
		x52*x2*y4*y32 + y22*x52*y3*x4 + y2*x42*x5*y32 + x22*y3*x5*y42 -
		x32*x2*y4*y52 - x3*y22*x52*y4 + x32*y22*x5*y4 - x32*y2*x5*y42 +
		x3*y22*x42*y5 + x3*y2*x52*y42 + x32*y2*x4*y52 + x42*x2*y3*y52 -
		x3*y2*x42*y52 + x3*x22*y4*y52 - x22*y4*x5*y32 - x3*x22*y5*y42;

	bqParams.b = (-y32*y2*x52*X4 - X2*y3*x52*y42 - x22*y3*X4*y52 + x22*y3*y42*X5 +
		y32*y2*x42*X5 - y22*x42*y3*X5 + y22*y3*x52*X4 + X2*x42*y3*y52 +
		X3*y2*x52*y42 - X3*y2*x42*y52 - X2*x42*y5*y32 + x32*y42*y5*X2 +
		X2*x52*y4*y32 - x32*y4*X2*y52 - x32*y2*y42*X5 + x32*y2*X4*y52 +
		X4*x22*y5*y32 - y42*x22*y5*X3 - x22*y4*y32*X5 + x22*y4*X3*y52 +
		y22*x42*y5*X3 + x32*y22*y4*X5 - y22*x52*y4*X3 - x32*y22*y5*X4) / den;

	bqParams.c = (-x32*x4*y22*X5 + x32*x5*y22*X4 - x32*y42*x5*X2 + x32*X2*x4*y52 +
		x32*x2*y42*X5 - x32*x2*X4*y52 - x3*y22*x52*X4 + x3*y22*x42*X5 +
		x3*x22*X4*y52 - x3*X2*x42*y52 + x3*X2*x52*y42 - x3*x22*y42*X5 -
		y22*x42*x5*X3 + y22*x52*x4*X3 + x22*y42*x5*X3 - x22*x4*X3*y52 -
		x2*y32*x42*X5 + X2*x42*x5*y32 + x2*X3*x42*y52 + x2*y32*x52*X4 +
		x22*x4*y32*X5 - x22*X4*x5*y32 - X2*x52*x4*y32 - x2*X3*x52*y42) / den;

	bqParams.d = -(-x4*y22*y3*X5 + x4*y22*y5*X3 - x4*y2*X3*y52 + x4*y2*y32*X5 -
		x4*y32*y5*X2 + x4*y3*X2*y52 - x3*y22*y5*X4 + x3*y22*y4*X5 +
		x3*y2*X4*y52 - x3*y2*y42*X5 + x3*y42*y5*X2 - x3*y4*X2*y52 -
		y22*y4*x5*X3 + y22*X4*y3*x5 - y2*X4*x5*y32 + y2*y42*x5*X3 +
		x2*y3*y42*X5 - y42*y3*x5*X2 + X4*x2*y5*y32 + y4*X2*x5*y32 -
		y42*x2*y5*X3 - x2*y4*y32*X5 + x2*y4*X3*y52 - x2*y3*X4*y52) / den;

	bqParams.e = -(-x3*y2*x52*X4 + x22*y3*x4*X5 + x22*y4*x5*X3 - x3*x42*y5*X2 -
		x42*x2*y3*X5 + x42*x2*y5*X3 + x42*y3*x5*X2 - y2*x42*x5*X3 +
		x32*x2*y4*X5 - x22*y3*x5*X4 + x32*y2*x5*X4 - x22*y5*x4*X3 +
		x2*y3*x52*X4 - x52*x2*y4*X3 - x52*y3*x4*X2 - x32*y2*x4*X5 +
		x3*x22*y5*X4 + x3*y2*x42*X5 + y2*x52*x4*X3 - x32*x5*y4*X2 -
		x32*x2*y5*X4 + x3*x52*y4*X2 + x32*x4*y5*X2 - x3*x22*y4*X5) / den;

	return bqParams;
}

//
// END HELPERS
//------------------------------------------------------


Trackster::Trackster() {
	m_hCam = NULL;
	m_hAVI = NULL;
	frameCount = 0;
	pupilThreshold = 16;
	glintThreshold = 254;
	trained = false;

	overlayView = NULL;

	lock = SDL_CreateMutex();
	cond = SDL_CreateCond();
}

Trackster::~Trackster() {
	this->Close();
}

void Trackster::CommonInit() {
	
	m_nSizeX = 320;
	m_nSizeY = 240;

	size.width = m_nSizeX;
	size.height = m_nSizeY;

	eye_image = cvCreateImage(size, IPL_DEPTH_8U, 1);
	working_image = cvCreateImage(size, IPL_DEPTH_8U, 1);

	eye_snapshot_image = cvCreateImage(size, IPL_DEPTH_8U, 1);
	working_snapshot_image = cvCreateImage(size, IPL_DEPTH_8U, 1);

	for (int i = 0; i < NUM_TEST_IMAGES; i++) {
		test_snapshot_image[i] = cvCreateImage(size, IPL_DEPTH_8U, 1);
	}

	mem_storage = cvCreateMemStorage(0);
}

void Trackster::Init() {

	CommonInit();

	if (m_hCam) this->Close();

	m_Ret = is_InitCamera(&m_hCam, NULL);

	if (m_Ret == IS_SUCCESS)
	{
		is_GetSensorInfo(m_hCam, &m_sInfo);

		//GetMaxImageSize(&m_nSizeX, &m_nSizeY);

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

		m_Ret = is_SetDisplayMode(m_hCam, IS_SET_DM_DIB);

		// display initialization
		IS_RECT aoi;
		aoi.s32X = 1044;
		aoi.s32Y = 902;
		aoi.s32Width = m_nSizeX;
		aoi.s32Height = m_nSizeY;
		setAOI(aoi);

		m_Ret = is_SetGainBoost(m_hCam, IS_SET_GAINBOOST_ON);
		//m_Ret = is_SetHardwareGain(m_hCam, IS_SET_ENABLE_AUTO_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
	}
}


void Trackster::setAOI(IS_RECT aoi) {
	m_Ret = is_AOI(m_hCam, IS_AOI_IMAGE_SET_AOI, (void*)&aoi, sizeof(aoi));

	INT speed = 256;
	m_Ret = is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_SET, (void*)&speed, sizeof(speed));

	m_Ret = is_SetFrameRate(m_hCam, 500, &m_fps);

	double exposure = 0;
	m_Ret = is_Exposure(m_hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposure, sizeof(exposure));
}


bool Trackster::StartCapture() {

	if (!m_hCam) return false;

	if (is_CaptureVideo(m_hCam, IS_WAIT) == IS_SUCCESS) {
		//m_Ret = is_GetActiveImageMem(m_hCam, &pLast, &dummy);
		//m_Ret = is_GetImageMem(m_hCam, (void**)&pLast);
	}

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
		frameCount++;
	}
	else {
		droppedFrameCount++;
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

	int scale = 1024;
	int log_scale = 10;

	cvThreshold(eye_image, working_image, pupilThreshold, 255, CV_THRESH_BINARY_INV);

	SDL_LockMutex(lock);

	test_snapshot_image_index = 0;
	if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	cvSmooth(working_image, working_image, CV_GAUSSIAN, 5);

	if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	//cvDilate(working_image, working_image);
	
	CvBox2D32f pupil_box = this->findBounds(working_image, { size.width/2, size.height/2 }, 2500);
	printf("%d %f %f\n", frameCount, pupil_box.center.x, pupil_box.center.y);

	//CvBox2D32f corneal_ref_box = { 0.0, 0.0, 0.0, 0.0 };// this->findBounds(working_image, pupil_box.center, 100);
	cvThreshold(eye_image, working_image, glintThreshold, 255, CV_THRESH_BINARY);
	if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	CvBox2D32f corneal_ref_box = this->findBounds(working_image, pupil_box.center, 100);

	CvPoint2D32f pupil_center = pupil_box.center;
	CvPoint2D32f corneal_ref_center = corneal_ref_box.center;

	pupil_box.center.x *= scale;
	pupil_box.center.y *= scale;
	pupil_box.size.width *= scale;
	pupil_box.size.height *= scale;

	cvEllipseBox(eye_image, pupil_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	CvPoint start = { pupil_box.center.x, 0 };
	CvPoint end = { pupil_box.center.x, size.height * scale };
	cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	start = { 0, pupil_box.center.y };
	end = { size.width * scale, pupil_box.center.y };
	cvLine(eye_image, start, end, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

	corneal_ref_box.center.x *= scale;
	corneal_ref_box.center.y *= scale;
	corneal_ref_box.size.width *= scale;
	corneal_ref_box.size.height *= scale;

	if (corneal_ref_box.size.width > 0) {
		cvEllipseBox(working_image, corneal_ref_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);
		cvEllipseBox(eye_image, corneal_ref_box, CV_RGB(0, 0, 0), 1, CV_AA, log_scale);

		cvLine(eye_image, cvPointFrom32f(pupil_box.center), cvPointFrom32f(corneal_ref_box.center), CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

		delta_x = pupil_center.x - corneal_ref_center.x;
		delta_y = pupil_center.y - corneal_ref_center.y;
	}

	if (pendingSnapshot) {
		cvCopy(eye_image, eye_snapshot_image);
		cvCopy(working_image, working_snapshot_image);
		pendingSnapshot = false;
	}

	SDL_CondSignal(cond);
	SDL_UnlockMutex(lock);
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

	if (pendingSnapshot) cvCopy(image, test_snapshot_image[test_snapshot_image_index++]);

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

		cvDrawContours(image, hull, CV_RGB(50, 50, 50), CV_RGB(128, 128, 128), 0);
		if (hull->total > 5) best_box = cvFitEllipse2(hull);

		if (pendingSnapshot) {
			int scale = 1024;
			int log_scale = 10;
			
			CvBox2D32f tempBox = best_box;
			tempBox.center.x *= scale;
			tempBox.center.y *= scale;
			tempBox.size.width *= scale;
			tempBox.size.height *= scale;

			cvEllipseBox(image, tempBox, CV_RGB(0, 0, 0), 1, CV_AA, log_scale);
			cvCopy(image, test_snapshot_image[test_snapshot_image_index++]);
		}
	}

	return best_box;
}

bool Trackster::Train(CvPoint2D32f deltas[5], CvPoint calibPoints[5]) {
	
	// x fit
	xParams = dqfit(
		deltas[0].x, deltas[0].y,
		deltas[1].x, deltas[1].y,
		deltas[2].x, deltas[2].y,
		deltas[3].x, deltas[3].y,
		deltas[4].x, deltas[4].y,

		calibPoints[0].x,
		calibPoints[1].x,
		calibPoints[2].x,
		calibPoints[3].x,
		calibPoints[4].x);


	// y fit
	yParams = dqfit(
		deltas[0].x, deltas[0].y,
		deltas[1].x, deltas[1].y,
		deltas[2].x, deltas[2].y,
		deltas[3].x, deltas[3].y,
		deltas[4].x, deltas[4].y,

		calibPoints[0].y,
		calibPoints[1].y,
		calibPoints[2].y,
		calibPoints[3].y,
		calibPoints[4].y);

	trained = true;

	return true;
}

CvPoint2D32f Trackster::GetProjection() {

	if (!trained) return { 0, 0 };
	
	float cur_delta_x = delta_x;
	float cur_delta_y = delta_y;

	float x_adj = cur_delta_x - xParams.offsetX;
	float y_adj = cur_delta_y - xParams.offsetY;

	float x = xParams.a + xParams.b*(x_adj) + xParams.c*(y_adj) + xParams.d*(x_adj*x_adj) + xParams.e*(y_adj*y_adj);
	
	x_adj = cur_delta_x - yParams.offsetX;
	y_adj = cur_delta_y - yParams.offsetY;

	float y = yParams.a + yParams.b*(x_adj) + yParams.c*(y_adj) + yParams.d*(x_adj*x_adj) + yParams.e*(y_adj*y_adj);

	return { x, y };
}

void Trackster::UpdateImageSnapshots() {
	SDL_LockMutex(lock);
	pendingSnapshot = true;
	SDL_CondWait(cond, lock);
	SDL_UnlockMutex(lock);
}

IplImage* Trackster::GetEyeImage() {
	return eye_snapshot_image;
}

IplImage* Trackster::GetWorkingImage() {
	return working_snapshot_image;
}

IplImage* Trackster::GetTestImage(int index) {
	return test_snapshot_image[index];
}


void Trackster::PrepareAVI() {

	if (!m_hCam) return;

	INT ret;

	ret = isavi_InitAVI(&m_hAVI, m_hCam);

	ret = isavi_SetImageSize(m_hAVI, IS_AVI_CM_Y8, size.width, size.height, 0, 0, 0);
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

