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
	h_AVIwriter = NULL;

	frameCount = 0;
	pupilThreshold = 15;
	glintThreshold = 250;
	trained = false;

	prevPupilBoxCenter = { 0.0f, 0.0f };

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

		// disply initialization
		IS_RECT aoi;
		aoi.s32X = 844;
		aoi.s32Y = 400;
		aoi.s32Width = m_nSizeX;
		aoi.s32Height = m_nSizeY;
		setAOI(aoi);

		//m_Ret = is_SetGainBoost(m_hCam, IS_SET_GAINBOOST_ON);

		// Set new mode (enable auto blacklevel)
		int nMode = IS_AUTO_BLACKLEVEL_OFF;
		m_Ret = is_Blacklevel(m_hCam, IS_BLACKLEVEL_CMD_SET_MODE, (void*)&nMode, sizeof(nMode));

		int blackOffset = 0;
		m_Ret = is_Blacklevel(m_hCam, IS_BLACKLEVEL_CMD_SET_OFFSET, (void*)&blackOffset, sizeof(blackOffset));
		
		//m_Ret = is_SetHardwareGain(m_hCam, IS_SET_ENABLE_AUTO_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
	}
}


void Trackster::setAOI(IS_RECT aoi) {
	m_Ret = is_AOI(m_hCam, IS_AOI_IMAGE_SET_AOI, (void*)&aoi, sizeof(aoi));

	INT speed = 100;
	m_Ret = is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_SET, (void*)&speed, sizeof(speed));

	m_Ret = is_SetFrameRate(m_hCam, 100, &m_fps);

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
		
		//if (m_hAVI) m_Ret = isavi_AddFrame(m_hAVI, m_pcImageMemory);

		eye_image->imageData = m_pcImageMemory;

		if (h_AVIwriter) {
			m_Ret = cvWriteFrame(h_AVIwriter, eye_image);
			
			// We have to create a new video every 5k frames to avoid the 2Gig limit
			if (frameCount > 0 && !(frameCount % 5000)) {
				cvReleaseVideoWriter(&h_AVIwriter);
				char filename[32];
				sprintf_s(filename, "test_uc%d.avi", m_AVIIndex++);
				h_AVIwriter = cvCreateVideoWriter(filename, CV_FOURCC('D', 'I', 'B', ' '), 25, size, 0);
			}
		}

		DoEyeTracking();

		// Update rolling deltas and projections
		rollingDeltas[rollingDeltaIndex] = cvPoint2D32f(delta_x, delta_y);
		rollingDeltaIndex++;
		if (rollingDeltaIndex >= NUM_ROLLING_PROJECTIONS) rollingDeltaIndex = 0;

		if (trained) {
			rollingProjections[rollingProjectionIndex] = GetProjection();
			rollingProjectionIndex++;
			if (rollingProjectionIndex >= NUM_ROLLING_PROJECTIONS) rollingProjectionIndex = 0;
		}

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

	SDL_LockMutex(lock);

	test_snapshot_image_index = 0;

	int scale = 1024;
	int log_scale = 10;

	//cvDilate(working_image, working_image);
	
	//CvBox2D32f corneal_ref_box = { 0.0, 0.0, 0.0, 0.0 };// this->findBounds(working_image, pupil_box.center, 100);
	cvThreshold(eye_image, working_image, glintThreshold, 255, CV_THRESH_BINARY);
	if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	CvBox2D corneal_ref_box = this->findBounds(working_image, prevPupilBoxCenter, 150, NULL);

	cvThreshold(eye_image, working_image, pupilThreshold, 255, CV_THRESH_BINARY_INV);

	if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	cvSmooth(working_image, working_image, CV_GAUSSIAN, 5);
	//if (pendingSnapshot) cvCopy(working_image, test_snapshot_image[test_snapshot_image_index++]);

	CvBox2D pupil_box = this->findBounds(working_image, prevPupilBoxCenter, 6000, &corneal_ref_box);
	
	// Save pupil boxes for reference
	for (int i = 9; i > 0; i--) {
		prevPupilBoxes[i] = prevPupilBoxes[i - 1];
	}
	prevPupilBoxes[0] = pupil_box;
	printf("%d %f %f %f %f\n", frameCount, pupil_box.center.x, pupil_box.center.y, corneal_ref_box.center.x, corneal_ref_box.center.y);

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

		cvEllipseBox(working_image, pupil_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);
		cvEllipseBox(working_image, corneal_ref_box, CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

		cvEllipseBox(eye_image, corneal_ref_box, CV_RGB(0, 0, 0), 1, CV_AA, log_scale);

		cvLine(eye_image, cvPointFrom32f(pupil_box.center), cvPointFrom32f(corneal_ref_box.center), CV_RGB(255, 255, 255), 1, CV_AA, log_scale);

		delta_x = pupil_center.x -corneal_ref_center.x;
		delta_y = pupil_center.y -corneal_ref_center.y;
	}

	// For pupil only tracking
	//delta_x = pupil_center.x;
	//delta_y = pupil_center.y;

	if (pendingSnapshot) {
		cvCopy(eye_image, eye_snapshot_image);
		cvCopy(working_image, working_snapshot_image);
		pendingSnapshot = false;
	}

	prevPupilBoxCenter = pupil_center;
		
	SDL_CondSignal(cond);
	SDL_UnlockMutex(lock);

	cvClearMemStorage(mem_storage);
}

