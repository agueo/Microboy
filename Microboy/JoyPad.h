#ifndef JOYPAD_H
#define JOYPAD_H

#include <memory>
#include "InterruptObserver.h"

enum class JoyPadInput {
    DOWN,
    UP,
    LEFT,
    RIGHT,
    START,
    SELECT,
    A,
    B,
};

class JoyPad {
public:
    void reset() { m_joyp = 0x3f; }
    void connect_interrupt_observer(std::shared_ptr<InterruptObserver> obs) { m_int_obs = obs; }
    void handle_press(JoyPadInput input);
    void handle_release(JoyPadInput input);
    uint8_t read_byte() { return m_joyp; }
    void write_byte(uint8_t val) { m_joyp = val & 0x30; }

private:
    void select_dir() { m_joyp &= ~(0x1 << 4); m_joyp |= 0x1 << 5; }
    void select_action() { m_joyp &= ~(0x1 << 5); m_joyp |= 0x1 << 4;}
    uint8_t m_joyp = 0x3F;
    std::shared_ptr<InterruptObserver> m_int_obs;
};

#endif