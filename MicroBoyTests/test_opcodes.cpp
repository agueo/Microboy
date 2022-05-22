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

/*--------------------------------------------------*/
/* Test Load Instructions							*/
/*--------------------------------------------------*/
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

TEST_F(TestOpcodes, TestOpcode_ld_m_r16_a) {
	cpu.reset();
	// Test LD (BC,DE), A
	// also test LD (u16), SP
	cpu.write_word(BC, WRAM_BASE);
	cpu.write_word(DE, WRAM_BASE+2);
	cpu.write_word(AF, 0x9000);
	cpu.write_word(PC, 0x0000);

	// test rom
	std::vector<uint8_t> rom_data {
		0x02, 0x12, 0x08, 0x04, 0xc0, 0x00, 0x00
	};

	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));

	// expected state structure
	struct expected {
		uint16_t addr;
		uint8_t value_expected;
		int cycles_taken;
	};

	// tables driven testing
	std::vector<expected> expected_state {
		{WRAM_BASE, 0x90, 8}, {WRAM_BASE+2, 0x90, 8}, {WRAM_BASE + 4, 0xFE, 20}
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(bus->read_byte(exp.addr), exp.value_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_a_m_r16)
{
	cpu.reset();
	// Test LD A, (BC,DE)
	cpu.write_word(BC, WRAM_BASE);
	cpu.write_word(DE, WRAM_BASE+2);
	bus->write_word(WRAM_BASE, 0x12);
	bus->write_word(WRAM_BASE+2, 0x34);
	cpu.write_word(AF, 0x9000);
	cpu.write_word(PC, 0x0000);

	// test rom
	std::vector<uint8_t> rom_data {
		0x0A, 0x1A, 0x00, 0x00
	};

	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));

	// expected state structure
	struct expected {
		uint16_t addr;
		uint8_t value_expected;
		int cycles_taken;
	};

	// tables driven testing
	std::vector<expected> expected_state {
		{WRAM_BASE, 0x12, 8}, {WRAM_BASE+2, 0x34, 8},
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(bus->read_byte(exp.addr), exp.value_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_r_m_hl) {
	// Test LD R, (HL)
	// setup some data where HL is pointing. Lets use WRAM addresses
	bus->write_word(WRAM_BASE + 0x04, 0xCEED);
	cpu.write_word(HL, WRAM_BASE + 0x04);

	// test rom
	std::vector<uint8_t> rom_data {
		0x46, 0x56, 0x66, 0x4E, 0x5E, 0x6E, 0x7E, 0x00, 0x00
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
		{B, 0xED, 8}, {D, 0xED, 8}, {H, 0xED, 8}, {C, 0xED, 8}, {E, 0xED, 8}, {L, 0xED, 8}, {A, 0xED, 8},
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		// Reset HL to 0xC004 -> 0xCEED, NOTE: GB is litte endian so we are reading the LSB byte (0xED)
		cpu.write_word(HL, WRAM_BASE + 0x04);
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_m_hl_r) {
	// Test LD (HL), R
	cpu.write_word(BC, 0x1234);
	cpu.write_word(DE, 0x5678);
	cpu.write_word(HL, WRAM_BASE);
	cpu.write_word(AF, 0x9000);

	// test rom
	std::vector<uint8_t> rom_data {
		// LD (HL), r
		0x70, 0x71, 0x72, 0x73, 0x77, 0x74, 0x75,
		// LD (HL+/-), A
		0x22, 0x32,
		// LD (HL), u8
		0x36, 0x55,
		0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		uint16_t addr;
		uint8_t value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{WRAM_BASE, 0x12, 8}, {WRAM_BASE, 0x34, 8}, {WRAM_BASE, 0x56, 8}, {WRAM_BASE, 0x78, 8}, {WRAM_BASE, 0x90, 8}, {WRAM_BASE, 0xC0, 8}, {WRAM_BASE, 0x00, 8},
		{WRAM_BASE, 0x90, 8}, {WRAM_BASE + 1, 0x90, 8}, {WRAM_BASE, 0x55, 12}
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(bus->read_byte(exp.addr), exp.value_expected);
		if (exp.value_expected == 0xC0 || exp.value_expected == 0x00) {
			// reset the value to 0xc000
			cpu.write_word(HL, WRAM_BASE);
		}
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_IO_a) {
	// Test istructions
	// LD A, (0xFF00 + u8)
	// LD (0xFF00 + u8), A
	// LD A, (0xFF00 + C)
	// LD (0xFF00 + C), A
	cpu.write_byte(C, 0x04);
	bus->write_byte(IO_BASE, 0xAB);
	cpu.write_word(PC, 0);

	// test rom
	std::vector<uint8_t> rom_data {
		0xF0, 0x00, 0xE0, 0x00, 0xF2, 0xE2, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));

	// expected state structure
	struct expected {
		uint16_t addr;
		uint8_t value_expected;
		uint8_t A_value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{IO_BASE, 0xAB, 0xAB, 12}, {IO_BASE, 0xAB, 0xAB, 12},
		{IO_BASE+4, 0x00, 0x00, 8}, {IO_BASE+4, 0x00, 0x00, 8}
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(bus->read_byte(exp.addr), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(A), exp.A_value_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_ld_hl_sp_i8) {
	// LD HL, SP + i8
	cpu.write_word(PC, 0);
	cpu.write_word(SP, 0xC000);
	bus->write_word(0xC000+18, 0xDEAD);
	bus->write_word(0xC100, 0xBEEF);
	// test rom
	std::vector<uint8_t> rom_data {
		0xf8, 0x12, // LD HL, SP + i8	// test h carry
		0x31, 0xFF, 0xC0, // LD SP, u8	// setup sp to test carry overflow
		0xf8, 0x01, // LD HL, SP + i8	// test carry
		0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));

	// expected state structure
	struct expected {
		uint16_t HL_value_expected;
		uint8_t flag_value_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{0xDEAD, 0b0000'0000, 12},
		{0xDEAD, 0b0000'0000, 12},
		{0xBEEF, 0b0011'0000, 12}
	};

	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(HL), exp.HL_value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flag_value_expected);
	}
}

/*--------------------------------------------------*/
/* Test Stack Instructions							*/
/*--------------------------------------------------*/
TEST_F(TestOpcodes, TestOpcodes_push_pop) {
	// test rom
	std::vector<uint8_t> rom_data {
		// push values
		0xC5, 0xD5, 0xE5, 0xF5,
		// pop values
		0xC1, 0xD1, 0xE1, 0xF1, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));

	cpu.reset();
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t SP_addr;
		uint16_t value_expected;
		int cycles_taken;
		RegisterName16Bit reg;
	};

	std::vector<expected> expected_state_push{
		{0xFFFC, 0x0100, 16}, {0xFFFA, 0x13D8, 16}, {0xFFF8, 0x014D, 16}, {0xFFF6, 0x100, 16}
	};

	std::vector<expected> expected_state_pop {
		{0xFFF8, 0x0100, 12, BC}, {0xFFFA, 0x014D, 12, DE}, {0xFFFC, 0x13D8, 12, HL}, {0xFFFE, 0x100, 12, AF}
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state_push) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(SP), exp.SP_addr);
		ASSERT_EQ(bus->read_word(exp.SP_addr), exp.value_expected);
	}

	// check pop
	for (auto &exp : expected_state_pop) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(SP), exp.SP_addr);
		ASSERT_EQ(cpu.read_word(exp.reg), exp.value_expected);
	}
}