CvSeq* removeBoxInclusionsFromSequence(CvSeq* seq, CvBox2D* maskArea, CvMemStorage* mem_storage = NULL, IplImage* image = NULL) {
	

	CvPoint* cur_point_ptr;
	CvPoint cur_point;
	
	CvSeq* resultSeq = cvCreateSeq(seq->flags, seq->header_size, sizeof(CvPoint), mem_storage);

	float mask_area_max_dim = 1.5*MAX(maskArea->size.height, maskArea->size.width);

	for (int i = 0; i < seq->total; i++) {
		cur_point_ptr = (CvPoint*)cvGetSeqElem(seq, i);
		cur_point = *cur_point_ptr;
		if ((cur_point.x < maskArea->center.x - mask_area_max_dim) ||
			(cur_point.x > maskArea->center.x + mask_area_max_dim) ||
			(cur_point.y < maskArea->center.y - mask_area_max_dim) ||
			(cur_point.y > maskArea->center.y + mask_area_max_dim))
		{
			cvSeqPush(resultSeq, cur_point_ptr);
		} 

		// Display points we've removed for sanity checking
		else {
			if (image) {
				for (int i = -1; i < 2; i++) {
					for (int j = -1; j < 2; j++) {
						cvSet2D(image, cur_point.y + i, cur_point.x + j, cvScalar(255));
					}
				}
			}
		}
	}

	return resultSeq;
}

typedef struct{
	double x, y, majorAxis, minorAxis, angle;
	double f1_x, f1_y, f2_x, f2_y;
	int fitCount;
} Ellipse_t;

double acot_d(double val){
	double acot = atan(1 / val);

	return acot * 180 / M_PI;
}

void getEllipseParam(double a, double b, double c, double d, double f, double g, Ellipse_t& ellipse){
	ellipse.x = (c * d - b * f) / (b * b - a * c);
	ellipse.y = (a * f - b * d) / (b * b - a * c);

	ellipse.majorAxis = sqrt((2 * (a*f*f + c*d*d + g*b*b - 2 * b*d*f - a*c*g)) / ((b*b - a*c)*(sqrt((a - c)*(a - c) + 4 * b*b) - (a + c))));
	ellipse.minorAxis = sqrt((2 * (a*f*f + c*d*d + g*b*b - 2 * b*d*f - a*c*g)) / ((b*b - a*c)*(sqrt((a - c)*(a - c) + 4 * b*b) + (a + c))));

	ellipse.angle = 0;
	if (b == 0 && a < c){
		ellipse.angle = 0;
	}
	else if (b == 0 && a > c){
		ellipse.angle = 90;
	}
	else if (b != 0 && a < c){
		ellipse.angle = 0.5 * acot_d((a - c) / (2 * b));
	}
	else if (b != 0 && a > c){
		ellipse.angle = 90 + 0.5 * acot_d((a - c) / (2 * b));
	}
	if (ellipse.minorAxis > ellipse.majorAxis){
		double temp = ellipse.majorAxis;
		ellipse.majorAxis = ellipse.minorAxis;
		ellipse.minorAxis = temp;
		ellipse.angle += 90;
	}

	double temp_c;
	if (ellipse.majorAxis > ellipse.minorAxis)
		temp_c = sqrt(ellipse.majorAxis * ellipse.majorAxis - ellipse.minorAxis * ellipse.minorAxis);
	else
		temp_c = sqrt(ellipse.minorAxis * ellipse.minorAxis - ellipse.majorAxis * ellipse.majorAxis);
	ellipse.f1_x = ellipse.x - temp_c * cos(ellipse.angle*M_PI / 180);
	ellipse.f1_y = ellipse.y - temp_c * sin(ellipse.angle*M_PI / 180);
	ellipse.f2_x = ellipse.x + temp_c * cos(ellipse.angle*M_PI / 180);
	ellipse.f2_y = ellipse.y + temp_c * sin(ellipse.angle*M_PI / 180);
}

