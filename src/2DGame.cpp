#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>

const int SCREEN_WIDTH = 768;
const int SCREEN_HEIGHT = 384;
const int ANIMATION_FRAMES = 6;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int BG_ANIMATION_FRAMES = 7;

class LTexture
{
	SDL_Texture* mTexture;
	int mHeight;
	int mWidth;

public:
	LTexture();
	~LTexture();

	bool loadFromFile(std::string path);
	#if defined(SDL_TTF_MAJOR_VERSION)
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
	#endif
	void free();
	void render(int x, int y, SDL_Rect* clip);
	int getWidth();
	int getHeight();
};

class LTimer
{
	Uint32 mStartTicks;
	Uint32 mPausedTicks;
bool mStarted;
bool mPaused;

	public:
		LTimer();
		void start();
		void stop();
		void pause();
		void unpause();
		Uint32 getTicks();
		bool isStarted();
		bool isPaused();
};


bool init();
bool loadMedia();
void close();

TTF_Font* gFont = NULL;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Rect gSprites[ANIMATION_FRAMES];
SDL_Rect gSpritesBG[BG_ANIMATION_FRAMES];

LTexture gBG_Texture;
LTexture gSpriteSheetTexture;
LTexture gFPSTexture;

Mix_Music* gMusic = NULL;
Mix_Chunk* gPunch = NULL;
Mix_Chunk* gKick = NULL;
Mix_Chunk* gHadoken = NULL;
Mix_Chunk* gShoryuken = NULL;

LTimer::LTimer()
{
	mStarted = false;
	mPaused = false;

	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::start()
{
	mStarted = true;
	mPaused = false;

	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop()
{
	mStarted = false;
	mPaused = false;

	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause()
{
	if (mStarted && !mPaused)
	{
		mPaused = true;

		mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
	}
}

void LTimer::unpause()
{
	if (mStarted && mPaused)
	{
		mPaused = false;
		mStartTicks = SDL_GetTicks() - mPausedTicks;
		mPausedTicks = 0;
	}
}

Uint32 LTimer::getTicks()
{
	Uint32 time = 0;
	if (mStarted)
	{
		if (mPaused)
		{
			time = mPausedTicks;
		}
		else
		{
			time = SDL_GetTicks() - mStartTicks;
		}
	}
	return time;
}

bool LTimer::isPaused()
{
	return mPaused && mStarted;
}

bool LTimer::isStarted()
{
	return mStarted;
}


LTexture::LTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	free();
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0, 0));
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}
	mTexture = newTexture;
	return mTexture != NULL;

}
#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	free();
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		SDL_FreeSurface(textSurface);
	}
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = { x,y,mWidth,mHeight };
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopy(gRenderer, mTexture, clip, &renderQuad);
}

int LTexture::getHeight()
{
	return mHeight;
}

int LTexture::getWidth()
{
	return mWidth;
}

bool init()
{
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		
		gWindow = SDL_CreateWindow("Fight!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
				if (TTF_Init() == -1)

				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					success = false;
				}
	
			}
		}
	}
	return success;
}

