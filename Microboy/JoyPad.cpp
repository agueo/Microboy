#include <memory>

#include "JoyPad.h"
#include "InterruptObserver.h"

void JoyPad::handle_press(JoyPadInput input){
    switch(input) {
    case JoyPadInput::DOWN:
        m_joyp &= ~(0x1 << 3);
        select_dir();
        break;
    case JoyPadInput::UP:
        m_joyp &= ~(0x1 << 2);
        select_dir();
        break;
    case JoyPadInput::LEFT:
        m_joyp &= ~(0x1 << 1);
        select_dir();
        break;
    case JoyPadInput::RIGHT:
        m_joyp &= ~(0x1 << 0);
        select_dir();
        break;
    case JoyPadInput::START:
        m_joyp &= ~(0x1 << 3);
        select_action();
        break;
    case JoyPadInput::SELECT:
        m_joyp &= ~(0x1 << 2);
        select_action();
        break;
    case JoyPadInput::A:
        m_joyp &= ~(0x1 << 1);
        select_action();
        break;
    case JoyPadInput::B:
        m_joyp &= ~(0x1 << 0);
        select_action();
        break;
    }
    m_int_obs->schedule_interrupt(InterruptSource::JOYPAD);
}

void JoyPad::handle_release(JoyPadInput input){
    switch(input) {
    case JoyPadInput::DOWN:
        m_joyp |= 0x1 << 3;
        select_dir();
        break;
    case JoyPadInput::UP:
        m_joyp |= 0x1 << 2;
        select_dir();
        break;
    case JoyPadInput::LEFT:
        m_joyp |= 0x1 << 1;
        select_dir();
        break;
    case JoyPadInput::RIGHT:
        m_joyp |= 0x1 << 0;
        select_dir();
        break;
    case JoyPadInput::START:
        m_joyp |= 0x1 << 3;
        select_action();
        break;
    case JoyPadInput::SELECT:
        m_joyp |= 0x1 << 2;
        select_action();
        break;
    case JoyPadInput::A:
        m_joyp |= 0x1 << 1;
        select_action();
        break;
    case JoyPadInput::B:
        m_joyp |= 0x1 << 0;
        select_action();
        break;
    }
    m_int_obs->schedule_interrupt(InterruptSource::JOYPAD);
}