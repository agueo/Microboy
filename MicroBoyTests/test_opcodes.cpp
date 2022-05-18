#include "pch.h"
#include <memory>

#include "../Microboy/Cartridge.h"
#include "../Microboy/Cpu.h"
#include "../Microboy/Mbc0.h"
#include "../Microboy/MemoryBus.h"

class TestOpcodes : public ::testing::Test {

protected:
	void SetUp() override {
		bus = std::make_shared<MemoryBus>();
		cpu.connect_bus(bus);
		reset_cpu();
	}

	void reset_cpu() {
		cpu.write_word(BC, 0xDEAD);
		cpu.write_word(DE, 0xBEEF);
		cpu.write_word(HL, 0xCAFE);
		cpu.write_word(AF, 0xDEED);
	}

// We'll need a few things to test the opcodes
	Cpu cpu{};
	std::shared_ptr<MemoryBus> bus{nullptr};
};

TEST_F(TestOpcodes, TestOpcode_ld_r_r) {
	// test rom
	std::vector<uint8_t> rom_data { 
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47, // LD B, r
		0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4F, // LD C, r
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57, // LD D, r
		0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5F, // LD E, r
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67, // LD H, r
		0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6F, // LD L, r
		0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7F, // LD L, r
		0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{B, 0xDE, 4}, {B, 0xAD, 4}, {B, 0xBE, 4}, {B, 0xEF, 4}, {B, 0xCA, 4}, {B, 0xFE, 4}, {B, 0xDE, 4}, 
		{C, 0xDE, 4}, {C, 0xAD, 4}, {C, 0xBE, 4}, {C, 0xEF, 4}, {C, 0xCA, 4}, {C, 0xFE, 4}, {C, 0xDE, 4}, 
		{D, 0xDE, 4}, {D, 0xAD, 4}, {D, 0xBE, 4}, {D, 0xEF, 4}, {D, 0xCA, 4}, {D, 0xFE, 4}, {D, 0xDE, 4}, 
		{E, 0xDE, 4}, {E, 0xAD, 4}, {E, 0xBE, 4}, {E, 0xEF, 4}, {E, 0xCA, 4}, {E, 0xFE, 4}, {E, 0xDE, 4}, 
		{H, 0xDE, 4}, {H, 0xAD, 4}, {H, 0xBE, 4}, {H, 0xEF, 4}, {H, 0xCA, 4}, {H, 0xFE, 4}, {H, 0xDE, 4}, 
		{L, 0xDE, 4}, {L, 0xAD, 4}, {L, 0xBE, 4}, {L, 0xEF, 4}, {L, 0xCA, 4}, {L, 0xFE, 4}, {L, 0xDE, 4}, 
		{A, 0xDE, 4}, {A, 0xAD, 4}, {A, 0xBE, 4}, {A, 0xEF, 4}, {A, 0xCA, 4}, {A, 0xFE, 4}, {A, 0xDE, 4}, 
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		reset_cpu();
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_r_u8) {
	// test rom
	std::vector<uint8_t> rom_data { 
		0x06, 0x01, 0x16, 0x02, 0x26, 0x03, 0x0e, 0x04, 0x1E, 0x05, 0x2E, 0x06, 0x3E, 0x07, 0x00, 0x00// LD r, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{B, 0x01, 8}, {D, 0x02, 8}, {H, 0x03, 8}, {C, 0x04, 8}, {E, 0x05, 8}, {L, 0x06, 8}, {A, 0x07, 8}, 
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_r_u16) {
	cpu.write_word(BC, 0x0000);
	cpu.write_word(DE, 0x0000);
	cpu.write_word(HL, 0x0000);
	cpu.write_word(AF, 0x0000);

	// test rom
	std::vector<uint8_t> rom_data { 
		0x01, 0xAD, 0xDE, 0x11, 0xFE, 0xCA, 0x21, 0xED, 0xFE, 0x31, 0xEF, 0xBE, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName16Bit r;
		uint16_t value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{BC, 0xDEAD, 12}, {DE, 0xCAFE, 12}, {HL, 0xFEED, 12}, {SP, 0xBEEF, 12}
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(exp.r), exp.value_expected);
	}
}