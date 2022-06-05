#pragma once

#include <array>
#include "Cartridge.h"

// constant ranges
constexpr int ROM_BASE	= 0x0000;
constexpr int ROM_END	= 0x7FFF;
constexpr int VRAM_BASE = 0x8000;
constexpr int VRAM_END	= 0x9FFF;
constexpr int EXRAM_BASE = 0xA000; // belongs to cartridge ram
constexpr int EXRAM_END = 0xBFFF;
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

// PPU registers
constexpr int  LCDC = 0xFF40;	// R/W
constexpr int  STAT = 0xFF41;	// R/W
constexpr int  SCY = 0xFF42;	// R/W
constexpr int  SCX = 0xFF43;	// R/W
constexpr int  LY = 0xFF44;		// R 
constexpr int  LYC = 0xFF45;	// R/W
constexpr int  DMA = 0xFF46;	// 
constexpr int  BGP = 0xFF47;	// R/W
constexpr int  OBJP0 = 0xFF48;	// R/W
constexpr int  OBJP1 = 0xFF49;	// R/W
constexpr int  WY = 0xFF4A;		// R/W
constexpr int  WX = 0xFF4B;		// R/W

class MemoryBus
{
public:

	void reset();
	void load_cart(std::unique_ptr<Cartridge> c);
	uint8_t read_byte(uint16_t addr);
	uint16_t read_word(uint16_t addr);
	void write_byte(uint16_t addr, uint8_t value);
	void write_word(uint16_t addr, uint16_t value);


	std::array<uint8_t, 0x2000> vram; // this will belong to the ppu
private:
	std::array<uint8_t, 0x4000> wram;
	std::array<uint8_t, 0xA0> oam; // this will belong to the ppu
	std::array<uint8_t, 0x80> IO; // TODO - put this in it's own thing eventually
	std::array<uint8_t, 0x7f> hram;
	uint8_t ie;
	std::unique_ptr<Cartridge> cart;
	// enhancement have a map for objects that want to register their high and low addr areas and a callback
};

