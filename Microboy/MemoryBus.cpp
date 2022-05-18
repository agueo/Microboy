#include "MemoryBus.h"

void MemoryBus::reset() {
	cart = nullptr;
}

void MemoryBus::load_cart(std::unique_ptr<Cartridge> c) {
	cart = std::move(c);
}

uint8_t MemoryBus::read_byte(uint16_t addr) {

	if (addr >= ROM_BASE && addr <= ROM_END) {
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
	return 0;
}

void MemoryBus::write_byte(uint16_t addr, uint8_t value) {
	
	if (addr >= ROM_BASE && addr <= ROM_END) {
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
	}
	else if (addr >= OAM_BASE && addr <= OAM_END) {
		oam[addr - OAM_BASE] = value;
	}
	return;
}

uint16_t MemoryBus::read_word(uint16_t addr) {
	uint16_t lo, hi;
	lo = read_byte(addr);
	hi = read_byte(addr + 1);
	return ((hi << 8) & 0xFF00) | lo & 0xFF;
}

void MemoryBus::write_word(uint16_t addr, uint16_t value) {
	// lo write
	write_byte(addr, value & 0xFF);
	write_byte(addr + 1, (value >> 8) & 0xFF);
}
