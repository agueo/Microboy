#include <array>
#include "Window.h"

Window::Window() {
	window.create(sf::VideoMode(600, 800), "Microboy");
}

void Window::update() {
}

void Window::draw(const uint32_t *data) {
}