bool pointInEllipse(CvPoint point, Ellipse_t ellipse){
	double dist1 = sqrt((point.x - ellipse.f1_x) * (point.x - ellipse.f1_x) +
		(point.y - ellipse.f1_y) * (point.y - ellipse.f1_y));
	double dist2 = sqrt((point.x - ellipse.f2_x) * (point.x - ellipse.f2_x) +
		(point.y - ellipse.f2_y) * (point.y - ellipse.f2_y));
	double max;
	if (ellipse.majorAxis > ellipse.minorAxis)
		max = ellipse.majorAxis;
	else
		max = ellipse.minorAxis;
	if (dist1 + dist2 <= 2 * max)
		return true;
	else
		return false;
}

CvSeq* pointsNearEllipseBox(CvSeq* seq, CvBox2D ellipseBox, CvMemStorage* mem_storage){
	
	double sin_theta = sin(ellipseBox.angle / 180 * M_PI);
	double cos_theta = cos(ellipseBox.angle / 180 * M_PI);

	double ellipse_center_x = ellipseBox.center.x;
	double ellipse_center_y = ellipseBox.center.y;

	double ellipse_a_squared = (ellipseBox.size.width / 2) * (ellipseBox.size.width / 2);
	double ellipse_b_squared = (ellipseBox.size.height / 2) * (ellipseBox.size.height / 2);

	CvSeq* result_set = cvCreateSeq(seq->flags, seq->header_size, sizeof(CvPoint), mem_storage);

	CvSeqReader reader;
	CvPoint point;
	float rotated_x, rotated_y, result;
	
	cvStartReadSeq(seq, &reader);
	
	for (int i = 0; i < seq->total; i++) {
		CV_READ_SEQ_ELEM(point, reader);

		rotated_x = (point.x - ellipse_center_x)*cos_theta + (point.y - ellipse_center_y)*sin_theta;
		rotated_y = (point.x - ellipse_center_x)*-sin_theta + (point.y - ellipse_center_y)*cos_theta;

		result = (rotated_x*rotated_x) / ellipse_a_squared + (rotated_y*rotated_y) / ellipse_b_squared - 1;

		if (result > -0.08 && result < 0.08) 
			cvSeqPush(result_set, &point);
	}

	return result_set;
}

