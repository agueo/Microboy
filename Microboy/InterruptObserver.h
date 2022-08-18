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

constexpr int NUM_INTERRUPTS = 5;

/*
 * InterruptObserver
 * Responsible for scheduling and setting interrupt flags
 */
class InterruptObserver {
public:
    void reset() { m_if = 0xE1; }
    void schedule_interrupt(InterruptSource src);
    uint8_t read_byte() { return m_if; }
    void write_byte(uint8_t val) { m_if = val & 0x1F; }

private:
    uint8_t m_if = 0xE1;
};


#endif