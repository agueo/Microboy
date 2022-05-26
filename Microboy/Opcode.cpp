#include "Opcode.h"
#include "Cpu.h"
#include <fmt/core.h>

/*--------------------------------------------------*/
/* Helper functions									*/
/*--------------------------------------------------*/
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

/*--------------------------------------------------*/
/* Opcode handlers									*/
/*--------------------------------------------------*/
inline int Unimplemented_Opcode(uint8_t opcode) {
	fmt::print("unimplemented opcode {:#02x}: {}", opcode, CYCLE_TABLE_DEBUG[opcode].name);
	exit(-1);
}

inline int Unimplemented_Opcode_CB(uint8_t opcode) {
	fmt::print("unimplemented opcode {:#02x}: {}", opcode, CYCLE_TABLE_DEBUG_CB[opcode].name);
	exit(-1);
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


int Cpu::handle_opcode() {
	switch (m_opcode) {
	// NOP
	case 0x00: { break;  }
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
			m_bus->read_byte(hl)
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
		set_flag_c(calc_8_bit_carry(m_SP, (int8_t)imm_u8));
		set_flag_h(calc_8_bit_hcarry((uint8_t)m_SP, (int8_t)imm_u8));
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
		RegisterName16Bit op = static_cast<RegisterName16Bit>((m_opcode >> 4) & 3);
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
		static uint16_t RST_ADDR[8] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 };
		opcode_call(RST_ADDR[(m_opcode >> 3) & 7]);
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
	// DI
	case 0xF3:
	{
		IME = false;
		break;
	}
	// EI
	case 0xFB:
	{
		IME = true;
		break;
	}
	// STOP
	case 0x10:
	{
		// don't know what this does for the DMG but could be used in the GBC
		// for now treat as halt and just loop forever
		m_halted = true;
		fmt::print("Hit Stop\n");
		break;
	}
	// DAA
	case 0x27:
	{
		uint8_t a = read_byte(A);
		// if flag N is set we subtract
		if (m_flags.N) {
			if (m_flags.C) a -= 0x60;
			if (m_flags.H) a -= 0x06;
		}
		// if otherwise we add
		else {
			if (m_flags.C || a > 0x99) { a += 0x60; m_flags.C = 1; }
			if (m_flags.H || (a & 0x0f) > 9) { a += 0x06; }
		}
		set_flag_z(a == 0);
		m_flags.H = 0;
		break;
	}
	/*-------------------- Arithmetic Instructions --------------------*/
	// INC R16
	case 0x03: case 0x13: case 0x23: case 0x33:
	{
		write_word(m_r16, read_word(m_r16) + 1);
		break;
	}
	// DEC R16
	case 0x0B: case 0x1B: case 0x2B: case 0x3B:
	{
		write_word(m_r16, read_word(m_r16) - 1);
		break;
	}
	// ADD HL, R16
	case 0x09: case 0x19: case 0x29: case 0x39:
	{
		uint16_t hl = read_word(HL);
		uint16_t r = read_word(m_r16);
		uint32_t sum = hl + r;
		write_word(HL, sum);
		if (sum > (uint16_t)(hl + r)) m_flags.C = 1;
		else m_flags.C = 0;
		if ((((hl & 0xFFF) + (r & 0xFFF)) & 0x1000) == 0x1000) m_flags.H = 1;
		else m_flags.H = 0;
		m_flags.N = 0;
		break;
	}
	// INC R
	case 0x04: case 0x14: case 0x24: case 0x0C: case 0x1C: case 0x2C: case 0x3C:
	{
		uint8_t r = read_byte(m_r1);
		write_byte(m_r1, r + 1);
		set_flag_z((uint8_t)(r + 1) == 0);
		m_flags.N = 0;
		set_flag_h(calc_8_bit_hcarry(r, 1));
		break;
	}
	// INC (HL)
	case 0x34:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		m_bus->write_byte(hl, r + 1);
		set_flag_z((uint8_t)(r + 1) == 0);
		m_flags.N = 0;
		set_flag_h(calc_8_bit_hcarry(r, 1));
		break;
	}
	// DEC R
	case 0x05: case 0x15: case 0x25: case 0x0D: case 0x1D: case 0x2D: case 0x3D:
	{
		uint8_t r = read_byte(m_r1);
		write_byte(m_r1, r - 1);
		set_flag_z((uint8_t)(r - 1) == 0);
		m_flags.N = 1;
		set_flag_h(calc_8_bit_hborrow(r, 1));
		break;
	}
	// DEC (HL)
	case 0x35:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		m_bus->write_byte(hl, r - 1);
		set_flag_z((uint8_t)(r - 1) == 0);
		m_flags.N = 1;
		set_flag_h(calc_8_bit_hborrow(r, 1));
		break;
	}
	// ADD A, R
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
	// SBC A, R
	case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
	{
		opcode_sbc(read_byte(A), read_byte(m_r2));
		break;
	}
	// SBC A, (HL)
	case 0x9E:
	{
		opcode_sbc(read_byte(A), m_bus->read_byte(read_word(HL)));
		break;
	}
	// SBC A, u8
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
	// ADD SP, i8
	case 0xE8:
	{
		uint16_t sp = read_word(SP);
		m_flags.from_byte(0);
		set_flag_c(calc_8_bit_carry(sp, imm_u8));
		set_flag_h(calc_8_bit_hcarry(sp, imm_u8));
		sp += (int8_t)imm_u8;
		write_word(SP, sp);
		break;
	}
	/*-------------------- Rotate and shift Instructions --------------------*/
	// RLCA is C<-[7<-0]<-7
	case 0x07:
	{
		uint8_t a = read_byte(A);
		uint8_t bit7 = (a >> 7) & 0x1;
		a <<= 1;
		a |= bit7;
		write_byte(A, a);
		m_flags.from_byte(0);
		m_flags.C = bit7;
		break;
	}
	// RLA is C<-[7<-0]<-C
	case 0x17:
	{
		uint8_t a = read_byte(A);
		uint8_t old_carry = m_flags.C;
		uint8_t bit7 = (a >> 7) & 0x1;
		a <<= 1;
		// only set the bit if carry was set
		if (old_carry) a |= old_carry;
		write_byte(A, a);
		m_flags.from_byte(0);
		m_flags.C = bit7;
		break;
	}
	// RRCA is [0]->[7->0]->C
	case 0x0F:
	{
		uint8_t a = read_byte(A);
		uint8_t bit0 = a & 0x1;
		a >>= 1;
		// only set the bit if bit0 was set
		if (bit0) a |= bit0 << 7;
		write_byte(A, a);
		m_flags.from_byte(0);
		m_flags.C = bit0;
		break;
	}
	// RRA is C->[7->0]->C
	case 0x1F:
	{
		uint8_t a = read_byte(A);
		uint8_t old_carry = m_flags.C;
		uint8_t bit0 = a & 0x1;
		a >>= 1;
		if (old_carry) a |= old_carry << 7;
		write_byte(A, a);
		m_flags.from_byte(0);
		m_flags.C = bit0;
		break;
	}
	// CPL A
	case 0x2F:
	{
		write_byte(A, read_byte(A));
		m_flags.N = 1;
		m_flags.H = 1;
		break;
	}
	/*-------------------- Misc Instructions --------------------*/
	// CCY (complement cy)
	case 0x3F:
	{
		m_flags.C = m_flags.C ^ 1;
		m_flags.N = 0;
		m_flags.H = 0;
		break;
	}
	// SCF (set c flag)
	case 0x37:
	{
		m_flags.C = 1;
		m_flags.N = 0;
		m_flags.H = 0;
		break;
	}
	default: Unimplemented_Opcode(m_opcode);
	}
	return 0;
}

