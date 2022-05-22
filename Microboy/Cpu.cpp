#include <fmt/core.h>

#include "Cpu.h"
#include "Opcode.h"

//-----------------------------------------------------
// Flags Methods
//-----------------------------------------------------
void Flag::from_byte(uint8_t byte) {
	C = (byte & 0x10) >> 4;
	H = (byte & 0x20) >> 5;
	N = (byte & 0x40) >> 6;
	Z = (byte & 0x80) >> 7;
	pad = 0;
}
uint8_t Flag::to_byte() {
	uint8_t byte = 0;
	byte |= C << 4;
	byte |= H << 5;
	byte |= N << 6;
	byte |= Z << 7;
	return byte;
}

inline bool calc_8_bit_carry(uint16_t byte1, uint16_t byte2, uint8_t carry=0) {
	return ((((byte1 & 0xFF) + (byte2 & 0xFF) + (carry & 0xFF)) & 0x100) == 0x100);
}

inline bool calc_8_bit_hcarry(uint8_t byte1, uint8_t byte2, uint8_t carry=0) {
	return ((((byte1 & 0x0F) + (byte2 & 0x0F) + (carry & 0x0f)) & 0x10) == 0x10);
}

inline bool calc_8_bit_borrow(uint8_t byte1, uint8_t byte2, uint8_t carry=0) {
	return ((byte2 + carry) > byte1);
}

