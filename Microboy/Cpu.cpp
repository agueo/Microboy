#include <fmt/core.h>
#include "Cpu.h"
#include "Opcode.h"

//-----------------------------------------------------
// Flags Methods
//-----------------------------------------------------
void Flag::from_byte(uint8_t byte) {
	Z = (byte & 0x10) >> 4;
	N = (byte & 0x20) >> 5;
	H = (byte & 0x40) >> 6;
	C = (byte & 0x80) >> 7;
	pad = 0;
}
uint8_t Flag::to_byte() {
	uint8_t byte = 0;
	byte |= Z << 4;
	byte |= N << 5;
	byte |= H << 6;
	byte |= C << 7;
	return byte;
}

//-----------------------------------------------------
// Cpu Methods
//-----------------------------------------------------
void Cpu::connect_bus(std::shared_ptr<MemoryBus> bus) {
	m_bus = bus;
}

int Cpu::step(int cycles) {
	int cycles_taken = 0;
	while (cycles_taken < cycles) {
		// handle interrupts and halt
		fetch();
		cycles_taken += decode();
		cycles_taken += execute();
	}

	return cycles_taken;
}

void Cpu::reset() {
	m_flags.from_byte(0x00);
	m_reg[B] = 0x01;
	m_reg[C] = 0x00;
	m_reg[D] = 0x13;
	m_reg[E] = 0xD8;
	m_reg[H] = 0x01;
	m_reg[L] = 0x4D;
	m_reg[A] = 0x01;
	m_PC = 0x100;
	m_SP = 0xFFFE;
}

//--------------------------------------------------
// Opcode handlers
//--------------------------------------------------

inline int Unimplemented_Opcode(uint8_t opcode) {
	fmt::print("unimplemented opcode {:#02x}-{}", opcode, CYCLE_TABLE_DEBUG[opcode].name);
	exit(-1);
}

inline int Unimplemented_Opcode_CB(uint8_t opcode) {
	fmt::print("unimplemented opcode {:#02x}-{}", opcode, CYCLE_TABLE_DEBUG_CB[opcode].name);
	exit(-1);
}

//--------------------------------------------------
// Cpu functions
//--------------------------------------------------

uint8_t Cpu::read_byte(RegisterName8Bit reg) {
	if (reg == F) return m_flags.to_byte();
	return m_reg[reg];
}

void Cpu::write_byte(RegisterName8Bit reg, uint8_t value) {
	if (reg == F) { m_flags.from_byte(value & 0xF0); return; }
	m_reg[reg] = value;
}

uint16_t Cpu::read_word(RegisterName16Bit reg) {
	switch (reg) {
	case BC: return m_reg[B] << 8 | m_reg[C];
	case DE: return m_reg[D] << 8 | m_reg[E];
	case HL: return m_reg[H] << 8 | m_reg[L];
	case AF: return m_reg[A] << 8 | m_flags.to_byte();
	case SP: return m_SP;
	case PC: return m_PC;
	default: return 0;
	}
}

void Cpu::write_word(RegisterName16Bit reg, uint16_t value) {
	if (reg == SP) { m_SP = value;  return; }
	switch (reg) {
	case BC: { m_reg[B] = (value >> 8) & 0xFF; m_reg[C] = value & 0xFF; return; }
	case DE: { m_reg[D] = (value >> 8) & 0xFF; m_reg[E] = value & 0xFF; return; }
	case HL: { m_reg[H] = (value >> 8) & 0xFF; m_reg[L] = value & 0xFF; return; }
	case AF: { m_reg[A] = (value >> 8) & 0xFF; m_flags.from_byte(value & 0xF0); return; }
	case SP: { m_SP = value; return; }
	case PC: { m_PC = value; return; }
	}
}

inline void Cpu::fetch() {
	m_opcode = m_bus->read_byte(m_PC);
}

