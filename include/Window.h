#ifndef WINDOW_H
#define WINDOW_H

#include <memory>

#include <SFML/Graphics.hpp>
#include "common.h"
#include "JoyPad.h"

void handle_key_pressed(sf::Event &event, std::shared_ptr<JoyPad> joypad);
void handle_key_released(sf::Event &event, std::shared_ptr<JoyPad> joypad);

#endif