#include "Mbc1.h"

const uint16_t BANK1_BASE = 0x0000;
const uint16_t BANK1_END  = 0x3FFF;
const uint16_t BANK2_BASE = 0x4000;
const uint16_t BANK2_END  = 0x7FFF;

const uint16_t RAM_BASE = 0xA000;
const uint16_t RAM_END  = 0xBFFF;

const uint16_t RAM_EN_BASE = 0x0000;
const uint16_t RAM_EN_END 	= 0x1FFF;
const uint16_t ROM_REG_BASE = 0x2000;
const uint16_t ROM_REG_END 	= 0x3FFF;
const uint16_t RAM_REG_BASE = 0x0000;
const uint16_t RAM_REG_END 	= 0x1FFF;


uint8_t Mbc1::read_byte(uint16_t addr) {
	// rom 
	// Bank 0 0x0000 - 0x3FFF
	if (addr >= BANK1_BASE && addr <= BANK1_END) {
		return rom_data[addr];
	}
	// Bank 0x1-0x7F - 0x4000 - 0x7FFF
	else if (addr >= BANK2_BASE && addr <= BANK2_END) {
		return rom_data[addr * rom_bank_sel];
	}

	// RAM access
	if (addr >= RAM_BASE && addr <= RAM_END && ram_data.size() > 0) {
		if (!ram_enabled) { return 0xFF; }
		return ram_data[addr + ram_bank_sel];
	}

	return 0xFF;
}

void Mbc1::write_byte(uint16_t addr, uint8_t value) {
	// when writing to rom we access MBC registers
	if (addr >= RAM_EN_BASE && addr <= RAM_EN_END) {
		ram_enabled = ((value & 0xF) == 0xA);
		return;
	}
	// select rom bank
	if (addr >= ROM_REG_BASE && addr <= ROM_REG_END) {
		rom_bank_sel = (value & 0x1F);
		if (rom_bank_sel == 0) rom_bank_sel = 1;
		return;
	}
	// select ram bank
	if (addr >= RAM_REG_BASE && addr <= RAM_REG_BASE) {
		ram_bank_sel = (value & 0x3);
	}

	// RAM access
	if (addr >= RAM_BASE && addr <= RAM_END && ram_data.size() > 0) {
		if (!ram_enabled) { return; }
		ram_data[addr + (RAM_BASE * ram_bank_sel)] = value;
	}

	return;
}
