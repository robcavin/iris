#ifndef __SDLMANAGER_H__
#define __SDLMANAGER_H__

#include <opencv2\opencv.hpp>
#include "Trackster.h"
#include "SDL.h"

class TracksterRenderer {

	//The window we'll be rendering to
	SDL_Window* gWindow = NULL;

	//The surface contained by the window
	SDL_Surface* gScreenSurface = NULL;

	//The window renderer
	SDL_Renderer* gRenderer = NULL;

	SDL_Texture* gEyeTexture = NULL;
	SDL_Texture* gWorkingTexture = NULL;

	SDL_Texture* gTestImageTexture[NUM_TEST_IMAGES];

	SDL_Texture* loadTexture(char* path);
	
	void updateTexture(SDL_Texture* texture, IplImage* image);

public:
	Trackster* hTrackster;

	bool displayStaticCrosshair;
	CvPoint2D32f staticCrosshairCoord;

	bool init();
	void close();

	virtual void render();
};

#endif
