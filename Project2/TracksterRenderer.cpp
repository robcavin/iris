#include "TracksterRenderer.h"
#include <stdio.h>

#define SCREEN_WIDTH  320+1000
#define SCREEN_HEIGHT 500+1000

bool TracksterRenderer::init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow("IRIS", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, 
			SCREEN_HEIGHT,
			SDL_WINDOW_SHOWN);
		
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Get window surface
			gScreenSurface = SDL_GetWindowSurface(gWindow);

			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}

			gEyeTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
			gWorkingTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);

			for (int i = 0; i < NUM_TEST_IMAGES; i++) {
				gTestImageTexture[i] = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
			}

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 255);
			SDL_RenderClear(gRenderer);
		}
	}

	return success;
}

SDL_Texture* TracksterRenderer::loadTexture(char* path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

void TracksterRenderer::updateTexture(SDL_Texture* texture, IplImage* image) {
	
	if (!image) return;

	char* sourceGrayscalePixels;
	Uint32* pixels;

	int pitch;
	int success = SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

	pitch /= sizeof(Uint32);

	for (int i = 0; i < 240; i++) {
		for (int j = 0; j < 320; j++) {
			unsigned char grayscale = image->imageData[i*pitch + j];
			int color = 0xff000000 | (grayscale << 16) | (grayscale << 8) | grayscale;
			pixels[i*pitch + j] = color;
		}
	}

	SDL_UnlockTexture(texture);
}

void TracksterRenderer::render() {
	int success;

	hTrackster->UpdateImageSnapshots();

	//Render texture to screen
	SDL_Rect rect = { 1000, 0, 320, 240 };
	updateTexture(gEyeTexture, hTrackster->GetEyeImage());
	success = SDL_RenderCopy(gRenderer, gEyeTexture, NULL, &rect);
	
	rect = { 1000, 240, 320, 240 };
	updateTexture(gWorkingTexture, hTrackster->GetWorkingImage());
	success = SDL_RenderCopy(gRenderer, gWorkingTexture, NULL, &rect);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			int index = i * 4 + j;

			if (index >= NUM_TEST_IMAGES) break;

			rect = { 330 * j, 1000 + (250 * i), 320, 240 };
			updateTexture(gTestImageTexture[index], hTrackster->GetTestImage(index));
			success = SDL_RenderCopy(gRenderer, gTestImageTexture[index], NULL, &rect);
		}
	}

	if (hTrackster->trained) {

		// Set the tracker to update at the same rate as the renderer
		hTrackster->waitForUpdateRequest = true;

		CvPoint2D32f target = hTrackster->GetProjection();
		success = SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(gRenderer, target.x - 5, target.y - 5, target.x + 5, target.y + 5);
		SDL_RenderDrawLine(gRenderer, target.x - 5, target.y + 5, target.x + 5, target.y - 5);
	}

	if (displayStaticCrosshair) {
		success = SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(gRenderer, 
			staticCrosshairCoord.x - 5, staticCrosshairCoord.y - 5, 
			staticCrosshairCoord.x + 5, staticCrosshairCoord.y + 5);

		SDL_RenderDrawLine(gRenderer, 
			staticCrosshairCoord.x - 5, staticCrosshairCoord.y + 5, 
			staticCrosshairCoord.x + 5, staticCrosshairCoord.y - 5);
	}

	//Update screen
	SDL_RenderPresent(gRenderer);
}

void TracksterRenderer::close()
{
	//Free loaded image
	SDL_DestroyTexture(gEyeTexture);
	gEyeTexture = NULL;

	//Destroy window    
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();

}