int Cpu::decode() {
	// decode the instruction and set the instruction to ease execution
	int cycles = 0;
	uint16_t inc_pc = 0;
	if (m_opcode == 0xcb) {
		++m_PC;
		m_opcode = m_bus->read_byte(m_PC);
		inc_pc = CYCLE_TABLE_DEBUG_CB[m_opcode].len;
		cycles = CYCLE_TABLE_DEBUG_CB[m_opcode].cycles;
	} else {
		inc_pc = CYCLE_TABLE_DEBUG[m_opcode].len;
		cycles = CYCLE_TABLE_DEBUG[m_opcode].cycles;
	}

	// TODO - add the things we need to make decoding instructions easier
	m_r1 = static_cast<RegisterName8Bit>((m_opcode >> 3) & 0x7);
	m_r2 = static_cast<RegisterName8Bit>(m_opcode & 0x7);
	m_r16 = static_cast<RegisterName16Bit>((m_opcode >> 4) & 7); // will only work for BC, DE, HL, SP NOTE: AF and HL will need to be handled separately
	imm_u8 = m_bus->read_byte(m_PC + 1);
	imm_u16 = m_bus->read_word(m_PC + 1);

	// Increment the PC to the next instruction // TODO - verify this doesn't cause issues
	m_PC += inc_pc;

	return cycles;
}

// Return any extra cycles taken due to memory read/writes
int Cpu::execute() {
	switch (m_opcode) {
	case 0: { return 0;  }
	/*-------------------- Load Instructions 8/16 bit --------------------*/
	// LD R, R
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47: // ld b, r
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4f: // ld c, r
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57: // ld d, r
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5f: // ld e, r
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67: // ld h, r
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6f: // ld l, r
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7f: // ld l, r
	{
		write_byte(m_r1, read_byte(m_r2));
		break;
	}
	// LD R, u8
	case 0x06: case 0x16: case 0x26: case 0x0E: case 0x1E: case 0x2E: case 0x3E:
	{
		write_byte(m_r1, imm_u8);
		break;
	}
	// LD R, u16
	case 0x01: case 0x11: case 0x21: case 0x31:
	{
		write_word(m_r16, imm_u16);
		break;
	}
	// LD R, (HL)
	case 0x46: case 0x56: case 0x66:
	case 0x4E: case 0x5E: case 0x6E: case 0x7E:
	{
		uint8_t read_val = m_bus->read_byte(read_word(HL));
		write_byte(m_r1, read_val);
		break;
	}
	// LD (HL), R
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
	{
		m_bus->write_byte(read_word(HL), read_byte(m_r2));
		break;
	}
	/*-------------------- Stack Instructions -------------------------*/
	// POP R16
	case 0xC1: case 0xD1: case 0xE1:
	{
		RegisterName16Bit op = static_cast<RegisterName16Bit>((m_opcode >> 4 )& 3);
		opcode_pop(op);
		break;
	}
	// POP AF
	case 0xF1:
	{
		opcode_pop(AF);
		break;
	}
	// PUSH R16
	case 0xC5: case 0xD5: case 0xE5:
	{
		RegisterName16Bit op = static_cast<RegisterName16Bit>((m_opcode >> 4) & 3);
		opcode_push(op);
		break;
	}
	// PUSH AF
	case 0xF5:
	{
		opcode_push(AF);
		break;
	}
	/*-------------------- Arithmetic Instructions --------------------*/
	default: Unimplemented_Opcode(m_opcode);
	}
	return 0;
}

void Cpu::opcode_push(RegisterName16Bit reg) {
	uint16_t value = read_word(reg);
	m_SP -= 2;
	m_bus->write_word(m_SP, value);
}

void Cpu::opcode_pop(RegisterName16Bit reg) {
	uint16_t value = m_bus->read_word(m_SP);
	write_word(reg, value);
	m_SP += 2;
}

void Cpu::opcode_call(uint16_t addr) {
	opcode_push(static_cast<RegisterName16Bit>(m_PC));
	m_PC = addr;
}