Ellipse_t fitEllipseRANSAC(CvSeq* contour, IplImage* image, int imageIndex) {
	
	Ellipse_t ellipse;
	int count = 0;

	int randomSampleIndices[5];
	CvPoint points[5];

	// Pick 5 random values
	for (int i = 0; i < 5; i++) {
		bool duplicate = false;
		int randomVal;
		do {
			randomVal = rand() % contour->total;
			for (int j = 0; j < i; j++) {
				duplicate |= randomSampleIndices[i] == randomVal;
			}
		} while (duplicate);
		randomSampleIndices[i] = randomVal;

		points[i] = *(CvPoint*)cvGetSeqElem(contour, randomVal);

		if (image) {
			for (int ii = -2; ii < 3; ii++) {
				for (int jj = -2; jj < 3; jj++) {
					cvSet2D(image, points[i].y + ii, points[i].x + jj, cvScalar(255));
				}
			}
		}
	}

	double aData[] = {
		points[0].x * points[0].x, 2 * points[0].x * points[0].y, points[0].
		y * points[0].y, 2 * points[0].x, 2 * points[0].y,

		points[1].x * points[1].x, 2 * points[1].x * points[1].y, points[1].
		y * points[1].y, 2 * points[1].x, 2 * points[1].y,

		points[2].x * points[2].x, 2 * points[2].x * points[2].y, points[2].
		y * points[2].y, 2 * points[2].x, 2 * points[2].y,

		points[3].x * points[3].x, 2 * points[3].x * points[3].y, points[3].
		y * points[3].y, 2 * points[3].x, 2 * points[3].y,

		points[4].x * points[4].x, 2 * points[4].x * points[4].y, points[4].
		y * points[4].y, 2 * points[4].x, 2 * points[4].y };

	CvMat matA = cvMat(5, 5, CV_64F, aData);
	CvMat *D, *U, *V;
	D = cvCreateMat(5, 5, CV_64F);
	U = cvCreateMat(5, 5, CV_64F);
	V = cvCreateMat(5, 5, CV_64F);

	cvSVD(&matA, D, U, V, CV_SVD_MODIFY_A);

	double a, b, c, d, f, g;
	a = cvmGet(V, 0, 4);
	b = cvmGet(V, 1, 4);
	c = cvmGet(V, 2, 4);
	d = cvmGet(V, 3, 4);
	f = cvmGet(V, 4, 4);
	g = 1;

	getEllipseParam(a, b, c, d, f, g, ellipse);

	CvSeqReader reader;
	CvPoint point;
	cvStartReadSeq(contour, &reader);
	if (ellipse.majorAxis > 0 && ellipse.minorAxis > 0){
		for (int i = 0; i < contour->total; i++) {
			CV_READ_SEQ_ELEM(point, reader);
			if (pointInEllipse(point, ellipse)){
				count++;
			}
		}
	}

	ellipse.fitCount = count;

	return ellipse;
}

CvSeq* fitEllipseRANSAC2(CvSeq* contour, IplImage* image, CvBox2D &ellipse, CvMemStorage* memStorage) {

	int randomSampleIndices[10];
	CvSeq* subContour = cvCreateSeq(contour->flags, contour->header_size, sizeof(CvPoint), memStorage);

	// Pick 5 random values
	for (int i = 0; i < 10; i++) {
		bool duplicate = false;
		int randomVal;
		do {
			randomVal = rand() % contour->total;
			for (int j = 0; j < i; j++) {
				duplicate |= randomSampleIndices[i] == randomVal;
			}
		} while (duplicate);
		randomSampleIndices[i] = randomVal;

		cvSeqPush(subContour, (CvPoint*)cvGetSeqElem(contour, randomVal));

		/*if (image) {
			CvPoint point = *(CvPoint*)cvGetSeqElem(contour, randomVal);
			for (int ii = -2; ii < 3; ii++) {
				for (int jj = -2; jj < 3; jj++) {
					cvSet2D(image, point.y + ii, point.x + jj, cvScalar(255));
				}
			}
		}*/
	}

	ellipse = cvFitEllipse2(subContour);
	
	//cvEllipseBox(image, ellipse, CV_RGB(255, 255, 255));

	return pointsNearEllipseBox(contour, ellipse, memStorage);
}

CvBox2D ellipseFit(CvSeq* contour, IplImage* image, int imageIndex) {

	Ellipse_t ellipse;
	Ellipse_t best_ellipse;

	best_ellipse.fitCount = 0;

	for (int i = 0; i < 1; i++) {
		ellipse = fitEllipseRANSAC(contour, image, imageIndex);
		if (ellipse.fitCount > best_ellipse.fitCount) 
			best_ellipse = ellipse;
	} 

	CvBox2D result;
	result.size.width = 0;

	if (best_ellipse.fitCount > 0) {
		result.center = { best_ellipse.x, best_ellipse.y };
		result.size = { best_ellipse.majorAxis, best_ellipse.minorAxis };
		result.angle = best_ellipse.angle;
	}

	return result;
}

