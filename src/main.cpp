// STL
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

// external dependencies
#include <SFML/Graphics.hpp>
#include <fmt/core.h>

// dmg Headers
#include "Cartridge.h"
#include "Cpu.h"
#include "MemoryBus.h"
#include "InterruptObserver.h"
#include "JoyPad.h"
#include "Ppu.h"
#include "Timer.h"
#include "Window.h"


int main(int argc, char **argv) {
	 
	// initialize game window
	sf::RenderWindow game_window;
	game_window.create(sf::VideoMode(dmg::WIDTH, dmg::HEIGHT), "Microboy");
	game_window.setSize(sf::Vector2u(dmg::WIDTH * dmg::SCALE, dmg::HEIGHT * dmg::SCALE));

	// dmg objects
	Cpu cpu{};
	std::shared_ptr<MemoryBus> bus = std::make_shared<MemoryBus>();
	std::shared_ptr<InterruptObserver> int_obs = std::make_shared<InterruptObserver>();
	std::shared_ptr<JoyPad> joypad = std::make_shared<JoyPad>();
	std::shared_ptr<Timer> timer = std::make_shared<Timer>();
	std::shared_ptr<Ppu> ppu = std::make_shared<Ppu>();

	cpu.connect_bus(bus);
	ppu->connect_bus(bus);
	bus->connect_joypad(joypad);
	bus->connect_timer(timer);
	bus->connect_ppu(ppu);

	bus->connect_interrupt_observer(int_obs);
	joypad->connect_interrupt_observer(int_obs);
	timer->connect_interrupt_observer(int_obs);
	ppu->connect_interrupt_observer(int_obs);

	
	// control flags
	bool running{ true };
	bool rom_loaded{ true };
    bool draw_frame {false};
	sf::Event event;

	// reset the system
	cpu.reset();
	bus->reset();
	joypad->reset();
	int_obs->reset();
	timer->reset();
	if (argc > 1) { 
		const std::string rom_name {argv[1]};
		bus->load_cart(system_load_rom(rom_name));
	} else {
		bus->load_cart(system_load_rom("./roms/tetris.gb"));
	}

	// game loop
	while (running) {
		while (game_window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					game_window.close();
					running = false;
					break;
				case sf::Event::KeyPressed:
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
					break;
				case sf::Event::KeyReleased:
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
					break;
				default:
					break;
			}
		}

		// Render

		// run gameboy only if game is loaded
		if (rom_loaded) {
			int cycles_ran = cpu.step(dmg::CYCLES_PER_FRAME);
			timer->step(cycles_ran);
			draw_frame = ppu->step(cycles_ran);
		}
		if (draw_frame) {
			game_window.clear();
			// update texture
			sf::Image bg;
			sf::Texture bg_texture;
			sf::Sprite bgsprite;

			bg.create(dmg::WIDTH, dmg::HEIGHT,  (const uint8_t *)ppu->get_frame_buffer());
			bg_texture.loadFromImage(bg);
			bgsprite.setTexture(bg_texture);
			game_window.draw(bgsprite);
			game_window.display();
			draw_frame = false;
		}

	}

	return 0;
}
