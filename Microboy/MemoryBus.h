#pragma once

#include <array>
#include "Cartridge.h"

// constant ranges
constexpr int ROM_BASE	= 0x0000;
constexpr int ROM_END	= 0x7FFF;
constexpr int VRAM_BASE = 0x8000;
constexpr int VRAM_END	= 0x9FFF;
// missing 0xA000-BFFF belongs to external rom
constexpr int WRAM_BASE = 0xC000;
constexpr int WRAM_END	= 0xDFFF;
constexpr int OAM_BASE	= 0xFE00;
constexpr int OAM_END	= 0xFE9F; // belongs to PPU?
constexpr int IO_BASE	= 0xFF00;
constexpr int IO_END	= 0xFF7F;
constexpr int HRAM_BASE = 0xFF80;
constexpr int HRAM_END	= 0xFFFE;

// interrupt flags
constexpr int IF_ADDR	= 0xFF0F;
constexpr int IE_ADDR	= 0xFFFF;

class MemoryBus
{
public:

	void reset();
	void load_cart(std::unique_ptr<Cartridge> c);
	uint8_t read_byte(uint16_t addr);
	uint16_t read_word(uint16_t addr);
	void write_byte(uint16_t addr, uint8_t value);
	void write_word(uint16_t addr, uint16_t value);


private:
	std::array<uint8_t, 0x2000> vram; // this will belong to the ppu
	std::array<uint8_t, 0x4000> wram;
	std::array<uint8_t, 0xA0> oam; // this will belong to the ppu
	std::array<uint8_t, 0x80> IO; // TODO - put this in it's own thing eventually
	std::array<uint8_t, 0x7f> hram;
	std::unique_ptr<Cartridge> cart;
	// enhancement have a map for objects that want to register their high and low addr areas and a callback
};

