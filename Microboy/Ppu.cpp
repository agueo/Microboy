
#include "common.h"
#include "Lcd.h"
#include "MemoryBus.h"
#include "Ppu.h"

#include <fmt/core.h>
#include <vector>

const std::vector<uint32_t> gPalette{0x89d795FF, 0x629A6AFF, 0x3B5C40FF, 0x141F15FF};

Ppu::Ppu() 
    : frame_ready{false}, LX{0}, m_vram_blocked{false}, m_oam_blocked{false}, m_dots{0}, 
    m_mode{LcdMode::HBLANK}, m_lcd{}, m_vram(0x2000, 0), m_oam(0xA0, 0), 
    m_pixel_fifo{0}, m_oam_fifo{0}, m_bus{}, m_frame_buffer(WIDTH*HEIGHT, 0), 
    m_int_observer{nullptr} {}

bool Ppu::step(int cycles) {
    // cycles are in T cycles,
    frame_ready = false;
    for (int i = 0; i < cycles; i += 4) {
        switch(m_mode) {
            case LcdMode::HBLANK:
                ppu_mode_hblank();
                break;
            case LcdMode::VBLANK:
                ppu_mode_vblank();
                break;
            case LcdMode::OAM_SEARCH:
                ppu_mode_oam_search();
                break;
            case LcdMode::DATA_TRANSFER:
                ppu_mode_data_xfer();
                break;
        }
    }
    return frame_ready;
}

void Ppu::reset() {
    m_vram_blocked = false;
    m_oam_blocked = false;
    m_dots = 0;
    m_mode = LcdMode::HBLANK;
    m_lcd.LCDC = 0x91;
    m_lcd.STAT = 0x85;
    m_lcd.SCY = 0x00;
    m_lcd.SCX = 0x00;
    m_lcd.LY = 0x90;
    m_lcd.LYC = 0x00;
    m_lcd.BGP = 0xFC;
    m_lcd.OBP0 = 0x00;
    m_lcd.OBP1 = 0x00;
    m_lcd.WY = 0x00;
    m_lcd.WX = 0x00;
    std::fill(m_vram.begin(), m_vram.end(), 0);
    std::fill(m_oam.begin(), m_oam.end(), 0);
    m_pixel_fifo.clear();
    m_oam_fifo.clear();
    std::fill(m_frame_buffer.begin(), m_frame_buffer.end(), gPalette[0]);
}

uint8_t Ppu::read_byte(uint16_t addr) {
    if (addr == LCDC_ADDR) {
        return m_lcd.LCDC;
    } else if (addr == STAT_ADDR) {
        return m_lcd.STAT;
    } else if (addr == SCY_ADDR) {
        return m_lcd.SCY;
    } else if (addr == SCX_ADDR) {
        return m_lcd.SCX;
    } else if (addr == LY_ADDR) {
        return m_lcd.LY;
    } else if (addr == LYC_ADDR) {
        return m_lcd.LYC;
    } else if (addr == BGP_ADDR) {
        return m_lcd.BGP;
    } else if (addr == OBJ0_ADDR) {
        return m_lcd.OBP0;
    } else if (addr == OBJ1_ADDR) {
        return m_lcd.OBP1;
    } else if (addr == WY_ADDR) {
        return m_lcd.WY;
    } else if (addr == WX_ADDR) {    
        return m_lcd.WX;
    } else if (addr >= VRAM_BASE && addr <= VRAM_END) {
        if (m_vram_blocked) return 0xFF;
        return m_vram[addr - VRAM_BASE];
    } else if (addr >= OAM_BASE && addr <= OAM_END) {
        if (m_oam_blocked) return 0xFF;
        return m_oam[addr - OAM_BASE];
    }
    return 0;
}

void Ppu::write_byte(uint16_t addr, uint8_t value) {
    if (addr == LCDC_ADDR) {
        m_lcd.LCDC = value;
    } else if (addr == STAT_ADDR) {
        uint8_t old_stat = m_lcd.STAT;
        m_lcd.STAT = value & 0xFC;
        m_lcd.STAT |= old_stat & 0x03;
    } else if (addr == SCY_ADDR) {
        m_lcd.SCY = value;
    } else if (addr == SCX_ADDR) {
        m_lcd.SCX = value;
    } else if (addr == LY_ADDR) {
        return;
    } else if (addr == LYC_ADDR) {
        m_lcd.LYC = value;
    } else if (addr == DMA_ADDR) {
        request_dma_transfer(value);
    } else if (addr == BGP_ADDR) {
        m_lcd.BGP = value;
    } else if (addr == OBJ0_ADDR) {
        m_lcd.OBP0 = value;
    } else if (addr == OBJ1_ADDR) {
        m_lcd.OBP1 = value;
    } else if (addr == WY_ADDR) {
        m_lcd.WY = value;
    } else if (addr == WX_ADDR) {
        m_lcd.WX = value;
    } else if (addr >= VRAM_BASE && addr <= VRAM_END) {
        if (m_vram_blocked) return;
        m_vram[addr - VRAM_BASE] = value;
    } else if (addr >= OAM_BASE && addr <= OAM_END) {
        if (m_oam_blocked) return;
        m_oam[addr >= OAM_BASE] = value;
    }
}

