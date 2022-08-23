#include <fmt/core.h>
#include "InterruptObserver.h"
#include "JoyPad.h"
#include "MemoryBus.h"

MemoryBus::MemoryBus() {
    vram.resize(0x2000);
    wram.resize(0x4000);
    oam.resize(0xA0);
    IO.resize(0x80);
    hram.resize(0x7f);
}

void MemoryBus::reset() {
    cart = nullptr;
    // All hardware registers at PC 0x100
    IO[JOYP_ADDR - IO_BASE] = 0xCF; // P1
    IO[SB_ADDR - IO_BASE] 	= 0x00; // SB
    IO[SC_ADDR - IO_BASE] 	= 0x7E; // SC
    IO[DIV_ADDR - IO_BASE] 	= 0x18; // DIV
    IO[TIMA_ADDR - IO_BASE] = 0x00; // TIMA
    IO[TMA_ADDR - IO_BASE] 	= 0x00; // TMA
    IO[TAC_ADDR - IO_BASE] 	= 0xF8; // TAC
    IO[IF_ADDR - IO_BASE] 	= 0xE1; // IF
    // Skipping sound registers
    IO[LCDC_ADDR - IO_BASE] = 0x91; // LCDC
    IO[STAT_ADDR - IO_BASE] = 0x85; // STAT
    IO[SCY_ADDR - IO_BASE] 	= 0x00; // SCY
    IO[SCX_ADDR - IO_BASE] 	= 0x00; // SCX
    IO[LY_ADDR - IO_BASE] 	= 0x94; // LY
    IO[LYC_ADDR - IO_BASE] 	= 0x00; // LYC
    IO[DMA_ADDR - IO_BASE] 	= 0xFF; // DMA
    IO[BGP_ADDR - IO_BASE] 	= 0xFC; // BGP
    IO[WY_ADDR - IO_BASE] 	= 0x00; // WY
    IO[WX_ADDR - IO_BASE] 	= 0x00; // WX
    hram[IE_ADDR - HRAM_BASE] = 0x00; // IE
}

void MemoryBus::load_cart(std::unique_ptr<Cartridge> c) {
    cart = std::move(c);
}

void MemoryBus::connect_interrupt_observer(std::shared_ptr<InterruptObserver> observer) {
    m_int_observer = observer;
}

uint8_t MemoryBus::read_byte(uint16_t addr) {
    if (addr >= ROM_BASE && addr <= ROM_END) {
        return cart->read_byte(addr);
    }
    else if (addr >= EXRAM_BASE && addr <= EXRAM_END) {
        return cart->read_byte(addr);
    }
    else if (addr >= VRAM_BASE && addr <= VRAM_END) {
        return vram[addr - VRAM_BASE];
    }
    else if (addr >= WRAM_BASE && addr <= WRAM_END) {
        return wram[addr - WRAM_BASE];
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
			data = m_int_observer->read_byte();
			break;
		default:
        	data = IO[addr - IO_BASE];
			break;
		}
		return data;
    }
    else if (addr >= OAM_BASE && addr <= OAM_END) {
        return oam[addr - OAM_BASE];
    }
    else if (addr == IE_ADDR) {
        return hram[IE_ADDR - HRAM_BASE];
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
        vram[addr - VRAM_BASE] = value;
    }
    else if (addr >= WRAM_BASE && addr <= WRAM_END) {
        wram[addr - WRAM_BASE] = value;
    }
    else if (addr >= HRAM_BASE && addr <= HRAM_END) {
        hram[addr - HRAM_BASE] = value;
    }
    else if (addr >= IO_BASE && addr <= IO_END) {
        // TODO maybe clean this up
		switch(addr) {
		case JOYP_ADDR:
            m_joypad->write_byte(value);
			break;
		case IF_ADDR:
            m_int_observer->write_byte(value);
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
        oam[addr - OAM_BASE] = value;
    }
    else if (addr == IE_ADDR) {
        hram[IE_ADDR - HRAM_BASE] = value;
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
