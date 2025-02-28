#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <memory>
#include <string>
#include <vector>

#include "common.h"

enum class CartridgeType : uint8_t{
	MBC0,
	MBC1,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2,
	MBC2_BATTERY,
	// TODO Add more supported types as needed
};

enum class CartridgeRomSize : uint8_t {
	_32KB,		// 2 banks (no banking)
	_64KB,		// 4 banks
	_128KB,		// 8 banks
	_256KB,		// 16 banks
	_512KB,		// 32 banks
	_1MB,		// 64 banks
	_2MB,		// 128 banks
	_4MB,		// 256 banks
	// TODO Add more supported types as needed
};

enum class CartridgeRamSize : uint8_t {
	_0KB,		// no ram
	unused,
	_8KB,		// 1 bank
	_32KB,		// 4 banks of 8KB
	_128KB,		// 16 banks of 8KB 
	_64KB		// 8 banks of 8KB
};

struct CartridgeSettings{
	std::string title;
	CartridgeType type;
	CartridgeRomSize rom_size;
	CartridgeRamSize ram_size;
};

static const char cartridge_types[24][32] = {
	{"ROM ONLY"},
	{"MBC1"},
	{"MBC1+RAM"},
	{"MBC1+RAM+BATTERY"},
	{"MBC2"},
	{"MBC2+BATTERY"},
	{"ROM+RAM 1"},
	{"ROM+RAM+BATTERY 1"},
	{"MMM01"},
	{"MMM01+RAM"},
	{"MMM01+RAM+BATTERY"},
	{"MBC3+TIMER+BATTERY"},
	{"MBC3+TIMER+RAM+BATTERY_d"},
	{"MBC3"},
	{"MBC3+RAM 2"},
	{"MBC3+RAM+BATTERY 2"},
	{"MBC5"},
	{"MBC5+RAM"},
	{"MBC5+RAM+BATTERY"},
	{"MBC5+RUMBLE"},
	{"MBC5+RUMBLE+RAM"},
	{"MBC5+RUMBLE+RAM+BATTERY"},
	{"MBC6"},
	{"MBC7+SENSOR+RUMBLE+RAM+BATTERY"},
};

static const char rom_sizes_str[9][10] = {
	{"32 KB"},
	{"64 KB"},
	{"128 KB"},
	{"256 KB"},
	{"512 KB"},
	{"1 MB"},
	{"2 MB"},
	{"4 MB"},
	{"8 MB"},
};

static const char ram_sizes_str[6][7] = {
	{"0 KB"},
	{"-"},
	{"8 KB"},
	{"32 KB"},
	{"128 KB"},
	{"64 KB"},
};

class Cartridge
{
public:
	virtual ~Cartridge() noexcept = default;

	virtual uint8_t read_byte(uint16_t addr) = 0;
	virtual void write_byte(uint16_t addr, uint8_t value) = 0;
};

std::unique_ptr<Cartridge> system_load_rom(const std::string &filename);

CartridgeSettings parse_header(const std::vector<uint8_t> &data);

#endif