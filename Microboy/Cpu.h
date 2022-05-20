#pragma once

#include <cstdint>
#include <memory>
#include "MemoryBus.h"

struct Flag {
	uint8_t C : 1;
	uint8_t H : 1;
	uint8_t N : 1;
	uint8_t Z : 1;
	uint8_t pad : 4;

	void from_byte(uint8_t byte);
	uint8_t to_byte();
	// flag setting operations
};

enum RegisterName8Bit : uint8_t {
	B, C, D, E, H, L, F, A
};

// TODO - When we need to parse the different group types 
// we may need to overload the read_word and write_word methods
enum RegisterName16Bit : uint8_t {
	BC, DE, HL, SP, AF, PC
};

class Cpu {
public:
	void connect_bus(std::shared_ptr<MemoryBus> bus);
	int step(int cycles);
	void reset();
	// void clock(); // maybe needed
	bool is_halted() { return m_halted; }
	
	// read and write functions for registers
	uint8_t read_byte(RegisterName8Bit reg);
	void write_byte(RegisterName8Bit reg, uint8_t value);
	uint16_t read_word(RegisterName16Bit reg);
	void write_word(RegisterName16Bit reg, uint16_t value);

private:
	
	//--------------------
	// member functions
	//--------------------
	void fetch();
	int decode();
	int execute();

	void opcode_push(RegisterName16Bit reg);
	void opcode_pop(RegisterName16Bit reg);
	void opcode_call(uint16_t addr);
	void opcode_ret();

	//--------------------
	// Data members
	//--------------------
	uint8_t m_reg[8];
	Flag m_flags;
	uint16_t m_SP;
	uint16_t m_PC;

	// Create an instruction class that holds everything it needs to execute
	uint8_t m_opcode;
	bool m_is_cb;
	// useful things for executing instructions
	// TODO look through all the instructions and glean out the common ones
	// TODO - which registers 8 bit src and dest
	RegisterName8Bit m_r1;
	RegisterName8Bit m_r2;
	RegisterName16Bit m_r16;
	uint8_t imm_u8;
	uint16_t imm_u16;

	// TODO - which registers 16 bit
	// TODO - an address?
	// TODO - a signed value

	bool m_halted;
	bool IME;

	// Bus connection
	std::shared_ptr<MemoryBus> m_bus;
};

