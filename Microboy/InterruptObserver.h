#ifndef INTERRUPT_OBSERVER_H
#define INTERRUPT_OBSERVER_H

#include <memory>

// ordered highest priority to lowest
enum class InterruptSource {
    VBLANK,
    LCD_STAT,
    TIMER,
    SERIAL,
    JOYPAD,
};

// Interrupt Addresses
constexpr int IF_ADDR	= 0xFF0F;
constexpr int IE_ADDR	= 0xFFFF;

/*
 * InterruptObserver
 * Responsible for scheduling and setting interrupt flags
 */
class InterruptObserver {
public:
    void reset();
    void schedule_interrupt(InterruptSource src);
    uint8_t read_byte(uint16_t addr);
    void write_byte(uint16_t addr, uint8_t val);

private:
    uint8_t m_if = 0xE1;
    uint8_t m_ie = 0x00;
};


#endif