/*--------------------------------------------------*/
/* Test Control Flow Instructions					*/
/*--------------------------------------------------*/
TEST_F(TestOpcodes, TestOpcode_call_ret) {
	// test rom
	std::vector<uint8_t> rom_data {
		// call then ret but put the ret at the place the call was expected in ram
		0xCD, 0x00, 0xC0, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	bus->write_byte(WRAM_BASE, 0xC9);

	cpu.reset();
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t SP_addr;
		uint16_t PC_addr;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{HRAM_END-2, WRAM_BASE, 24}, {HRAM_END, 0x03, 16}
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(SP), exp.SP_addr);
		ASSERT_EQ(cpu.read_word(PC), exp.PC_addr);
	}
}

TEST_F(TestOpcodes, TestOpcode_call_ret_n_cond) {
	// test rom
	std::vector<uint8_t> rom_data {
		// testing call NZ, ret NZ, call NC, RET NC
		0xC4, 0x00, 0xC0, 0xD4, 0x01, 0xC0, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	bus->write_byte(WRAM_BASE, 0xC0);
	bus->write_byte(WRAM_BASE+1, 0xD0);
	cpu.write_byte(F, 0x00); // f = ZNHC

	cpu.reset();
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t SP_addr;
		uint16_t PC_addr;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{HRAM_END-2, WRAM_BASE, 24}, {HRAM_END, 0x03, 20},
		{HRAM_END-2, WRAM_BASE+1, 24}, {HRAM_END, 0x06, 20}
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(SP), exp.SP_addr);
		ASSERT_EQ(cpu.read_word(PC), exp.PC_addr);
	}
}

