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

// JOYPAD address
constexpr int JOYP_ADDR = 0xFF00;

class JoyPad {
public:
    void reset();
    void connect_interrupt_observer(std::shared_ptr<InterruptObserver> obs) { m_int_obs = obs; }
    void handle_press(JoyPadInput input);
    void handle_release(JoyPadInput input);

    uint8_t read_byte();
    void write_byte(uint8_t val);

private:
    void select_dir();
    void select_action();
    uint8_t m_joyp = 0xCF;
    uint8_t m_dir_button = 0x0F;
    uint8_t m_action_button = 0x0F;
    std::shared_ptr<InterruptObserver> m_int_obs;
};

#endif
