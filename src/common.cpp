#include "common.h"

bool is_between(uint16_t value, uint16_t start, uint16_t end) {
    return value >= start && value <= end;
}

uint8_t low_uint8_t(uint16_t data) {
    return (uint8_t) data & 0xFF;
}

uint8_t high_uint8_t(uint16_t data) {
    return (uint8_t) (data >> 8) & 0xFF;
}

uint8_t low_nibble(uint8_t data) {
    return  data & 0x0F;
}

uint8_t high_nibble(uint16_t data) {
    return (data >> 4) & 0x0F;
}

uint16_t combine(uint8_t low, uint8_t high) {
    return (uint16_t) (high << 8) | low;
}


uint8_t clear_bit(uint8_t data, uint8_t bit) {
    return data & (~(0x01 << bit));
}

bool is_bit_set(uint8_t data, uint8_t bit) {
    return (data >> bit) & 1 ? true : false;
}

uint8_t set_bit(uint8_t data, uint8_t bit) {
    return data | (0x1 << bit);
}