int Cpu::handle_cb_prefix() {
	switch (m_opcode) {
		// RLC R C<-[7<-0]<-7
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x07:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		if (bit7) r |= bit7;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// RLC (HL)
	case 0x06:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		if (bit7) r |= bit7;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// RRC R [0]->[7->0]->C
	case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0F:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		if (bit0) r |= bit0 << 7;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// RRC (HL) [0]->[7->0]->C
	case 0x0E:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		if (bit0) r |= bit0 << 7;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// RL r C<-[7<-0]<-C_old
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x17:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t old_carry = m_flags.C;
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		if (old_carry) r |= old_carry;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// RL (HL) C<-[7<-0]<-C_old
	case 0x16:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t old_carry = m_flags.C;
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		if (old_carry) r |= old_carry;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// RR r C_old->[7->0]->C
	case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1F:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t old_carry = m_flags.C;
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		if (old_carry) r |= old_carry << 7;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// RR (HL) C_old->[7->0]->C
	case 0x1E:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t old_carry = m_flags.C;
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		if (old_carry) r |= old_carry << 7;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// SLA R - C <- [7 <- 0] <- 0
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x27:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// SLA (HL) - C <- [7 <- 0] <- 0
	case 0x26:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t bit7 = (r >> 7) & 0x1;
		r <<= 1;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit7;
		break;
	}
	// SRA R - [7] -> [7 -> 0] -> C
	case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2F:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t bit0 = r & 0x1;
		uint8_t bit7 = (r >> 7) & 0x1;
		r >>= 1;
		if (bit7) r |= bit7 << 7;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// SRA (HL)
	case 0x2E:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t bit0 = r & 0x1;
		uint8_t bit7 = (r >> 7) & 0x1;
		r >>= 1;
		if (bit7) r |= bit7 << 7;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// SWAP R
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x37:
	{
		uint8_t r = read_byte(m_r2);
		r = ((r & 0xF0) >> 4) | ((r & 0x0F) << 4);
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		break;
	}
	// SWAP (HL)
	case 0x36:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		r = ((r & 0xF0) >> 4) | ((r & 0x0F) << 4);
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		break;
	}
	// SRL R - 0 -> [7 -> 0] -> C
	case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3F:
	{
		uint8_t r = read_byte(m_r2);
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		write_byte(m_r2, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// SRL (HL) - 0 -> [7 -> 0] -> C
	case 0x3E:
	{
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		uint8_t bit0 = r & 0x1;
		r >>= 1;
		m_bus->write_byte(hl, r);
		m_flags.from_byte(0);
		set_flag_z(r == 0);
		m_flags.C = bit0;
		break;
	}
	// BIT u3, r8 - test bit n
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
	case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4F:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
	case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5F:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
	case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6F:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
	case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7F:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint8_t bit = (read_byte(m_r2) >> testbit) & 0x1;
		set_flag_z(bit == 0);
		m_flags.N = 0;
		m_flags.H = 1;
		break;
	}
	// BIT u3, (HL)
	case 0x46: case 0x56: case 0x66: case 0x76:
	case 0x4E: case 0x5E: case 0x6E: case 0x7E:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint8_t bit = (m_bus->read_byte(read_word(HL)) >> testbit) & 0x1;
		set_flag_z(bit == 0);
		m_flags.N = 0;
		m_flags.H = 1;
		break;
	}
	// RES u3, r
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
	case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
	case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
	case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
	case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
	case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
	case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint8_t r = read_byte(m_r2);
		r &= ~(1 << testbit);
		write_byte(m_r2, r);
		break;
	}
	// RES u3, (HL)
	case 0x86: case 0x8E: case 0x96: case 0x9E:
	case 0xA6: case 0xAE: case 0xB6: case 0xBE:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		r &= ~(1 << testbit);
		m_bus->write_byte(hl, r);
		break;
	}
	// SET u3, r
	case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC7:
	case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCF:
	case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD7:
	case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDF:
	case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE7:
	case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEF:
	case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF7:
	case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFF:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint8_t r = read_byte(m_r2);
		r &= 1 << testbit;
		write_byte(m_r2, r);
		break;
	}
	// SET u3, (HL)
	case 0xC6: case 0xCE: case 0xD6: case 0xDE:
	case 0xE6: case 0xEE: case 0xF6: case 0xFE:
	{
		uint8_t testbit = static_cast<uint8_t>(m_r1);
		uint16_t hl = read_word(HL);
		uint8_t r = m_bus->read_byte(hl);
		r &= 1 << testbit;
		m_bus->write_byte(hl, r);
		break;
	}
	default: Unimplemented_Opcode_CB(m_opcode);
	}
	return 0;
}
