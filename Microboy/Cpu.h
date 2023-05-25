#ifndef CPU_H
#define CPU_H

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
};

enum RegisterName8Bit : uint8_t {
	B, C, D, E, H, L, F, A
};

enum RegisterName16Bit : uint8_t {
	BC, DE, HL, SP, AF, PC
};

class Cpu {
public:
	void connect_bus(std::shared_ptr<MemoryBus> bus);
	int step(int cycles);
	void reset();
	bool is_halted() { return m_halted; }

	// read and write functions for registers
	uint8_t read_byte(RegisterName8Bit reg);
	void write_byte(RegisterName8Bit reg, uint8_t value);
	uint16_t read_word(RegisterName16Bit reg);
	void write_word(RegisterName16Bit reg, uint16_t value);
	friend void debug_print(Cpu &cpu);

private:

	//--------------------
	// member functions
	//--------------------
	void fetch();
	int decode();
	int execute();

	int handle_opcode();
	int handle_cb_prefix();

	// common Opcodes
	void opcode_push(RegisterName16Bit reg);
	void opcode_pop(RegisterName16Bit reg);
	void opcode_call(uint16_t addr);
	void opcode_ret();
	void opcode_add(uint8_t a, uint8_t b);
	void opcode_adc(uint8_t a, uint8_t b);
	void opcode_sub(uint8_t a, uint8_t b);
	void opcode_sbc(uint8_t a, uint8_t b);
	void opcode_and(uint8_t a);
	void opcode_xor(uint8_t a);
	void opcode_or(uint8_t a);
	void opcode_cp(uint8_t a, uint8_t b);

	// interrupt service routine
	int service_interrupt();

	// flag setting operations
	void set_flag_c(bool set);
	void set_flag_h(bool set);
	void set_flag_z(bool set);
	void set_flag_n(bool set);

	//--------------------
	// Data members
	//--------------------
	uint8_t m_reg[8];
	Flag m_flags;
	uint16_t m_SP;
	uint16_t m_PC;

	uint8_t m_opcode;
	bool m_is_cb;

	// useful things for executing instructions
	RegisterName8Bit m_r1;
	RegisterName8Bit m_r2;
	RegisterName16Bit m_r16;
	uint8_t imm_u8;
	uint16_t imm_u16;

	bool m_halted;
	bool IME;
	uint8_t ei_delay;
	uint8_t ime_enable;

	// Bus connection
	std::shared_ptr<MemoryBus> m_bus;
};


#endif
