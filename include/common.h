#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

namespace dmg {

constexpr int CPU_SPEED = 4194304; // 4 MHz
constexpr int FRAMERATE = 60;
constexpr int CYCLES_PER_FRAME = 70224; //CPU_SPEED / FRAMERATE;
constexpr int CYCLE_STEP = 16;
constexpr int WIDTH = 160;
constexpr int HEIGHT = 144;
constexpr int SCALE = 4;
constexpr int SCREEN_WIDTH = WIDTH * SCALE;
constexpr int SCREEN_HEIGHT = HEIGHT * SCALE;

};

// helper methods for common bitwise ops
uint8_t low_uint8_t(uint16_t data);
uint8_t high_uint8_t(uint16_t data);
uint8_t low_nibble(uint8_t data);
uint8_t high_nibble(uint8_t data);

bool is_between(uint16_t value, uint16_t begin, uint16_t end);
uint16_t combine(uint8_t low, uint8_t high);
bool is_bit_set(uint8_t data, uint8_t bit);
uint8_t set_bit(uint8_t data, uint8_t bit);
uint8_t clear_bit(uint8_t data, uint8_t bit);

#endif