inline bool calc_8_bit_hborrow(uint8_t byte1, uint8_t byte2, uint8_t carry= 0) {
	return ((((byte1 & 0x0F) - (byte2 & 0x0F) - (carry & 0x0F)) & 0x10) == 0x10);
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

// flag setting operations
void Cpu::set_flag_c(bool set) {
	if (set) {
		m_flags.C = 1;
		return;
	}
	m_flags.C = 0;
}

void Cpu::set_flag_h(bool set) {
	if (set) {
		m_flags.H = 1;
		return;
	}
	m_flags.H = 0;
}

void Cpu::set_flag_z(bool set) {
	if (set) {
		m_flags.Z = 1;
		return;
	}
	m_flags.Z = 0;
}

void Cpu::set_flag_n(bool set) {
	if (set) {
		m_flags.N = 1;
		return;
	}
	m_flags.N = 0;
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
	fmt::print("Opcode: {:#02x}: {}\n", m_opcode, CYCLE_TABLE_DEBUG[m_opcode].name);
}

int Cpu::decode() {
	// decode the instruction and set the instruction to ease execution
	int cycles = 0;
	uint16_t inc_pc = 0;
	if (m_opcode == 0xcb) {
		++m_PC;
		m_opcode = m_bus->read_byte(m_PC);
		fmt::print("0xCB prefixed {:#02x}: {}\n", m_opcode, CYCLE_TABLE_DEBUG_CB[m_opcode].name);
		inc_pc = CYCLE_TABLE_DEBUG_CB[m_opcode].len;
		cycles = CYCLE_TABLE_DEBUG_CB[m_opcode].cycles;
		m_is_cb = true;
		m_PC += inc_pc;
		return cycles;
	}

	inc_pc = CYCLE_TABLE_DEBUG[m_opcode].len;
	cycles = CYCLE_TABLE_DEBUG[m_opcode].cycles;

	// TODO - add the things we need to make decoding instructions easier
	m_r1 = static_cast<RegisterName8Bit>((m_opcode >> 3) & 0x7);
	m_r2 = static_cast<RegisterName8Bit>(m_opcode & 0x7);
	// will only work for BC, DE, HL, SP NOTE: AF and HL will need to be handled separately
	m_r16 = static_cast<RegisterName16Bit>((m_opcode >> 4) & 7);
	imm_u8 = m_bus->read_byte(m_PC + 1);
	imm_u16 = m_bus->read_word(m_PC + 1);

	// Increment the PC to the next instruction // TODO - verify this doesn't cause issues
	m_PC += inc_pc;

	return cycles;
}

// Return any extra cycles taken due to memory read/writes
int Cpu::execute() {
	if (m_is_cb) {
		m_is_cb = false;
		// handle 0xcb prefix
		return 0;
	}
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
	// LD (HL), u8
	case 0x36:
	{
		m_bus->write_byte(read_word(HL), imm_u8);
		break;
	}
	// LD R, u16
	case 0x01: case 0x11: case 0x21: case 0x31:
	{
		write_word(m_r16, imm_u16);
		break;
	}
	// LD (BC, DE), A - Note: (BC, DE)
	case 0x02: case 0x12:
	{
		m_bus->write_byte(
			read_word(m_r16),
			read_byte(A)
		);
		break;
	}
	// LD (HL+), A
	// LD (HL-), A
	case 0x22: case 0x32:
	{
		uint16_t hl = read_word(HL);
		m_bus->write_byte(hl, read_byte(A));
		if (m_opcode == 0x22) { write_word(HL, hl + 1); }
		else if (m_opcode == 0x32) { write_word(HL, hl - 1); }
		break;
	}
	// LD A, (BC, DE)
	case 0x0A: case 0x1A:
	{
		write_byte(
			A,
			m_bus->read_byte(
				read_word(m_r16)
			));
		break;
	}
	// LD A, (HL+)
	case 0x2A: case 0x3A:
	{
		uint16_t hl = read_word(HL);
		write_byte(
			A,
			m_bus->read_byte(HL)
		);
		if (m_opcode == 0x2a) { write_word(HL, hl + 1); }
		else if (m_opcode == 0x3a) { write_word(HL, hl - 1); }
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
	case 0x70: case 0x71: case 0x72:
	case 0x73: case 0x74: case 0x75: case 0x77:
	{
		m_bus->write_byte(read_word(HL), read_byte(m_r2));
		break;
	}
	// LD (0xFF00 + u8), A
	case 0xE0:
	{
		m_bus->write_byte(IO_BASE + imm_u8, read_byte(A));
		break;
	}
	// LD A, (0xFF00 + u8)
	case 0xF0:
	{
		write_byte(A, m_bus->read_byte(IO_BASE + imm_u8));
		break;
	}
	// LD (0xFF00 + C), A
	case 0xE2:
	{
		m_bus->write_byte(IO_BASE + read_byte(C), read_byte(A));
		break;
	}
	// LD A, (0xFF00 + C)
	case 0xF2:
	{
		write_byte(A, m_bus->read_byte(IO_BASE + read_byte(C)));
		break;
	}
	// LD (u16), A
	case 0xEA:
	{
		m_bus->write_byte(imm_u16, read_byte(A));
		break;
	}
	// LD A, (u16)
	case 0xFA:
	{
		write_byte(A, m_bus->read_byte(imm_u8));
		break;
	}
	// LD HL, SP + i8
	case 0xF8:
	{
		write_word(HL, m_bus->read_word(m_SP + (int8_t)imm_u8));
		// Update Flags for add
		// TODO - Refactor
		set_flag_c(calc_8_bit_carry(m_SP, (int8_t)imm_u8));
		set_flag_h(calc_8_bit_hcarry(m_SP, (int8_t)imm_u8));
		m_flags.Z = 0;
		m_flags.N = 0;
		break;
	}
	// LD (u16), SP
	case 0x08:
	{
		m_bus->write_word(imm_u16, m_SP);
		break;
	}
	// LD (SP), HL
	case 0xF9:
	{
		write_word(SP, read_word(HL));
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
	/*-------------------- Control flow Instructions ------------------*/
	// Call
	case 0xCD:
	{
		opcode_call(imm_u16);
		break;
	}
	// Call NZ, u16
	case 0xC4:
	{
		if (!m_flags.Z) {
			opcode_call(imm_u16);
			return 12;
		}
		break;
	}
	// Call NC, u16
	case 0xD4:
	{
		if (!m_flags.C) {
			opcode_call(imm_u16);
			return 12;
		}
		break;
	}
	// Call Z, u16
	case 0xCC:
	{
		if (m_flags.Z) {
			opcode_call(imm_u16);
			return 12;
		}
		break;
	}
	// Call C, u16
	case 0xDC:
	{
		if (m_flags.C) {
			opcode_call(imm_u16);
			return 12;
		}
		break;
	}
	// RET
	case 0xC9:
	{
		opcode_ret();
		break;
	}
	// RETI
	case 0xD9:
	{
		opcode_ret();
		IME = true;
		break;
	}
	// RET NZ
	case 0xC0:
	{
		if (!m_flags.Z) {
			opcode_ret();
			return 12;
		}
		break;
	}
	// RET NC
	case 0xD0:
	{
		if (!m_flags.C) {
			opcode_ret();
			return 12;
		}
		break;
	}
	// RET Z
	case 0xC8:
	{
		if (m_flags.Z) {
			opcode_ret();
			return 12;
		}
		break;
	}
	// RET C
	case 0xD8:
	{
		if (m_flags.C) {
			opcode_ret();
			return 12;
		}
		break;
	}
	// RST
	case 0xC7: case 0xD7: case 0xE7: case 0xF7:
	case 0xCF: case 0xDF: case 0xEF: case 0xFF:
	{
		static uint16_t RST_ADDR[8] = {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38};
		opcode_call((m_opcode >> 3) & 7);
		break;
	}
	// JP i8
	case 0x18:
	{
		m_PC += (int8_t)imm_u8;
		break;
	}
	// JP NZ, i8
	case 0x20:
	{
		if (!m_flags.Z) {
			m_PC += (int8_t)imm_u8;
			return 4;
		}
		break;
	}
	// JP Z, i8
	case 0x28:
	{
		if (m_flags.Z) {
			m_PC += (int8_t)imm_u8;
			return 4;
		}
		break;
	}
	// JP NC, i8
	case 0x30:
	{
		if (!m_flags.C) {
			m_PC += (int8_t)imm_u8;
			return 4;
		}
		break;
	}
	// JP C, i8
	case 0x38:
	{
		if (m_flags.C) {
			m_PC += (int8_t)imm_u8;
			return 4;
		}
		break;
	}
	// JP u16
	case 0xC3:
	{
		m_PC = imm_u16;
		break;
	}
	// JP NZ, u16
	case 0xC2:
	{
		if (!m_flags.Z) {
			m_PC = imm_u16;
			return 4;
		}
		break;
	}
	// JP NC, u16
	case 0xD2:
	{
		if (!m_flags.C) {
			m_PC = imm_u16;
			return 4;
		}
		break;
	}
	// JP Z, u16
	case 0xCA:
	{
		if (m_flags.Z) {
			m_PC = imm_u16;
			return 4;
		}
		break;
	}
	// JP C, u16
	case 0xDA:
	{
		if (m_flags.C) {
			m_PC = imm_u16;
			return 4;
		}
		break;
	}
	// JP HL
	case 0xE9:
	{
		m_PC = read_word(HL);
		break;
	}
	/*-------------------- Special Instructions --------------------*/
	// HALT
	case 0x76:
	{
		m_halted = true;
		break;
	}
	/*-------------------- Arithmetic Instructions --------------------*/
	// ADD A, R // TODO - enhancement refactor into fn
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
	{
		opcode_add(read_byte(A), read_byte(m_r2));
		break;
	}
	// ADD A, (HL)
	case 0x86:
	{
		opcode_add(read_byte(A),  m_bus->read_byte(read_word(HL)));
		break;
	}
	// ADC A, u8
	case 0xC6:
	{
		opcode_add(read_byte(A), imm_u8);
		break;
	}
	// ADC A, R
	case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
	{
		opcode_adc(read_byte(A), read_byte(m_r2));
		break;
	}
	// ADC A, (HL)
	case 0x8E:
	{
		opcode_adc(read_byte(A), m_bus->read_byte(read_word(HL)));
		break;
	}
	// ADC A, u8
	case 0xCE:
	{
		opcode_adc(read_byte(A), imm_u8);
		break;
	}
	// SUB A, R
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
	{
		opcode_sub(read_byte(A), read_byte(m_r2));
		break;
	}
	// SUB A, (HL)
	case 0x96:
	{
		opcode_sub(read_byte(A), m_bus->read_byte(read_word(HL)));
		break;
	}
	// SUB A, u8
	case 0xD6:
	{
		opcode_sub(read_byte(A), imm_u8);
		break;
	}
	case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
	{
		opcode_sbc(read_byte(A), read_byte(m_r2));
		break;
	}
	case 0x9E:
	{
		opcode_sbc(read_byte(A), m_bus->read_byte(read_word(HL)));
		break;
	}
	case 0xDE:
	{
		opcode_sbc(read_byte(A), imm_u8);
		break;
	}
	// AND A, R
	case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
	{
		opcode_and(read_byte(A) & read_byte(m_r2));
		break;
	}
	// AND A, (HL)
	case 0xA6:
	{
		opcode_and(read_byte(A) & m_bus->read_byte(read_word(HL)));
		break;
	}
	// AND A, u8
	case 0xE6:
	{
		opcode_and(read_byte(A) & imm_u8);
		break;
	}
	// XOR A, R
	case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
	{
		opcode_xor(read_byte(A) ^ read_byte(m_r2));
		break;
	}
	// XOR A, (HL)
	case 0xAE:
	{
		opcode_xor(read_byte(A) ^ m_bus->read_byte(read_word(HL)));
		break;
	}
	// XOR A, u8
	case 0xEE:
	{
		opcode_xor(read_byte(A) ^ imm_u8);
		break;
	}
	// OR A, r
	case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
	{
		opcode_or(read_byte(A) | read_byte(m_r2));
		break;
	}
	// OR A, (HL)
	case 0xB6:
	{
		opcode_or(read_byte(A) | m_bus->read_byte(read_word(HL)));
		break;
	}
	// OR A, u8
	case 0xF6:
	{
		opcode_or(read_byte(A) | imm_u8);
		break;
	}
	// CP A, r
	case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
	{
		opcode_cp(read_byte(A), read_byte(m_r2));
		break;
	}
	// CP A, (HL)
	case 0xBE:
	{
		opcode_cp(read_byte(A), m_bus->read_byte(read_word(HL)));
		break;
	}
	// CP A, (u8)
	case 0xFE:
	{
		opcode_cp(read_byte(A), imm_u8);
		break;
	}

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
	opcode_push(PC);
	m_PC = addr;
}

void Cpu::opcode_ret() {
	opcode_pop(PC);
}

void Cpu::opcode_add(uint8_t a, uint8_t b) {
	uint8_t add_a_r = a + b;
	write_byte(A, add_a_r);
	set_flag_z(add_a_r == 0);
	m_flags.N = 0;
	set_flag_c(calc_8_bit_carry(a, b));
	set_flag_h(calc_8_bit_hcarry(a, b));
}

void Cpu::opcode_adc(uint8_t a, uint8_t b) {
	uint8_t add_a_r = a + b + m_flags.C;
	write_byte(A, add_a_r);
	set_flag_z(add_a_r == 0);
	m_flags.N = 0;
	set_flag_c(calc_8_bit_carry(a, b, m_flags.C));
	set_flag_h(calc_8_bit_hcarry(a, b, m_flags.C));

}
void Cpu::opcode_sub(uint8_t a, uint8_t b) {
	uint8_t sub_a_r = a - b;
	write_byte(A, sub_a_r);
	set_flag_z(sub_a_r == 0);
	m_flags.N = 1;
	set_flag_h(calc_8_bit_hborrow(a, b));
	set_flag_c(calc_8_bit_borrow(a, b));
}

void Cpu::opcode_sbc(uint8_t a, uint8_t b) {
	uint8_t sub_a_r = a - b - m_flags.C;
	write_byte(A, sub_a_r);
	set_flag_z(sub_a_r == 0);
	m_flags.N = 1;
	set_flag_h(calc_8_bit_hborrow(a, b, m_flags.C));
	set_flag_c(calc_8_bit_borrow(a, b, m_flags.C));
}
void Cpu::opcode_and(uint8_t a) {
	write_byte(A, a);
	m_flags.from_byte(0);
	set_flag_z(a == 0);
	m_flags.H = 1;
}

void Cpu::opcode_xor(uint8_t a) {
	write_byte(A, a);
	m_flags.from_byte(0);
	set_flag_z(a == 0);
}

void Cpu::opcode_or(uint8_t a) {
	write_byte(A, a);
	m_flags.from_byte(0);
	set_flag_z(a == 0);
}

void Cpu::opcode_cp(uint8_t a, uint8_t b) {
	set_flag_z(a - b == 0);
	m_flags.N = 1;
	set_flag_c(calc_8_bit_borrow(a, b));
	set_flag_h(calc_8_bit_hborrow(a, b));
}
