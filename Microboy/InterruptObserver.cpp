#include <memory>
#include <fmt/core.h>
#include "InterruptObserver.h"

static std::string interrupt_source_str[NUM_INTERRUPTS] = {
    "VBLANK",
    "LCD_STAT",
    "TIMER",
    "SERIAL",
    "JOYPAD"
};

void InterruptObserver::schedule_interrupt(InterruptSource src) {
    switch (src) {
    case InterruptSource::VBLANK:
        m_if |= 1 << 0;  
        fmt::print("Requesting interrupt: {}\n", interrupt_source_str[0]);
        break;
    case InterruptSource::LCD_STAT:
        m_if |= 1 << 1;
        fmt::print("Requesting interrupt: {}\n", interrupt_source_str[1]);
        break;
    case InterruptSource::TIMER:
        m_if |= 1 << 2;  
        fmt::print("Requesting interrupt: {}\n", interrupt_source_str[2]);
        break;
    case InterruptSource::SERIAL:
        m_if |= 1 << 3;  
        fmt::print("Requesting interrupt: {}\n", interrupt_source_str[3]);
        break;
    case InterruptSource::JOYPAD:
        m_if |= 1 << 4;  
        fmt::print("Requesting interrupt: {}\n", interrupt_source_str[4]);
        break;
    }
}
