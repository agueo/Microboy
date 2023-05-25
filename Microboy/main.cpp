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
#include "Ppu.h"
#include "Timer.h"
#include "Window.h"


int main(int, char **) {
	// initialize game window
	Window game_window{};
	if (game_window.init() < 0) {
		fmt::print("Failed to initialize window!");
		return -1;
	}

	// DMG objects
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
    bool draw_frame {true};
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
                timer->reset();

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
			int cycles_ran = cpu.step(456);
			timer->step(cycles_ran);
			draw_frame = ppu->step(cycles_ran);
		} else {
			SDL_Delay(10);
		}

		// Throttle cpu if idle
		if (cpu.is_halted()) {
			SDL_Delay(10);
		}

        if (draw_frame) {
            game_window.draw(ppu->get_frame_buffer());
        }
		fflush(stdout);
	}
	return 0;
}
