#pragma once
#include "Cartridge.h"

class Mbc1 : public Cartridge {
public:
	Mbc1(std::vector<uint8_t> data) : rom_bank_sel(1), ram_bank_sel{1}, ram_enabled{false}, rom_data{data}, ram_data{} {}
	virtual ~Mbc1() noexcept override = default;
	virtual uint8_t read_byte(uint16_t addr) override;
	virtual void write_byte(uint16_t addr, uint8_t value) override;

private:
	uint8_t rom_bank_sel;
	uint8_t ram_bank_sel;
	bool ram_enabled;
	std::vector<uint8_t> rom_data;
	std::vector<uint8_t> ram_data;
};

