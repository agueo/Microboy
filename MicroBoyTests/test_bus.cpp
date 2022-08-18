#include "pch.h"
#include <algorithm>
#include <memory>
#include <vector>

#include "../Microboy/MemoryBus.h"
#include "../Microboy/Cartridge.h"
#include "../Microboy/Mbc1.h"

TEST(BusTests, BusReadWriteByte) {
	MemoryBus bus{};

	// write to address 
	bus.write_byte(WRAM_BASE + 4, 0xEF);
	uint8_t value = bus.read_byte(WRAM_BASE + 4);
	ASSERT_EQ(value, 0xEF);
	
	bus.write_byte(WRAM_BASE + 5, 0xBE);
	value = bus.read_byte(WRAM_BASE + 5);
	ASSERT_EQ(value, 0xBE);
}

TEST(CarTests, CartTest_mbc1) {
	std::vector<uint8_t> rom_data{};
	rom_data.resize(0x10000);
	std::fill(rom_data.begin(), rom_data.begin()+0x4000, 0x11);
	std::fill(rom_data.begin()+0x4000, rom_data.begin()+0x8000, 0x22);
	std::fill(rom_data.begin()+0x8000, rom_data.begin()+0xC000, 0x33);
	std::fill(rom_data.begin()+0xC000, rom_data.end(), 0x44);
	
	auto bus = std::make_unique<MemoryBus>();
	auto cart = std::make_unique<Mbc1>(rom_data);
	bus->load_cart(std::move(cart));

	// Read from bank 0
	ASSERT_EQ(bus->read_byte(0x100), 0x11);

	// Read from bank 1
	ASSERT_EQ(bus->read_byte(0x4100), 0x22);

	// write to rom bank register 
	bus->write_byte(0x2000, 0x02);
	// Read from bank 2
	ASSERT_EQ(bus->read_byte(0x4100), 0x33);

	// write to rom bank register
	bus->write_byte(0x2000, 0x03);
	// Read from bank 3
	ASSERT_EQ(bus->read_byte(0x4100), 0x44);

	// write to rom bank register
	bus->write_byte(0x2000, 0x00);
	ASSERT_EQ(bus->read_byte(0x4100), 0x22);
}

// TODO add negative tests for reading and writing to invalid areas
// TODO Do the same for the cart ie reading to rom and

// Lets Load an MBC1 rom and try to read from the different banks 64KB should have 4 banks
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
