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
};

struct Registers {
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t A;
	Flag F;

};

class Cpu {
public:
	void connect_bus(std::shared_ptr<MemoryBus> bus);
	int step(int cycles);
	void reset();
	// void clock(); // maybe needed

private:
	void fetch();
	int decode();
	int execute();
	// read and write functions
	// maybe some register functions to help facilitate things

private:
	Registers reg;
	uint16_t PC;
	uint16_t SP;

	// useful things for executing instructions
	// Create an instruction class that holds everything it needs to execute

	// Bus connection
	std::shared_ptr<MemoryBus> bus_m;
};