bool loadMedia()
{
	bool success = true;
	gMusic = Mix_LoadMUS("Ken_Theme.mp3");
	if (gMusic == NULL)
	{
		printf("Failed to load theme music! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	gPunch = Mix_LoadWAV("Punch.mp3");
	if (gPunch == NULL)
	{
		printf("Failed to load punch sound! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	gKick = Mix_LoadWAV("Kick.mp3");
	if (gKick == NULL)
	{
		printf("Failed to load kick sound! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	gHadoken = Mix_LoadWAV("Hadoken.mp3");
	if (gHadoken == NULL)
	{
		printf("Failed to load hadoken sound! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	gShoryuken = Mix_LoadWAV("Shoriuken-.mp3");
	if (gShoryuken == NULL)
	{
		printf("Failed to load Shoryuken sound! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	if (!gSpriteSheetTexture.loadFromFile("spritesheet.png"))
	{
		printf("Failed to load Foo texture image!\n");
		success = false;
	}

	if (!gBG_Texture.loadFromFile("bgSprite.png"))
	{
		printf("Failed to load BackGround texture image!\n");
		success = false;
	}
	gFont = TTF_OpenFont("Inktype.ttf", 16);
	if (gFont == NULL)
	{
		printf("Failed to load Sans font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false;
	}
	else
	{
		for (int i = 0; i < ANIMATION_FRAMES; ++i)
		{
			gSprites[i].x = i * 107;
			gSprites[i].y = 0;
			gSprites[i].w = 106;
			gSprites[i].h = 200;
		}

		for (int i = 0; i < BG_ANIMATION_FRAMES; ++i)
		{
			gSpritesBG[i].x = i * 768;
			gSpritesBG[i].y = 0;
			gSpritesBG[i].w = 768;
			gSpritesBG[i].h = 384;
		}
	}
	return success;
}

void close()
{
	gFPSTexture.free();
	TTF_CloseFont(gFont);
	gFont = NULL;
	gBG_Texture.free();
	gSpriteSheetTexture.free();

	Mix_FreeChunk(gKick);
	Mix_FreeChunk(gPunch);
	Mix_FreeChunk(gHadoken);
	Mix_FreeChunk(gShoryuken);
	gKick = NULL;
	gPunch = NULL;
	gHadoken = NULL;
	gShoryuken = NULL;

	Mix_FreeMusic(gMusic);
	gMusic = NULL;


	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gRenderer = NULL;
	gWindow = NULL;

	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			bool quit = false;
			SDL_Event e;
			int frame = 0;
			int bgFrame = 0;
			SDL_Color textColor = { 255,255,255,255 };
			LTimer fpsTimer;
			LTimer capTimer;
			std::stringstream timeText;
			Mix_PlayMusic(gMusic, -1);
			int countedFrames = 0;
			fpsTimer.start();
			while (!quit)
			{
				capTimer.start();

				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					else if (e.type == SDL_KEYDOWN)
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_j:
								Mix_PlayChannel(-1, gPunch, 0);
								break;

							case SDLK_u:
								Mix_PlayChannel(-1, gKick, 0);
								break;
								
							case SDLK_l:
								Mix_PlayChannel(-1, gHadoken, 0);
								break;

							case SDLK_o:
								Mix_PlayChannel(-1, gShoryuken, 0);
								break;

							case SDLK_RETURN:
								if (Mix_PlayingMusic() == 0)
								{
									Mix_PlayMusic(gMusic, -1);
								}
								else
								{
									if (Mix_PausedMusic() == 1)
									{
										Mix_ResumeMusic();
									}
									else
									{
										Mix_PauseMusic();
									}
								}
								break;

							case SDLK_BACKSPACE:
								Mix_HaltMusic();
								break;
						}
					}
				}
				float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
				if (avgFPS > 500000)
				{
					avgFPS = 0;
				}
				timeText.str("");
				timeText << "FPS: " << avgFPS;
				if (!gFPSTexture.loadFromRenderedText(timeText.str().c_str(), textColor))
				{
					printf("Unable to render FPS texture!\n");
				}
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				SDL_Rect* currentBGClip = &gSpritesBG[bgFrame / 7];
				gBG_Texture.render(0,0, currentBGClip);

				SDL_Rect* currentClip = &gSprites[frame / 4];
				gSpriteSheetTexture.render((SCREEN_WIDTH - currentClip->w) / 4 -50, (SCREEN_HEIGHT - currentClip->h) , currentClip);
				gFPSTexture.render(0, 0,0);

				SDL_RenderPresent(gRenderer);

				++countedFrames;
				int frameTicks = capTimer.getTicks();
				if (frameTicks < SCREEN_TICKS_PER_FRAME)
				{
					SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
				}
				++bgFrame;
				if (bgFrame / 7 >= BG_ANIMATION_FRAMES)
				{
					bgFrame = 0;
				}
				++frame;
				if (frame / 4 >= ANIMATION_FRAMES)
				{
					frame = 0;
				}
			}
		}
	}
	close();
	return 0;
}