CvBox2D ellipseFit2(CvSeq* contour, IplImage* image, CvMemStorage* memStorage, CvBox2D* prevPupilBoxes) {

	CvBox2D ellipse;
	CvBox2D best_ellipse;

	int best_count = 0;
	CvSeq* best_sequence = NULL;
	CvSeq* current_sequence;
	float aspect_score;

	float filtered_aspect_ratio = 0;
	float filtered_width = 0;
	float filtered_height = 0;
	float filtered_angle = 0;

	float sin_theta, cos_theta, width, height, aspect_ratio;

	/*if (!prevPupilBoxes) {
		filtered_aspect_ratio = 1.0;
		filtered_width = 50;
		filtered_height = 50;
	}
	else {
		int denom = 0;
		int scale;
		for (int i = 0; i < 10; i++) {
			if (prevPupilBoxes[i].size.width != 0) {

				sin_theta = sin(prevPupilBoxes[i].angle / 180 * M_PI);
				cos_theta = cos(prevPupilBoxes[i].angle / 180 * M_PI);

				scale = i + 1;

				width = scale* MAX(prevPupilBoxes[i].size.width / 2 * cos_theta, prevPupilBoxes[i].size.height / 2 * sin_theta);
				height = scale * MAX(prevPupilBoxes[i].size.width / 2 * sin_theta, prevPupilBoxes[i].size.height / 2 * cos_theta);
				aspect_ratio = scale * (width > height ? width / height : height / width);

				filtered_width += width;
				filtered_height += height;
				filtered_aspect_ratio += aspect_ratio;
				denom += scale;
			}
		}

		if (denom > 0) {
			filtered_width /= denom;
			filtered_height /= denom;
			filtered_aspect_ratio /= denom;
			filtered_angle /= denom;
		}
		else {
			filtered_aspect_ratio = 1.0;
			filtered_width = 50;
			filtered_height = 50;
			filtered_angle = 90;
		}
	}*/

	/*if (prevPupilBoxes) {
		for (int i = 0; i < 10; i++) {
			printf("%f ", prevPupilBoxes[i].size.width);
		}
		printf("\n");
	}*/

	for (int i = 0; i < 100; i++) {
		current_sequence = fitEllipseRANSAC2(contour, image, ellipse, memStorage);
		
		sin_theta = sin(ellipse.angle / 180 * M_PI);
		cos_theta = cos(ellipse.angle / 180 * M_PI);

		width = MAX(ellipse.size.width/2 * cos_theta, ellipse.size.height/2 * sin_theta);
		height = MAX(ellipse.size.width/2 * sin_theta, ellipse.size.height/2 * cos_theta);
		aspect_ratio = width > height ? width / height : height / width;
		
		aspect_score = 1;
		//aspect_score -= 1 * fabs((width / filtered_width) - 1);
		//aspect_score -= 1 * fabs((height / filtered_height) - 1);
		//aspect_score -= 1 * fabs((aspect_ratio / filtered_aspect_ratio) - 1);

		//aspect_score -= 1 * fabs((ellipse.size.width / ellipse.size.height) - 1);

		aspect_ratio = ellipse.size.width / ellipse.size.height;

		if (current_sequence->total * aspect_score > best_count && aspect_ratio > 0.5 && aspect_ratio < 1.5) {
			best_sequence = current_sequence;
			best_count = current_sequence->total;
			best_ellipse = ellipse;
		}
	}

	ellipse.size = { 0.0, 0.0 };
	if (best_sequence && best_count >= 5) {
		cvDrawContours(image, best_sequence, CV_RGB(255, 255, 255), CV_RGB(128, 128, 128), 0);
		
		/*CvPoint2D32f prev_point, test_point;
		prev_point = *((CvPoint2D32f*)cvGetSeqElem(best_sequence, 0));
		for (int i = 1; i < best_sequence->total; i++) {
			test_point = *((CvPoint2D32f*)cvGetSeqElem(best_sequence, i));
			cvLine(image, { prev_point.x * 1024, prev_point.y * 1024 }, { test_point.x * 1024, test_point.y * 1024 }, cvScalar(255), 1, CV_AA, 10);
			prev_point = test_point;
			//cvSet2D(sourceImage, test_point.y, test_point.x, cvScalar(100));
		}*/

		ellipse = cvFitEllipse2(best_sequence);
		//ellipse = best_ellipse;
	}

	return ellipse;
}

