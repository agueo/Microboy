#include "MemoryBus.h"
#include <fmt/core.h>

void MemoryBus::reset() {
	cart = nullptr;
	// All hardware registers at 0x100
	IO[0xFF00 - IO_BASE] = 0xCF; // P1
    IO[0xFF01 - IO_BASE] = 0x00; // SB
    IO[0xFF02 - IO_BASE] = 0x7E; // SC
    IO[0xFF04 - IO_BASE] = 0x18; // DIV
    IO[0xFF05 - IO_BASE] = 0x00; // TIMA
    IO[0xFF06 - IO_BASE] = 0x00; // TMA
    IO[0xFF07 - IO_BASE] = 0xF8; // TAC
    IO[0xFF0F - IO_BASE] = 0xE1; // IF
	// Skipping sound registers
    IO[0xFF40 - IO_BASE] = 0x91; // LCDC
    IO[0xFF41 - IO_BASE] = 0x85; // STAT
    IO[0xFF42 - IO_BASE] = 0x00; // SCY
    IO[0xFF43 - IO_BASE] = 0x00; // SCX
    IO[0xFF44 - IO_BASE] = 0x90; // LY
    IO[0xFF45 - IO_BASE] = 0x00; // LYC
    IO[0xFF46 - IO_BASE] = 0xFF; // DMA
    IO[0xFF47 - IO_BASE] = 0xFC; // BGP
    IO[0xFF4A - IO_BASE] = 0x00; // WY
    IO[0xFF4B - IO_BASE] = 0x00; // WX
    IO[0xFF4D - IO_BASE] = 0xFF; // KEY1
    ie = 0x00;
}

void MemoryBus::load_cart(std::unique_ptr<Cartridge> c) {
	cart = std::move(c);
}

uint8_t MemoryBus::read_byte(uint16_t addr) {
	if (addr >= ROM_BASE && addr <= ROM_END) {
		return cart->read_byte(addr);
	}
	else if (addr >= EXRAM_BASE && addr <= EXRAM_END) {
		return cart->read_byte(addr);
	}
	else if (addr >= VRAM_BASE && addr <= VRAM_END) {
		return vram[addr - VRAM_BASE];
	}
	else if (addr >= WRAM_BASE && addr <= WRAM_END) {
		return wram[addr - WRAM_BASE];
	}
	else if (addr >= HRAM_BASE && addr <= HRAM_END) {
		return hram[addr - HRAM_BASE];
	}
	else if (addr >= IO_BASE && addr <= IO_END) {
		return IO[addr - IO_BASE];
	}
	else if (addr >= OAM_BASE && addr <= OAM_END) {
		return oam[addr - OAM_BASE];
	}
	else if (addr == IE_ADDR) {
		return ie;
	}
	return 0;
}

void MemoryBus::write_byte(uint16_t addr, uint8_t value) {
	if (addr >= ROM_BASE && addr <= ROM_END) {
		cart->write_byte(addr, value);
	}
	else if (addr >= EXRAM_BASE && addr <= EXRAM_END) {
		cart->write_byte(addr, value);
	}
	else if (addr >= VRAM_BASE && addr <= VRAM_END) {
		vram[addr - VRAM_BASE] = value;
	}
	else if (addr >= WRAM_BASE && addr <= WRAM_END) {
		wram[addr - WRAM_BASE] = value;
	}
	else if (addr >= HRAM_BASE && addr <= HRAM_END) {
		hram[addr - HRAM_BASE] = value;
	}
	else if (addr >= IO_BASE && addr <= IO_END) {
		IO[addr - IO_BASE] = value;
		if (addr == 0xFF01) {
			fmt::print("{:c}", value);
		}
	}
	else if (addr >= OAM_BASE && addr <= OAM_END) {
		oam[addr - OAM_BASE] = value;
	}
	else if (addr == IE_ADDR) {
		ie = value;
	}
	return;
}

uint16_t MemoryBus::read_word(uint16_t addr) {
	uint16_t lo, hi;
	lo = read_byte(addr);
	hi = read_byte(addr + 1);
	return ((hi << 8) & 0xFF00) | (lo & 0xFF);
}

void MemoryBus::write_word(uint16_t addr, uint16_t value) {
	// lo write
	write_byte(addr, value & 0xFF);
	// hi write
	write_byte(addr + 1, (value >> 8) & 0xFF);
}
