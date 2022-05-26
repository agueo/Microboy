// STL
#include <array>
#include <cassert>
#include <iostream>
#include <random>

// external dependencies
#include <SDL.h>
#include <fmt/core.h>

// DMG Headers
#include "Cartridge.h"
#include "Cpu.h"
#include "MemoryBus.h"
#include "Window.h"

int main(int, char **) {
	
	// initialize game window
	Window game_window{};
	if (game_window.init() < 0) {
		fmt::print("Failed to initialize window!");
		return -1;
	}

	// dummy pixel data
	std::array<uint32_t, WIDTH*HEIGHT> pixels {};
	auto reset_screen = [](std::array<uint32_t, WIDTH*HEIGHT> &p) {
		std::random_device r;
		std::default_random_engine e1(r());
		std::uniform_int_distribution<uint32_t> uniform_dist(0x000000FF, 0xFFFFFFFF);
		for (auto &pixel : p) {
			pixel = uniform_dist(e1);
		}
	};
	reset_screen(pixels);
	
	// DMG objects
	Cpu cpu{};
	cpu.reset();
	std::shared_ptr<MemoryBus> bus = std::make_shared<MemoryBus>();
	cpu.connect_bus(bus);
	cpu.read_byte(B);

	// control flags
	bool running{ true };
	bool rom_loaded{ false };
	SDL_Event event{};

	// game loop
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_DROPFILE) {
				// Handle drag-and-drop rom file 
				// meaning open the rom file and start the system
				cpu.reset();
				bus->reset();
				
				char *dropped_file = event.drop.file;
				std::string rom_name{ dropped_file };
				SDL_free(dropped_file);
				fmt::print("Loading rom: {}\n", rom_name);
				auto cart = system_load_rom(rom_name);
				SDL_assert(cart != nullptr);
				bus->load_cart(std::move(cart));
				rom_loaded = true;
			}
		}

		// Render
		game_window.update();

		// run gameboy only if game is loaded
		if (rom_loaded) {
			cpu.step(1);
			// TODO ppu step
			// TODO apu step
		}

		// Render
		game_window.draw(pixels.data()); // TODO - send the ppu data instead
	}
	return 0;
}