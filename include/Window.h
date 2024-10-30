#ifndef WINDOW_H
#define WINDOW_H

#include <SFML/Graphics.hpp>
#include "common.h"

class Window {
public:
	Window();
	~Window() = default;
	void update();
	void draw(const uint32_t *data);

private:
	sf::Window window;
};

#endif