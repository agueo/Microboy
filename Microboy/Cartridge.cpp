#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include <fmt/core.h>

#include "Cartridge.h"
#include "Mbc0.h"
#include "Mbc1.h"

CartridgeSettings parse_header(const std::vector<uint8_t> &data) {
	CartridgeSettings cart_settings{};
	// decode the cartridge type
	cart_settings.type = (CartridgeType)data.at(0x147);
	// decode the cartridge size
	cart_settings.rom_size = (CartridgeRomSize)data.at(0x148);
	// decode the cartridge ram size
	cart_settings.ram_size = (CartridgeRamSize)data.at(0x149);
	// decode the cartridge title
	for (int i = 0x134; i <= 0x143; ++i) {
		if (data.at(i) == 0x00) break;
		cart_settings.title += data.at(i);
	}
	return cart_settings;
}

std::unique_ptr<Cartridge> system_load_rom(const std::string &filename) {
	std::ifstream rom(filename, std::ios::in | std::ios::binary);
	if (!rom.is_open()) {
		return nullptr;
	}
	std::vector<uint8_t> rom_data((std::istreambuf_iterator<char>(rom)), std::istreambuf_iterator<char>());
	fmt::print("file size: {:#04x}\n", rom_data.size());
	auto cart_settings = parse_header(rom_data);
	fmt::print("Cartridge Title: {}\n", cart_settings.title);
	fmt::print("Cartridge Type: {}\n", cartridge_types[static_cast<uint8_t>(cart_settings.type)]);
	fmt::print("Cartridge Rom Size: {}\n", rom_sizes_str[static_cast<uint8_t>(cart_settings.rom_size)]);
	fmt::print("Cartridge Ram Size: {}\n", ram_sizes_str[static_cast<uint8_t>(cart_settings.ram_size)]);

	// TODO - create a cart builder
	switch(cart_settings.type) {
	case CartridgeType::MBC0: return std::make_unique<Mbc0>(rom_data);
	case CartridgeType::MBC1: return std::make_unique<Mbc1>(rom_data);
	default: return std::make_unique<Mbc0>(rom_data);
	}
}
