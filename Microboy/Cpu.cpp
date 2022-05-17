#include "Cpu.h"

// Flags Struct
void Flag::from_byte(uint8_t byte) {
	Z = byte  & 0b00010000 >> 4;
	N = byte  & 0b00100000 >> 5;
	H = byte  & 0b01000000 >> 6;
	C = byte  & 0b10000000 >> 7;
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


void Cpu::connect_bus(std::shared_ptr<MemoryBus> bus) {
	bus_m = bus;
}

int Cpu::step(int cycles) {
	int cycles_taken = 0;
	while (cycles_taken < cycles) {
		// handle interrupts and halt
		fetch();
		decode();
		cycles_taken += execute();
		++cycles_taken;
	}

	return cycles_taken;
}

void Cpu::reset() {
	reg.A = 0x01;
	reg.F.from_byte(0x00);
	reg.B = 0x01;
	reg.C = 0x00;
	reg.D = 0x13;
	reg.E = 0xD8;
	reg.H = 0x01;
	reg.L = 0x4D;
	PC = 0x100;
	SP = 0xFFFE;
}

void Cpu::fetch() {
	// Fetch data
	return;
}

int Cpu::decode() {
	// decode the instruction and set the instruction to ease execution
	return 0;
}

int Cpu::execute() {
	// execute instruction;
	return 0;
}

