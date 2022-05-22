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
		// interrupt handler
		cycles_taken += service_interrupt();
		if (m_halted) {
			cycles_taken += 4;
			break;
		}
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
	fmt::print("PC: {:#04x} Opcode: {:#02x}: {}\n", m_PC, m_opcode, CYCLE_TABLE_DEBUG[m_opcode].name);
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

	// Fetched data, registers and immediates to use
	m_r1 = static_cast<RegisterName8Bit>((m_opcode >> 3) & 0x7);
	m_r2 = static_cast<RegisterName8Bit>(m_opcode & 0x7);
	m_r16 = static_cast<RegisterName16Bit>((m_opcode >> 4) & 7);
	imm_u8 = m_bus->read_byte(m_PC + 1);
	imm_u16 = m_bus->read_word(m_PC + 1);

	// Increment the PC to the next instruction
	m_PC += inc_pc;

	return cycles;
}

// Return any extra cycles taken due to memory read/writes
int Cpu::execute() {
	if (m_is_cb) {
		m_is_cb = false;
		return handle_cb_prefix();
	}
	return handle_opcode();
}

// handle any pending interrupts
int Cpu::service_interrupt() {
	// ISR vectors
	static uint8_t isr_vectors[5]{ 0x40, 0x48, 0x50, 0x58, 0x60 };
	// Quit out early if IME is diabled
	if (IME == false) { return 0; }

	// Exit halt if IF & IE are non-zero NOTE: TODO emulate halt bug
	if (m_bus->read_byte(IE_ADDR) & m_bus->read_byte(IF_ADDR)) {
		m_halted = false;
	}

	// check each interrupt bit from highest priority to lowest
	for (int i = 0; i < 5; ++i) {
		// check IME and IF and IE bits
		uint8_t IE = m_bus->read_byte(IE_ADDR);
		uint8_t IF = m_bus->read_byte(IF_ADDR);
		if (IME &&
		   ((IE >> i) & 0x1) &&
		   ((IF >> i) & 0x1))
		{
			// ack interrupt by disabling IME
			IME = false;
			// ack interrupt request bit in IF
			IF &= ~(0x1 << i);
			opcode_call(isr_vectors[i]);
			break;
		}
	}
	return 20; // isr comsumes 5 M cycles
}

