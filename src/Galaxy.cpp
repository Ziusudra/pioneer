#include "libs.h"
#include "Galaxy.h"
#include "Pi.h"
#include "Sector.h"

namespace Galaxy {

// lightyears
const float GALAXY_RADIUS = 50000.0;
const float SOL_OFFSET_X = 25000.0;
const float SOL_OFFSET_Y = 0.0;

static SDL_Surface *s_galaxybmp;

void Init() 
{
	s_galaxybmp = IMG_Load("data/galaxy.png");
	if (!s_galaxybmp) {
		fprintf(stderr, "Could not open data/galaxy.png");
		Pi::Quit();
	}
}

const SDL_Surface *GetGalaxyBitmap()
{
	return s_galaxybmp;
}

Uint8 GetSectorDensity(int sx, int sy)
{
	// -1.0 to 1.0
	float offset_x = (sx*Sector::SIZE + SOL_OFFSET_X)/GALAXY_RADIUS;
	float offset_y = (-sy*Sector::SIZE + SOL_OFFSET_Y)/GALAXY_RADIUS;
	// 0.0 to 1.0
	offset_x = CLAMP((offset_x + 1.0)*0.5, 0.0, 1.0);
	offset_y = CLAMP((offset_y + 1.0)*0.5, 0.0, 1.0);

	int x = (int)floor(offset_x * (s_galaxybmp->w - 1));
	int y = (int)floor(offset_y * (s_galaxybmp->h - 1));

	SDL_LockSurface(s_galaxybmp);
	Uint8 val = ((char*)s_galaxybmp->pixels)[x + y*s_galaxybmp->pitch];
	SDL_UnlockSurface(s_galaxybmp);
	return val;
}

} /* namespace Galaxy */