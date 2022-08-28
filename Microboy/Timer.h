#ifndef TIMER_H
#define TIMER_H

#include <memory>
#include "InterruptObserver.h"

// Timer Registers
// DIV: is incremented at a rate of 16384 hz, will inc at double speed (32768 Hz) on CGB
//      Writing to this reg resets the value to 00
//      Stop instr will also reset this value
// TIMA: This timer is incremented at the clock frequency specified by the TAC register
//      When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06) 
//      and an interrupt is requested, as described below.
// TMA: Timer Modulo register
// 
// TAC: Timer Control Register
//      Bit  2   - Timer Enable (only affects TIMA)
//      Bits 1-0 - Input Clock Select
//                 00: CPU Clock / 1024 (DMG, SGB2, CGB Single Speed Mode:   4096 Hz, SGB1:   ~4194 Hz, CGB Double Speed Mode:   8192 Hz)
//                 01: CPU Clock / 16   (DMG, SGB2, CGB Single Speed Mode: 262144 Hz, SGB1: ~268400 Hz, CGB Double Speed Mode: 524288 Hz)
//                 10: CPU Clock / 64   (DMG, SGB2, CGB Single Speed Mode:  65536 Hz, SGB1:  ~67110 Hz, CGB Double Speed Mode: 131072 Hz)
//                 11: CPU Clock / 256  (DMG, SGB2, CGB Single Speed Mode:  16384 Hz, SGB1:  ~16780 Hz, CGB Double Speed Mode:  32768 Hz)

// Timer addresses
constexpr int DIV_ADDR = 0xFF04;
constexpr int TIMA_ADDR = 0xFF05;
constexpr int TMA_ADDR = 0xFF06;
constexpr int TAC_ADDR = 0xFF07;

constexpr int DIV_FREQ = 256;

class Timer {
public:
    void reset();
    void connect_interrupt_observer(std::shared_ptr<InterruptObserver> int_obs);
    void step(int cycles);
    uint8_t read_byte(uint16_t addr);
    void write_byte(uint16_t addr, uint8_t val);
private:
    uint16_t m_div = 0xAB00; 
    uint8_t m_tima = 0x00; 
    uint8_t m_tma = 0x00; 
    uint8_t m_tac = 0xF8; 
    bool m_timer_enabled = false;
    uint16_t m_tima_freq = 1024;
    uint16_t m_cycles_until_next_tima = 0;
    uint8_t m_tima_reset_delay = 0;
    bool m_tima_overflow = false;
    std::shared_ptr<InterruptObserver> m_int_obs{nullptr};
};

#endif