TEST_F(TestOpcodes, TestOpcode_call_ret_cond) {
	// test rom
	cpu.reset();
	std::vector<uint8_t> rom_data {
		// testing call NZ, ret NZ, call NC, RET NC
		0xCC, 0x00, 0xC0, 0xDC, 0x01, 0xC0, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	bus->write_byte(WRAM_BASE, 0xC8);
	bus->write_byte(WRAM_BASE+1, 0xD8);
	// set bits Z and C
	cpu.write_byte(F, 0b10010000); // f = CHNZ
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t SP_addr;
		uint16_t PC_addr;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{HRAM_END-2, WRAM_BASE, 24}, {HRAM_END, 0x03, 20},
		{HRAM_END-2, WRAM_BASE+1, 24}, {HRAM_END, 0x06, 20}
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(SP), exp.SP_addr);
		ASSERT_EQ(cpu.read_word(PC), exp.PC_addr);
	}
}

TEST_F(TestOpcodes, TestOpcode_jp_cond) {
	// test rom
	cpu.reset();
	std::vector<uint8_t> rom_data {
		// testing jp, jp Z, jp C with positive conditionals
		0xC3, 0x03, 0x00, 0xCA, 0x06, 0x00, 0xDA, 0x09, 0x00,
		// jp Z i8
		0x28, 0x02, 0x00, 0x00, 0x38, 0x01, 0x20 , 0x18, (uint8_t)-3, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// set bits Z and C
	cpu.write_byte(F, 0b10010000);
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t PC_addr;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{0x03, 16}, {0x06, 16}, {0x09, 16},
		{0x0D, 12}, {0x10, 12}, {0x0F, 12},
		// case where we jp NZ should not take branch checking correct offset and cycles
		{0x11, 8}
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(PC), exp.PC_addr);
	}
}


