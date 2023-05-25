#pragma once
#include <SDL.h>
#include "common.h"

class Window
{
public:
	Window();
	~Window();
	int init();
	void update();
	void draw(const uint32_t *data);

private:
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
};

