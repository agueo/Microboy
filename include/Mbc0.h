#ifndef MBC0_H
#define MBC0_H
#include <vector>

#include "Cartridge.h"

class Mbc0 : public Cartridge
{
public:
	Mbc0(std::vector<uint8_t> rom) : rom_data{ rom } {}
	virtual	~Mbc0() noexcept override = default;

	virtual uint8_t read_byte(uint16_t addr) override;
	virtual void write_byte(uint16_t addr, uint8_t value) override;

private:
	std::vector<uint8_t> rom_data;
};

#endif