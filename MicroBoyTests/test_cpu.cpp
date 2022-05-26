#include "pch.h"
#include "../Microboy/Cpu.h"

TEST(CpuTests, TestFlagReadWrite) {
	Cpu cpu{};
	cpu.write_byte(F, 0xF0);
	uint8_t ret = cpu.read_byte(F);
	ASSERT_EQ(ret, 0xF0);
}
TEST(CpuTests, CpuTestRegReadWriteByte) {
	Cpu cpu{};

	std::vector<uint8_t> reg_state{ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x70, 0x08 };
	for (uint8_t i = B; i <= A; ++i) {
		cpu.write_byte((RegisterName8Bit)i, reg_state.at(i));
		ASSERT_EQ(cpu.read_byte((RegisterName8Bit)i), reg_state.at(i));
	}
}

TEST(CpuTests, CpuTestRegReadWriteWord) {
	Cpu cpu{};

	std::vector<uint16_t> reg_state{ 0xDEAD, 0xBEEF, 0xCAFE, 0xFADE, 0xFEE0 };
	for (uint8_t i = BC; i <= AF; ++i) {
		cpu.write_word((RegisterName16Bit)i, reg_state.at(i));
		ASSERT_EQ(cpu.read_word((RegisterName16Bit)i), reg_state.at(i));
	}
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
