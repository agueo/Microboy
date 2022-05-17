#include "Mbc0.h"

uint8_t Mbc0::read_byte(uint16_t addr) {
	// for MBC0 we don't need to do any bank switching
	return rom_data[addr];
}

void Mbc0::write_byte(uint16_t addr, uint8_t value) {
	// for MBC0 we don't need to do any bank switching
	rom_data[addr] = value;
}
