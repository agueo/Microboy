#include "Window.h"

void handle_key_pressed(sf::Event &event, std::shared_ptr<JoyPad> joypad) {
	switch (event.key.code) {
	case sf::Keyboard::W:
		joypad->handle_press(JoyPadInput::UP);
		break;
	case sf::Keyboard::A:
		joypad->handle_press(JoyPadInput::LEFT);
		break;
	case sf::Keyboard::S:
		joypad->handle_press(JoyPadInput::DOWN);
		break;
	case sf::Keyboard::D:
		joypad->handle_press(JoyPadInput::RIGHT);
		break;
	case sf::Keyboard::LControl:
		joypad->handle_press(JoyPadInput::START);
		break;
	case sf::Keyboard::Space:
		joypad->handle_press(JoyPadInput::SELECT);
		break;
	case sf::Keyboard::J:
		joypad->handle_press(JoyPadInput::A);
		break;
	case sf::Keyboard::K:
		joypad->handle_press(JoyPadInput::B);
		break;
	default:
		break;
	}
}

void handle_key_released(sf::Event &event, std::shared_ptr<JoyPad> joypad) {
	switch (event.key.code) {
	case sf::Keyboard::W:
		joypad->handle_press(JoyPadInput::UP);
		break;
	case sf::Keyboard::A:
		joypad->handle_press(JoyPadInput::LEFT);
		break;
	case sf::Keyboard::S:
		joypad->handle_press(JoyPadInput::DOWN);
		break;
	case sf::Keyboard::D:
		joypad->handle_press(JoyPadInput::RIGHT);
		break;
	case sf::Keyboard::LControl:
		joypad->handle_press(JoyPadInput::START);
		break;
	case sf::Keyboard::Space:
		joypad->handle_press(JoyPadInput::SELECT);
		break;
	case sf::Keyboard::J:
		joypad->handle_press(JoyPadInput::A);
		break;
	case sf::Keyboard::K:
		joypad->handle_press(JoyPadInput::B);
		break;
	default:
		break;
	}
}
