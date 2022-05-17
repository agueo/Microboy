#include "pch.h"
#include <memory>
#include <vector>

#include "../Microboy/MemoryBus.h"
#include "../Microboy/Cartridge.h"
#include "../Microboy/Mbc0.h"

TEST(BusTests, BusRead) {
	// setup
	std::vector<uint8_t> data {0xC3, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x0c, 0x02, 0xff, 0xff, 0xff, 0xff};
	auto cart = system_load_rom("D:/Dev/Rust/file_transfer/Tetris.gb");
	EXPECT_TRUE(cart != nullptr);

	MemoryBus bus{};
	bus.load_cart(std::move(cart));

	// do some reads and expect some output 
	for (uint16_t i = 0; i < 16; ++i) {
		auto read_data = bus.read_byte(i);
		ASSERT_EQ(read_data, data.at(i));
	}
}
