#include <array>
#include "Window.h"

Window::Window() : window(nullptr), renderer(nullptr), texture(nullptr) {
	window = SDL_CreateWindow("MicroBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
}

Window::~Window() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int Window::init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Error: Failed to initialize SDL %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

void Window::update() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
}

void Window::draw(const uint32_t *data) {
		SDL_UpdateTexture(texture, NULL, data, WIDTH);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
}
