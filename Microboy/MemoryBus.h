#pragma once

#include <array>
#include "Cartridge.h"
#include "InterruptObserver.h"
#include "JoyPad.h"
#include "Timer.h"

// constant ranges
constexpr int ROM_BASE	= 0x0000;
constexpr int ROM_END	= 0x7FFF;
constexpr int VRAM_BASE = 0x8000;  // This entire area holds tile data 0x8000 - 0x97FF
constexpr int VRAM_END	= 0x9FFF;  // 0x9800 to 0x9fff hold 2 32x32 tile maps
constexpr int EXRAM_BASE = 0xA000; // belongs to cartridge ram
constexpr int EXRAM_END = 0xBFFF;
constexpr int WRAM_BASE = 0xC000;
constexpr int WRAM_END	= 0xDFFF;
constexpr int ECHO_BASE = 0xE000;
constexpr int ECHO_END	= 0xFDFF;
constexpr int OAM_BASE	= 0xFE00;
constexpr int OAM_END	= 0xFE9F; // belongs to PPU?
constexpr int PROHIB_BASE = 0xFEA0;
constexpr int PROHIB_END = 0xFEFF;
constexpr int IO_BASE	= 0xFF00; // This section needs to be split among the various objects ie joycon, ppu, apu, etc.
constexpr int IO_END	= 0xFF7F;
constexpr int HRAM_BASE = 0xFF80;
constexpr int HRAM_END	= 0xFFFE;

// IO Space
// Serial
constexpr int SB_ADDR = 0xFF01;
constexpr int SC_ADDR = 0xFF02;
// LCD
constexpr int LCDC_ADDR = 0xFF40;
constexpr int STAT_ADDR = 0xFF41;
constexpr int SCY_ADDR = 0xFF42;
constexpr int SCX_ADDR = 0xFF43;
constexpr int LY_ADDR = 0xFF44;
constexpr int LYC_ADDR = 0xFF45; 
constexpr int DMA_ADDR = 0xFF46;
constexpr int BGP_ADDR = 0xFF47;
constexpr int WY_ADDR = 0xFF4A;
constexpr int WX_ADDR = 0xFF4B;
// Skipping sound registers - TODO

class MemoryBus {
public:
	MemoryBus();
	~MemoryBus() = default;
	void reset();

	void load_cart(std::unique_ptr<Cartridge> c);
	void connect_interrupt_observer(std::shared_ptr<InterruptObserver> observer);
	void connect_joypad(std::shared_ptr<JoyPad> joypad);
	void connect_timer(std::shared_ptr<Timer> timer);

	uint8_t read_byte(uint16_t addr);
	uint16_t read_word(uint16_t addr);
	void write_byte(uint16_t addr, uint8_t value);
	void write_word(uint16_t addr, uint16_t value);

	std::vector<uint8_t> vram; // this will belong to the ppu
private:
	std::vector<uint8_t> wram;
	std::vector<uint8_t> oam; // this will belong to the ppu
	std::vector<uint8_t> IO; // TODO - put this in it's own thing eventually
	std::vector<uint8_t> hram;

	std::unique_ptr<Cartridge> cart;
	std::shared_ptr<JoyPad> m_joypad;
	std::shared_ptr<Timer> m_timer;
	std::shared_ptr<InterruptObserver> m_int_observer;

	// std::shared_ptr<Ppu> m_ppu;
	// enhancement: have a map for objects that want to register their high and low addr areas and a callback
	// or create a radix tree for these callbacks
	// Maybe most helpful for the IO space
};