#include <fmt/core.h>
#include "InterruptObserver.h"
#include "JoyPad.h"
#include "Lcd.h"
#include "MemoryBus.h"
#include "Ppu.h"
#include "Timer.h"

MemoryBus::MemoryBus() 
    : wram(0x2000, 0), IO(0x80, 0), hram(0x7F, 0),
    cart{nullptr}, m_joypad{nullptr}, m_timer{nullptr}, m_int_observer{nullptr}
{}

void MemoryBus::reset() {
    cart = nullptr;
    m_joypad->reset();
    m_int_observer->reset();
    m_timer->reset();
    m_ppu->reset();

    // All hardware registers at PC 0x100
    // Serial
    IO[SB_ADDR - IO_BASE] 	= 0x00; // SB
    IO[SC_ADDR - IO_BASE] 	= 0x7E; // SC
    // Skipping sound registers
}

void MemoryBus::load_cart(std::unique_ptr<Cartridge> c) {
    cart = std::move(c);
}

void MemoryBus::connect_interrupt_observer(std::shared_ptr<InterruptObserver> observer) {
    m_int_observer = observer;
}

void MemoryBus::connect_joypad(std::shared_ptr<JoyPad> joypad) { 
    m_joypad = joypad;
}

void MemoryBus::connect_ppu(std::shared_ptr<Ppu> ppu) {
    m_ppu = ppu;
}

void MemoryBus::connect_timer(std::shared_ptr<Timer> timer) {
    m_timer = timer;
}

uint8_t MemoryBus::read_byte(uint16_t addr) {
    if (addr >= ROM_BASE && addr <= ROM_END) {
        return cart->read_byte(addr);
    }
    else if (addr >= EXRAM_BASE && addr <= EXRAM_END) {
        return cart->read_byte(addr);
    }
    else if (addr >= VRAM_BASE && addr <= VRAM_END) {
        return m_ppu->read_byte(addr);
    }
    else if (addr >= WRAM_BASE && addr <= WRAM_END) {
        return wram[addr - WRAM_BASE];
    }
    else if (addr >= ECHO_BASE && addr <= ECHO_END) {
        return wram[addr - ECHO_BASE];
    }
    else if (addr >= HRAM_BASE && addr <= HRAM_END) {
        return hram[addr - HRAM_BASE];
    }
    else if (addr >= IO_BASE && addr <= IO_END) {
		uint8_t data;
		switch (addr) {
		case JOYP_ADDR:
            data = m_joypad->read_byte();
			break;
		case IF_ADDR:
			data = m_int_observer->read_byte(IF_ADDR);
			break;
        case DIV_ADDR:
        case TIMA_ADDR:
        case TMA_ADDR:
        case TAC_ADDR:
            data = m_timer->read_byte(addr);
            break;
        case LCDC_ADDR:
        case STAT_ADDR:
        case SCY_ADDR:
        case SCX_ADDR:
        case LY_ADDR:
        case LYC_ADDR:
        case BGP_ADDR:
        case OBJ0_ADDR:
        case OBJ1_ADDR:
        case WY_ADDR:
        case WX_ADDR:
            data = m_ppu->read_byte(addr);
            break;
        case 0xFF4D:  // KEY1 - used to switch speed in CGB mode
            // STUB to 0xFF
            data = 0xFF;
            break;
		default:
        	data = IO[addr - IO_BASE];
			break;
		}
		return data;
    }
    else if (addr >= OAM_BASE && addr <= OAM_END) {
        return m_ppu->read_byte(addr);
    }
    else if (addr == IE_ADDR) {
        return m_int_observer->read_byte(IE_ADDR);
    } else if (addr >= PROHIB_BASE && addr <= PROHIB_END) {
        return 00;
    } else {
       //fmt::print("Illegal memory access: {:#04x}\n", addr);
       exit(-1);
    }
    return 0;
}

void MemoryBus::write_byte(uint16_t addr, uint8_t value) {
    if (addr >= ROM_BASE && addr <= ROM_END) {
        cart->write_byte(addr, value);
    }
    else if (addr >= EXRAM_BASE && addr <= EXRAM_END) {
        cart->write_byte(addr, value);
    }
    else if (addr >= VRAM_BASE && addr <= VRAM_END) {
        m_ppu->write_byte(addr, value);
    }
    else if (addr >= WRAM_BASE && addr <= WRAM_END) {
        wram[addr - WRAM_BASE] = value;
    }
    else if (addr >= ECHO_BASE && addr <= ECHO_END) {
        wram[addr - ECHO_BASE] = value;
    }
    else if (addr >= HRAM_BASE && addr <= HRAM_END) {
        hram[addr - HRAM_BASE] = value;
    }
    else if (addr >= IO_BASE && addr <= IO_END) {
		switch(addr) {
		case JOYP_ADDR:
            m_joypad->write_byte(value);
			break;
		case IF_ADDR:
            m_int_observer->write_byte(IF_ADDR, value);
			break;
        case DIV_ADDR:
        case TIMA_ADDR:
        case TMA_ADDR:
        case TAC_ADDR:
            m_timer->write_byte(addr, value);
            break;
        case LCDC_ADDR:
        case STAT_ADDR:
        case SCY_ADDR:
        case SCX_ADDR:
        case LY_ADDR:
        case LYC_ADDR:
        case BGP_ADDR:
        case OBJ0_ADDR:
        case OBJ1_ADDR:
        case WY_ADDR:
        case WX_ADDR:
            m_ppu->write_byte(addr, value);
            break;
        case DMA_ADDR:
        	//IO[addr - IO_BASE] = value;
            m_ppu->write_byte(addr, value);
            break;
		default:
        	IO[addr - IO_BASE] = value;
			break;
		}
        if (addr == SB_ADDR) {
            fmt::print("{:c}", value);
        }
    }
    else if (addr >= OAM_BASE && addr <= OAM_END) {
        m_ppu->write_byte(addr, value);
    } else if (addr == IE_ADDR) {
        m_int_observer->write_byte(IE_ADDR, value);
    } else {
       //fmt::print("Illegal memory access: {:#04x}\n", addr);
    }
    return;
}

uint16_t MemoryBus::read_word(uint16_t addr) {
    uint16_t lo, hi;
    lo = read_byte(addr);
    hi = read_byte(addr + 1);
    return ((hi << 8) & 0xFF00) | (lo & 0xFF);
}

void MemoryBus::write_word(uint16_t addr, uint16_t value) {
    // lo write
    write_byte(addr, value & 0xFF);
    // hi write
    write_byte(addr + 1, (value >> 8) & 0xFF);
}
