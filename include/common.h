#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

namespace dmg {

constexpr int CPU_SPEED = 4194304; // 4 MHz
constexpr int FRAMERATE = 60;
constexpr int CYCLES_PER_FRAME = CPU_SPEED / FRAMERATE;
constexpr int WIDTH = 160;
constexpr int HEIGHT = 144;
constexpr int SCALE = 4;
constexpr int SCREEN_WIDTH = WIDTH * SCALE;
constexpr int SCREEN_HEIGHT = HEIGHT * SCALE;

};

#endif