void Ppu::request_dma_transfer(uint8_t addr) {
    (void)addr;
    // TODO make this play well with the timing
    uint16_t source_addr = addr * 0x100;
    auto p = m_bus.lock();
    // TODO - if this is a bottleneck figure out how to make this faster
    for (size_t i = 0; i < m_oam.size(); ++i) {
        // maybe a std::copy
        m_oam[i] = p->read_byte(source_addr + i);
    }
}

void Ppu::ppu_switch_mode(LcdMode next) {
    bool stat_int{false};
    switch(next) {
    case LcdMode::HBLANK:
        frame_ready = true;
        m_mode = LcdMode::HBLANK;
        stat_int = m_lcd.stat_get_hblank_int_enabled();
        break;
    case LcdMode::VBLANK:
        m_mode = LcdMode::VBLANK;
        m_int_observer->schedule_interrupt(InterruptSource::VBLANK);
        stat_int = m_lcd.stat_get_vblank_int_enabled();
        break;
    case LcdMode::DATA_TRANSFER:
        m_mode = LcdMode::DATA_TRANSFER;
        // TODO - initialize pixel fifo
        break;
    case LcdMode::OAM_SEARCH:
        m_mode = LcdMode::OAM_SEARCH;
        // TODO - clear oam_buffer;
        m_oam_fifo.clear();
        stat_int = m_lcd.stat_get_oam_int_enabled();
        break;
    }
    if (stat_int) {
        m_int_observer->schedule_interrupt(InterruptSource::LCD_STAT);
    }
}

void Ppu::ppu_mode_hblank() {
    m_vram_blocked = false;
    m_oam_blocked = false;

    if (++m_dots == 204) {
        m_dots = 0;
        ++m_lcd.LY;
        ++WLY;

        // Check if LY == LYC
        if (m_lcd.stat_ly_compare() && m_lcd.stat_get_lyc_int_enabled()) {
            m_int_observer->schedule_interrupt(InterruptSource::LCD_STAT);
        }

        // If we are at line 144 we need to switch to vblank otherwise we go to oam search
        if (m_lcd.LY >= 144) {
            ppu_switch_mode(LcdMode::VBLANK);
        } else {
            ppu_switch_mode(LcdMode::OAM_SEARCH);
        }
    }
}

void Ppu::ppu_mode_vblank() {
    if (++m_dots == 456) {
        m_dots = 0;
        ++m_lcd.LY;
        if (m_lcd.LY > 153) {
            m_lcd.LY = 0;
            WLY = 0;
            ppu_switch_mode(LcdMode::OAM_SEARCH);
        }
    }
}

void Ppu::ppu_mode_oam_search() {
    m_oam_blocked = true;
    // OAM buffer can only have 10 objects
    // Do oam fetch

    if (++m_dots == 80) {
        ppu_switch_mode(LcdMode::DATA_TRANSFER);
    }
}

void Ppu::ppu_mode_data_xfer() {
    // m_vram_blocked = true;
    // Tick the fetcher to get the next few pixels
    pixel_fetcher_tick();
}

void Ppu::pixel_fetcher_tick() {
    auto bus = m_bus.lock();
    uint16_t tilemap = m_lcd.lcdc_bg_tilemap() ? 0x9c00 : 0x9800;
    uint16_t tiledata = m_lcd.lcdc_bg_tile_data() ? 0x8000 : 0x8800;
    uint8_t offy = m_lcd.LY + m_lcd.SCY;  
    uint8_t offx = LX + m_lcd.SCX;  
    uint8_t tile_no = 0;
    uint8_t color_val = 0;
    uint32_t color = 0;

    while (++LX <= WIDTH) {
        tile_no = bus->read_byte(tilemap + ((offy / 8 * 32) + (offx / 8)));
        if (tiledata == 0x8800) {
            color_val = (
                bus->read_byte(tiledata + 0x800 + ((int8_t)tile_no * 0x10) + (offy % 8 * 2)) >> (7 - (offx % 8)) & 0x1) 
                + ((bus->read_byte(tiledata + 0x800 + ((int8_t)tile_no * 0x10) + (offy % 8 * 2) + 1) >> (7 - (offx % 8)) & 0x1) * 2);
        } else {
            color_val = (
                bus->read_byte(tiledata + 0x800 + ((int8_t)tile_no * 0x10) + (offy % 8 * 2)) >> (7 - (offx % 8)) & 0x1) 
                + ((bus->read_byte(tiledata + 0x800 + ((int8_t)tile_no * 0x10) + (offy % 8 * 2) + 1) >> (7 - (offx % 8)) & 0x1) * 2);
        }
        color = gPalette[(m_lcd.BGP >> (2 * color_val)) & 3];
        m_frame_buffer[m_lcd.LY * WIDTH + offx] = color;

        offx = LX + m_lcd.SCX;
        offy = m_lcd.LY + m_lcd.SCY;  
    }
    LX = 0;
    ppu_switch_mode(LcdMode::HBLANK);
}

