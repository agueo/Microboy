#pragma once
#include "Cartridge.h"
class Mbc1 : public Cartridge {
public:
	Mbc1(std::vector<uint8_t> data) : rom_data{ data } {}
	virtual ~Mbc1() noexcept override = default;
	virtual uint8_t read_byte(uint16_t addr) override;
	virtual void write_byte(uint16_t addr, uint8_t value) override;

private:
	std::vector<uint8_t> rom_data;
};

