#include <memory>

#include "common.h"
#include "JoyPad.h"
#include "InterruptObserver.h"

void JoyPad::reset() { 
    m_joyp = 0xCF; 
    m_action_button = 0xFF;
    m_dir_button = 0xFF;
}

void JoyPad::select_dir() { 
    m_joyp = set_bit(m_joyp, 5);
    m_joyp = clear_bit(m_joyp, 4);
}

void JoyPad::select_action() { 
    m_joyp = set_bit(m_joyp, 4);
    m_joyp = clear_bit(m_joyp, 5);
}

void JoyPad::handle_press(JoyPadInput input){
    switch(input) {
    case JoyPadInput::DOWN:
        m_dir_button = clear_bit(m_dir_button, 3);
        select_dir();
        break;
    case JoyPadInput::UP:
        m_dir_button = clear_bit(m_dir_button, 2);
        select_dir();
        break;
    case JoyPadInput::LEFT:
        m_dir_button = clear_bit(m_dir_button, 1);
        select_dir();
        break;
    case JoyPadInput::RIGHT:
        m_dir_button = clear_bit(m_dir_button, 0);
        select_dir();
        break;
    case JoyPadInput::START:
        m_action_button = clear_bit(m_action_button, 3);
        select_action();
        break;
    case JoyPadInput::SELECT:
        m_action_button = clear_bit(m_action_button, 2);
        select_action();
        break;
    case JoyPadInput::A:
        m_action_button = clear_bit(m_action_button, 1);
        select_action();
        break;
    case JoyPadInput::B:
        m_action_button = clear_bit(m_action_button, 0);
        select_action();
        break;
    }
    m_int_obs->schedule_interrupt(InterruptSource::JOYPAD);

}

void JoyPad::handle_release(JoyPadInput input){
    switch(input) {
    case JoyPadInput::DOWN:
        m_dir_button = set_bit(m_dir_button, 3);
        m_joyp = set_bit(m_joyp, 4);
        break;
    case JoyPadInput::UP:
        m_dir_button = set_bit(m_dir_button, 2);
        m_joyp = set_bit(m_joyp, 4);
        break;
    case JoyPadInput::LEFT:
        m_dir_button = set_bit(m_dir_button, 1);
        m_joyp = set_bit(m_joyp, 4);
        break;
    case JoyPadInput::RIGHT:
        m_dir_button = set_bit(m_dir_button, 0);
        m_joyp = set_bit(m_joyp, 4);
        break;
    case JoyPadInput::START:
        m_action_button = set_bit(m_action_button, 3);
        m_joyp = set_bit(m_joyp, 5);
        break;
    case JoyPadInput::SELECT:
        m_action_button = set_bit(m_action_button, 2);
        m_joyp = set_bit(m_joyp, 5);
        break;
    case JoyPadInput::A:
        m_action_button = set_bit(m_action_button, 1);
        m_joyp = set_bit(m_joyp, 5);
        break;
    case JoyPadInput::B:
        m_action_button = set_bit(m_action_button, 0);
        m_joyp = set_bit(m_joyp, 5);
        break;
    }
}

uint8_t JoyPad::read_byte() {
    // check bits 4 and 5:
    uint8_t data = 0xFF;
    switch (m_joyp & 0x30) {
        case 0x10: // select buttons
            data = m_action_button;
            break;
        case 0x20: // d-pad buttons
            data = m_dir_button;
            break;
        default:
            break;
    }
    return data;
}

void JoyPad::write_byte(uint8_t data) {
    // only high nibble is writable
    m_joyp = data & 0xF0;
}