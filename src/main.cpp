// STL
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
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
	bool rom_loaded{ false };
	bool draw_frame { false };
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
		bus->load_cart(system_load_rom("./roms/dmg-acid2.gb"));
	}
	rom_loaded = true;

	// game loop
	while (running) {
		while (game_window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running = false;
				break;
			}
			switch (event.type) {
				case sf::Event::KeyPressed:
					handle_key_pressed(event, joypad);
					break;
				case sf::Event::KeyReleased:
					handle_key_released(event, joypad);
					break;
				default:
					break;
			}
		}

		// run gameboy only if game is loaded
		if (!rom_loaded) {
			fmt::print("Failed to run because no game loaded!");
			break;
		}

		int cycle_count = 0;
		
		while (cycle_count < dmg::CYCLES_PER_FRAME) {
			int cycles_ran = cpu.step(dmg::CYCLE_STEP);
			cycle_count += cycles_ran;
			timer->step(cycles_ran);
			draw_frame = ppu->step(cycles_ran);
			if (draw_frame) break;
		}
		cycle_count = 0;
		// Render
		if (draw_frame) {
			game_window.clear();
			// update texture
			sf::Image bg;
			sf::Texture bg_texture;
			sf::Sprite bgsprite;

			bg.create(dmg::WIDTH, dmg::HEIGHT, (const uint8_t *)ppu->get_frame_buffer());
			bg_texture.loadFromImage(bg);
			bgsprite.setTexture(bg_texture);
			game_window.draw(bgsprite);
			game_window.display();
			draw_frame = false;
		} else {
			sf::sleep(sf::milliseconds(10));
		}
	}

	game_window.close();
	return 0;
}