TEST_F(TestOpcodes, TestOpcode_jp_n_cond) {
	// test rom
	cpu.reset();
	std::vector<uint8_t> rom_data {
		// testing jp, jp Z, jp C with positive conditionals
		0xC2, 0x03, 0x00, 0xD2, 0x06, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// don't set bits Z and C
	cpu.write_word(PC, 0);

	struct expected {
		uint16_t PC_addr;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{0x3, 16}, {0x06, 16},
	};

	int cycles = 0;
	// Check push
	for (auto &exp : expected_state) {
		cycles = cpu.step(1);
		ASSERT_EQ(cycles, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(PC), exp.PC_addr);
	}
}

/*--------------------------------------------------*/
/* Test Arithmetic Instructions						*/
/*--------------------------------------------------*/
TEST_F(TestOpcodes, TestOpcode_inc_dec_r) {
	cpu.write_word(BC, 0x00FF);
	cpu.write_word(DE, 0xFFFF);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0xDE00);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data{
	0x04, 0x14, 0x24, 0x0C, 0x1C, 0x2C, 0x3C,
	0x05, 0x15, 0x25, 0x0D, 0x1D, 0x2D, 0x3D,
	0x34, 0x35,
	0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flag_expected;
		int cycles_taken;
	};

	std::vector<expected> expected_state {
		// INC r 
		{B, 0x01, 0b0000'0000, 4}, {D, 0x00, 0b1010'0000, 4}, {H, 0xC1, 0b0000'0000, 4}, {C, 0x00, 0b1010'0000, 4},
		{E, 0x00, 0b1010'0000, 4}, {L, 0x01, 0b0000'0000, 4}, {A, 0xDF, 0b0000'0000, 4},
		// DEC r
		{B, 0x00, 0b1100'0000, 4}, {D, 0xFF, 0b0110'0000, 4}, {H, 0xC0, 0b0100'0000, 4}, {C, 0xFF, 0b0110'0000, 4},
		{E, 0xFF, 0b0110'0000, 4}, {L, 0x00, 0b1100'0000, 4}, {A, 0xDE, 0b0100'0000, 4},
	};

	for (const auto& exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flag_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_inc_dec_m_hl) {
	cpu.write_word(HL, WRAM_BASE);
	cpu.write_word(PC, 0);
	bus->write_byte(WRAM_BASE, 0xFF);
	std::vector<uint8_t> rom_data{
	0x34, 0x35,
	0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	struct expected {
		uint8_t value_expected;
		uint8_t flag_expected;
		int cycles_taken;
	};

	std::vector<expected> expected_state {
		// INC the DEC
		{0x00, 0b1010'0000, 12}, {0xFF, 0b0110'0000, 12}
	};

	for (const auto& exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(bus->read_word(WRAM_BASE), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flag_expected);
	}
}
TEST_F(TestOpcodes, TestOpcode_inc_dec_r16) {
	cpu.write_word(BC, 0x00FF);
	cpu.write_word(DE, 0xFFFF);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(SP, 0xFFFD);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data{
		0x03, 0x13, 0x23, 0x33, 
		0x0b, 0x1b, 0x2b, 0x3b, 
		0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	struct expected {
		RegisterName16Bit r;
		uint16_t value_expected;
		int cycles_taken;
	};

	std::vector<expected> expected_state{
		{BC, 0x0100, 8}, {DE, 0x0000, 8}, {HL, 0xC001, 8}, {SP, 0xFFFE, 8},
		{BC, 0x00FF, 8}, {DE, 0xFFFF, 8}, {HL, 0xC000, 8}, {SP, 0xFFFD, 8},
	};

	for (const auto& exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_word(exp.r), exp.value_expected);
	}
}
TEST_F(TestOpcodes, TestOpcode_add_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x0000);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data {
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x87, // ADD A, r
		0x86, 0xC6, 0x00, 0x00, 0x00, 0x00		  // ADD A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		//B							C						   D						  E
		{A, 0x11, 0b0000'0000, 4}, {A, 0x22, 0b0000'0000, 4}, {A, 0x33, 0b0000'0000, 4}, {A, 0x44, 0b0000'0000, 4},
		//H							L						   A						  E
		{A, 0x04, 0b0001'0000, 4}, {A, 0x04, 0b0000'0000, 4}, {A, 0x08, 0b0000'0000, 4},
		// (HL)						u8
		{A, 0x10, 0b0010'0000, 8}, {A, 0x10, 0b0000'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_adc_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x0000);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data {
		0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8F, // ADC A, r
		0x8E, 0xCE, 0x00, 0x00, 0x00, 0x00		  // ADC A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		//B							C						   D						  E
		{A, 0x11, 0b0000'0000, 4}, {A, 0x22, 0b0000'0000, 4}, {A, 0x33, 0b0000'0000, 4}, {A, 0x44, 0b0000'0000, 4},
		//H							L						   A						  E
		{A, 0x04, 0b0001'0000, 4}, {A, 0x05, 0b0000'0000, 4}, {A, 0x0A, 0b0000'0000, 4},
		// (HL)						u8
		{A, 0x12, 0b0010'0000, 8}, {A, 0x12, 0b0000'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_sub_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x0000);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data {
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x97, // SUB A, r
		0x96, 0xD6, 0x00, 0x00, 0x00, 0x00		  // SUB A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		//B							C						   D						  E
		{A, 0xEF, 0b0111'0000, 4}, {A, 0xDE, 0b0100'0000, 4}, {A, 0xCD, 0b0100'0000, 4}, {A, 0xBC, 0b0100'0000, 4},
		//H							L						   A						  E
		{A, 0xFC, 0b0101'0000, 4}, {A, 0xFC, 0b0100'0000, 4}, {A, 0x00, 0b1100'0000, 4},
		// (HL)						u8
		{A, 0xF8, 0b0111'0000, 8}, {A, 0xF8, 0b0100'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_sdc_a) {
	// test romdi{
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x0000);
	cpu.write_word(PC, 0);
	bus->write_byte(0xC000, 8);
	std::vector<uint8_t> rom_data {
		0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9F, // SDC A, r
		0x9E, 0xDE, 0x00, 0x00, 0x00, 0x00		  // SDC A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		//B							C						   D						  E
		{A, 0xEF, 0b0111'0000, 4}, {A, 0xDD, 0b0100'0000, 4}, {A, 0xCC, 0b0100'0000, 4}, {A, 0xBB, 0b0100'0000, 4},
		//H							L						   A						  E
		{A, 0xFB, 0b0101'0000, 4}, {A, 0xFA, 0b0100'0000, 4}, {A, 0x00, 0b1100'0000, 4},
		// (HL)						u8
		{A, 0xF8, 0b0111'0000, 8}, {A, 0xF7, 0b0100'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}


TEST_F(TestOpcodes, TestOpcode_and_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x1100);
	cpu.write_word(PC, 0);
	std::vector<uint8_t> rom_data {
		0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA7,
		0xA6, 0xE6, 0x00, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{A, 0x11, 0b0010'0000, 4}, {A, 0x11, 0b0010'0000, 4}, {A, 0x11, 0b0010'0000, 4}, {A, 0x11, 0b0010'0000, 4},
		{A, 0x0, 0b1010'0000 , 4}, {A, 0, 0b1010'0000, 4}, {A, 0x00, 0b1010'0000, 4},
		{A, 0x0, 0b1010'0000, 8}, {A, 0x0, 0b1010'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_xor_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x1100);
	cpu.write_word(PC, 0);
	std::vector<uint8_t> rom_data {
		0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAF, // LD A, r
		0xAE, 0xEE, 0x00, 0x00, 0x00, 0x00
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{A, 0x00, 0b1000'0000, 4}, {A, 0x11, 0x0, 4}, {A, 0x00, 0b1000'0000, 4}, {A, 0x11, 0, 4}, {A, 0xd1, 0, 4}, {A, 0xd1, 0,4}, {A, 0x00, 0b1000'0000, 4},
		{A, 0x0, 0b1000'0000, 8}, {A, 0x0, 0b1000'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_or_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x1100);
	cpu.write_word(PC, 0);
	std::vector<uint8_t> rom_data {
		0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB7, // OR A, r
		0xB6, 0xF6, 0x00, 0x00, 0x00, 0x00,		  // OR A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		{A, 0x11, 0, 4}, {A, 0x11, 0, 4}, {A, 0x11, 0, 4}, {A, 0x11, 0, 4}, {A, 0xd1, 0, 4}, {A, 0xd1, 0, 4}, {A, 0xd1, 0, 4},
		{A, 0xd1, 0, 8}, {A, 0xd1, 0, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

TEST_F(TestOpcodes, TestOpcode_cp_a) {
	// test rom
	cpu.write_word(BC, 0x1111);
	cpu.write_word(DE, 0x1111);
	cpu.write_word(HL, 0xC000);
	cpu.write_word(AF, 0x1100);
	cpu.write_word(PC, 0);
	std::vector<uint8_t> rom_data {
		0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBF, // CP A, r
		0xBE, 0xFE, 0x00, 0x00, 0x00, 0x00		  // CP A, (HL), OR A, u8
	};
	auto cart = std::make_unique<Mbc0>(rom_data);
	bus->load_cart(std::move(cart));
	// expected state structure
	struct expected {
		RegisterName8Bit r;
		uint8_t value_expected;
		uint8_t flags_expected;
		int cycles_taken;
	};
	// tables driven testing
	std::vector<expected> expected_state {
		//B							C						   D						  E
		{A, 0x11, 0b1100'0000, 4}, {A, 0x11, 0b1100'0000, 4}, {A, 0x11, 0b1100'0000, 4}, {A, 0x11, 0b1100'0000, 4},
		//H							L						   A						  E
		{A, 0x11, 0b0101'0000, 4}, {A, 0x11, 0b0100'0000, 4}, {A, 0x11, 0b1100'0000, 4},
		{A, 0x11, 0b0100'0000, 8}, {A, 0x11, 0b0100'0000, 8},
	};

	// run the test for all ld r, r opcodes
	for (const auto & exp : expected_state) {
		auto cycles_taken = cpu.step(1);
		ASSERT_EQ(cycles_taken, exp.cycles_taken);
		ASSERT_EQ(cpu.read_byte(exp.r), exp.value_expected);
		ASSERT_EQ(cpu.read_byte(F), exp.flags_expected);
	}
}

