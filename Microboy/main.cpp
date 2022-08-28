// STL
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

// external dependencies
#include <SDL.h>
#include <fmt/core.h>

// DMG Headers
#include "Cartridge.h"
#include "Cpu.h"
#include "MemoryBus.h"
#include "InterruptObserver.h"
#include "JoyPad.h"
#include "Timer.h"
#include "Window.h"

const std::vector<uint32_t> gPalette{0x89d795FF, 0x629A6AFF, 0x3B5C40FF, 0x141F15FF};
std::vector<uint8_t> calculate_pixel_line(uint8_t byte1, uint8_t byte2) {
	// start from the left using byte2 as msb
	std::vector<uint8_t> line(8);
	int j = 0;
	for (int i = 7; i >= 0; --i) {
		line[j] = (((byte2 >> i) & 0x1) << 1) | ((byte1 >> i) & 0x1);
		++j;
	}
	return line;
}
void fill_tile(std::vector<uint32_t> &pixels, const uint8_t *data, int offset, int tile_x, int tile_y) {
	int line_num = 0;
	for (int y = (tile_y * 8); y < 8 + (tile_y * 8); ++y) {
		auto line = calculate_pixel_line(data[offset + line_num], data[offset + line_num + 1]);
		for (int x = (tile_x * 8); x < 8 + (tile_x * 8); ++x) {
			pixels[y * WIDTH + x] = gPalette.at(line.at(x%8));
		}
		line_num += 2;
	}
}
int main(int, char **) {
	// initialize game window
	Window game_window{};
	if (game_window.init() < 0) {
		fmt::print("Failed to initialize window!");
		return -1;
	}
	// dummy pixel data
	std::vector<uint32_t>pixels(WIDTH * HEIGHT);
	auto reset_screen = [](std::vector<uint32_t> &p) {
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
	std::shared_ptr<MemoryBus> bus = std::make_shared<MemoryBus>();
	std::shared_ptr<InterruptObserver> int_obs = std::make_shared<InterruptObserver>();
	std::shared_ptr<JoyPad> joypad = std::make_shared<JoyPad>();
	std::shared_ptr<Timer> timer = std::make_shared<Timer>();

	cpu.reset();
	cpu.connect_bus(bus);
	bus->connect_joypad(joypad);
	bus->connect_timer(timer);

	bus->connect_interrupt_observer(int_obs);
	joypad->connect_interrupt_observer(int_obs);
	timer->connect_interrupt_observer(int_obs);

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
				// Reset on new rom loaded
				cpu.reset();
				bus->reset();
				joypad->reset();
				int_obs->reset();
				reset_screen(pixels);

				// Handle drag-and-drop rom file
				// meaning open the rom file and start the system
				char *dropped_file = event.drop.file;
				std::string rom_name{ dropped_file };
				SDL_free(dropped_file);
				fmt::print("Loading rom: {}\n", rom_name);
				auto cart = system_load_rom(rom_name);
				SDL_assert(cart != nullptr);
				bus->load_cart(std::move(cart));
				rom_loaded = true;
			} else if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
				case SDLK_w:
					joypad->handle_press(JoyPadInput::UP);
					break;
				case SDLK_a:
					joypad->handle_press(JoyPadInput::LEFT);
					break;
				case SDLK_s:
					joypad->handle_press(JoyPadInput::DOWN);
					break;
				case SDLK_d:
					joypad->handle_press(JoyPadInput::RIGHT);
					break;
				case SDLK_LCTRL:
					joypad->handle_press(JoyPadInput::START);
					break;
				case SDLK_SPACE:
					joypad->handle_press(JoyPadInput::SELECT);
					break;
				case SDLK_q:
					joypad->handle_press(JoyPadInput::A);
					break;
				case SDLK_e:
					joypad->handle_press(JoyPadInput::B);
					break;
				}
			} else if (event.type == SDL_KEYUP) {
				switch(event.key.keysym.sym) {
				case SDLK_w:
					joypad->handle_release(JoyPadInput::UP);
					break;
				case SDLK_a:
					joypad->handle_release(JoyPadInput::LEFT);
					break;
				case SDLK_s:
					joypad->handle_release(JoyPadInput::DOWN);
					break;
				case SDLK_d:
					joypad->handle_release(JoyPadInput::RIGHT);
					break;
				case SDLK_LCTRL:
					joypad->handle_release(JoyPadInput::START);
					break;
				case SDLK_SPACE:
					joypad->handle_release(JoyPadInput::SELECT);
					break;
				case SDLK_q:
					joypad->handle_release(JoyPadInput::A);
					break;
				case SDLK_e:
					joypad->handle_release(JoyPadInput::B);
					break;
				}
			}
		}
		// Render
		game_window.update();
		// run gameboy only if game is loaded
		if (rom_loaded) {
			int cycles_ran = cpu.step(440);
			timer->step(cycles_ran);
			// TODO ppu step
			// TODO apu step
		}

		// lets fill the pixel data tilemap is 20wx18h
		/*
		int offset = 0;
		for (int y = 0; y < 18; ++y) {
			for (int x = 0; x < 20; ++x) {
				fill_tile(pixels, bus->vram.data() + offset, 0, x, y);
				offset += 16;
			}
		}
		*/
		// Render
		game_window.draw(pixels.data()); // TODO - send the ppu data instead
		fflush(stdout);
	}
	return 0;
}