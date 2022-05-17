#include <algorithm>
#include <fstream>
#include <iostream>

#include <fmt/core.h>

#include "Cartridge.h"
#include "Mbc0.h"
#include "Mbc1.h"

CartridgeType parse_header(const std::vector<uint8_t> &data) {
	// decode the cartridge type
	uint8_t cartridge_type = data.at(0x147);
	// may also need to decode the ram and rom size for bigger cartridges
	return (CartridgeType)cartridge_type;
}

std::unique_ptr<Cartridge> system_load_rom(const std::string &filename) {
	std::ifstream rom(filename, std::ios::in | std::ios::binary);
	if (!rom.is_open()) {
		return nullptr;
	}
	std::vector<uint8_t> rom_data((std::istreambuf_iterator<char>(rom)), std::istreambuf_iterator<char>());
	fmt::print("file size: {:#04x}\n", rom_data.size());
	switch (parse_header(rom_data)) {
	case CartridgeType::MBC0: return std::make_unique<Mbc0>(rom_data);
	case CartridgeType::MBC1: return std::make_unique<Mbc1>(rom_data);
	default: return std::make_unique<Mbc0>(rom_data);
	}
}
