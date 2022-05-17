#include "Mbc1.h"

uint8_t Mbc1::read_byte(uint16_t addr) {
	// for MBC1 we need to do any bank switching
	return 0;
}

void Mbc1::write_byte(uint16_t addr, uint8_t value) {
	// for MBC1 we need to do bank switching
	(void)addr;
	(void)value;
	return;
}
