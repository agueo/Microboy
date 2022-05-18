#include "pch.h"
#include <memory>
#include <vector>

#include "../Microboy/MemoryBus.h"

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

TEST(BusTests, BusReadWriteWord) {
	MemoryBus bus{};
	// write to address
	bus.write_word(HRAM_BASE + 8, 0xDEAD);
	uint16_t value = bus.read_word(HRAM_BASE + 8);
	ASSERT_EQ(value, 0xDEAD);
}