CvBox2D Trackster::findBounds(IplImage* image, CvPoint2D32f nearestTo, float targetArea, CvBox2D* maskArea) {
	CvPoint offset = cvPoint(0, 0);

	cvFindContours(image,
		mem_storage,
		&contour,
		sizeof(CvContour),
		CV_RETR_EXTERNAL,          // external contour only
		CV_CHAIN_APPROX_NONE,       // no vert or hor segment contraction
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

	CvBox2D best_box;

	best_box.size = { 0.0, 0.0 };


	if (best_score < 10) {

		best_contour->h_next = NULL;

		if (maskArea != NULL) {
			bool debugBoxExclusion = TRUE;
			IplImage* currentImage = (debugBoxExclusion && pendingSnapshot) ? image : NULL;
			best_contour = removeBoxInclusionsFromSequence(best_contour, maskArea, mem_storage, currentImage);
			if (pendingSnapshot && currentImage) cvCopy(currentImage, test_snapshot_image[test_snapshot_image_index++]);
		}

		//CvMat* covMatrix = cvCreateMat(n, n, CV_32FC1);;
		//CvMat* mean = NULL;
		//CvMat* best_cont_array = cvCreateMat(best_contour->total, 2, CV_32S);
		//CvPoint* PointArray = (CvPoint *)malloc(best_contour->total * sizeof(CvPoint));
		//cvCvtSeqToArray(best_contour, best_cont_array->data.ptr);
		//cvCvtSeqToArray(best_contour, PointArray);
		//cvCalcCovarMatrix(best_cont_array->data, 1, covMatrix, mean, 0);

		//CvSeq* hull = cvConvexHull2(best_contour, mem_storage, CV_CLOCKWISE, 1);

		cvSet(image, cvScalar(0));
		if (best_contour->total > 5) {
			//cvDrawContours(image, best_contour, CV_RGB(255, 255, 255), CV_RGB(128, 128, 128), 0);

			CvBox2D* pupilBoxHistory = (frameCount > 20) ? prevPupilBoxes : NULL;
			if (maskArea != NULL) best_box = ellipseFit2(best_contour, image, mem_storage, pupilBoxHistory);

			// FALLBACK FOR NOW
			if (maskArea == NULL) // || best_box.size.width == 0)
				best_box = cvFitEllipse2(best_contour);
		}

		if (pendingSnapshot) cvCopy(image, test_snapshot_image[test_snapshot_image_index++]);
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

float Trackster::GetAveragePrecision() {
	return 0;
}

float Trackster::GetAverageAccuracy() {
	return 0;
}

void Trackster::PrepareAVI() {

	if (!m_hCam) return;

	INT ret;

	/*ret = isavi_InitAVI(&m_hAVI, m_hCam);

	ret = isavi_SetImageSize(m_hAVI, IS_AVI_CM_Y8, size.width, size.height, 0, 0, 0);
	ret = isavi_OpenAVI(m_hAVI, "test.avi");
	ret = isavi_StartAVI(m_hAVI);*/

	char filename[32];
	sprintf_s(filename, "test_uc%d.avi", m_AVIIndex++);
	h_AVIwriter = cvCreateVideoWriter(filename, CV_FOURCC('D', 'I', 'B', ' '), 25, size, 0);
}

void Trackster::CloseAVI() {
	
	/*
	if (!m_hAVI) return;

	INT ret;
	ret = isavi_StopAVI(m_hAVI);
	ret = isavi_CloseAVI(m_hAVI);
	ret = isavi_ExitAVI(m_hAVI);*/

	if (!h_AVIwriter) return;
	cvReleaseVideoWriter(&h_AVIwriter);
	h_AVIwriter = NULL;
}

