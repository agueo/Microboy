#include <memory>
// #include <fmt/core.h>
#include "InterruptObserver.h"

constexpr int NUM_INTERRUPTS = 5;
static std::string interrupt_source_str[NUM_INTERRUPTS] = {
    "VBLANK",
    "LCD_STAT",
    "TIMER",
    "SERIAL",
    "JOYPAD"
};

void InterruptObserver::reset() { 
    m_if = 0xE1; 
    m_ie = 0x00;
}

void InterruptObserver::schedule_interrupt(InterruptSource src) {
    m_if |= 1 << static_cast<uint8_t>(src);
    // fmt::print("Requesting interrupt: {}\n", interrupt_source_str[static_cast<size_t>(src)]);
}

uint8_t InterruptObserver::read_byte(uint16_t addr) {
    switch (addr) {
        case IE_ADDR:
            return m_ie;
        case IF_ADDR:
            return m_if;
    }
    return 0;
}

void InterruptObserver::write_byte(uint16_t addr, uint8_t val) {
    switch (addr) {
        case IE_ADDR:
            m_ie = val;
            break;
        case IF_ADDR:
            m_if = val & 0x1F;
            break;
    }
}
