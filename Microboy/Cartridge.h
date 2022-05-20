#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common.h"

enum class CartridgeType {
	MBC0,
	MBC1,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2,
	MBC2_BATTERY,
	// TODO Add more supported types as needed
};

class Cartridge
{
public:
	virtual ~Cartridge() noexcept = default;

	virtual uint8_t read_byte(uint16_t addr) = 0;
	virtual void write_byte(uint16_t addr, uint8_t value) = 0;
};

std::unique_ptr<Cartridge> system_load_rom(const std::string &filename);

CartridgeType parse_header(const std::vector<uint8_t> &data);
