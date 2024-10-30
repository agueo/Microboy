#include <memory>

#include "InterruptObserver.h"
#include "Timer.h"

void Timer::reset() {
    m_div = 0xAB00;
    m_tima = 0x00;
    m_tma = 0x00;
    m_tac = 0xF8;

    m_timer_enabled = true;
    m_tima_freq = 1024;
    m_cycles_until_next_tima = 0;
}

void Timer::connect_interrupt_observer(std::shared_ptr<InterruptObserver> int_obs) {
    m_int_obs = int_obs;
}

uint8_t Timer::read_byte(uint16_t addr) {
    switch(addr) {
        case DIV_ADDR: return (uint8_t)(m_div >> 8);
        case TIMA_ADDR: return m_tima;
        case TMA_ADDR: return m_tma;
        case TAC_ADDR: return m_tac;
    }
    return 0;
}

void Timer::write_byte(uint16_t addr, uint8_t val) {
    switch(addr) {
        case DIV_ADDR:
            m_div = 0x00;
            break;
        case TIMA_ADDR:
            m_tima = val;
            break;
        case TMA_ADDR: 
            m_tma = val;
            break;
        case TAC_ADDR: 
            m_tac = val & 0x07;
            m_timer_enabled = (m_tac >> 2 == 1) ? true : false;
            switch(m_tac & 0x03) {
                case 0x00:
                    m_tima_freq = 1024;
                    break;
                case 0x01:
                    m_tima_freq = 16;
                    break;
                case 0x02:
                    m_tima_freq = 64;
                    break;
                case 0x03:
                    m_tima_freq = 256;
                    break;
            }
            m_cycles_until_next_tima = 0;
            break;
    }
}

void Timer::step(int cycles) {
    uint8_t old_tima = m_tima;
    for(int i = 0; i < cycles; ++i) {

        if (m_tima_overflow) {
            m_tima_overflow = false;
            m_int_obs->schedule_interrupt(InterruptSource::TIMER);
        }
        // Div increments every 256
        if (++m_cycles_until_next_div == DIV_FREQ) {
            ++m_div;
            m_cycles_until_next_div = 0;
        }

        if (!m_timer_enabled) {
            continue;
        }

        // Increment tima
        if(++m_cycles_until_next_tima == m_tima_freq) {
            m_cycles_until_next_tima = 0;
            old_tima = m_tima;
            ++m_tima;
            // check if overflow
            if (m_tima < old_tima) {
                m_tima = m_tma;
                m_tima_overflow = true;
            }
        }
    }
}
