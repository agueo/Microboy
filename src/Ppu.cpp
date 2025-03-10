
#include "common.h"
#include "Lcd.h"
#include "MemoryBus.h"
#include "Ppu.h"

#include <vector>

// colors in ARGB
const std::vector<uint32_t> gPalette{0xff89d795, 0xff629A6A, 0xff3B5C40, 0xff141F15};
//const std::vector<uint32_t> gPalette{0xff9a9e3f, 0xff496b22, 0xff0e450b, 0xff1b2a09};

Ppu::Ppu()
: m_frame_ready{false},
  LX{0},
  WLY{0},
  m_sprites_visible{0},
  m_vram_blocked{false},
  m_oam_blocked{false},
  m_dots{0},
  m_mode{LcdMode::HBLANK},
  m_lcd{},
  m_vram(0x2000, 0),
  m_oam(0xA0, 0),
  m_oam_table{0},
  m_bus{},
  m_frame_buffer(dmg::WIDTH * dmg::HEIGHT, 0),
  m_int_observer{nullptr} {}

// TODO - FIXME Change this function to not run in a loop ??
bool Ppu::step(int cycles) {
    // cycles are in T cycles,
    m_frame_ready = false;
    while (cycles > 0) {
        switch(m_mode) {
            case LcdMode::HBLANK:
                cycles = ppu_mode_hblank(cycles);
                break;
            case LcdMode::VBLANK:
                cycles = ppu_mode_vblank(cycles);
                break;
            case LcdMode::OAM_SEARCH:
                cycles = ppu_mode_oam_search(cycles);
                break;
            case LcdMode::DATA_TRANSFER:
                cycles = ppu_mode_data_xfer(cycles);
                break;
        }
    }
    return m_frame_ready;
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
    // std::fill(m_oam_table.begin(), m_oam_table.end(), 0);
    m_oam_table.clear();
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
        m_oam[addr - OAM_BASE] = value;
    }
}

void Ppu::request_dma_transfer(uint8_t addr) {
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
        m_mode = LcdMode::HBLANK;
        m_vram_blocked = false;
        m_oam_blocked = false;
        stat_int = m_lcd.stat_get_hblank_int_enabled();
        break;
    case LcdMode::VBLANK:
        m_frame_ready = true;
        m_mode = LcdMode::VBLANK;
        m_int_observer->schedule_interrupt(InterruptSource::VBLANK);
        stat_int = m_lcd.stat_get_vblank_int_enabled();
        break;
    case LcdMode::DATA_TRANSFER:
        m_mode = LcdMode::DATA_TRANSFER;
        //m_vram_blocked = true;
        break;
    case LcdMode::OAM_SEARCH:
        //m_oam_blocked = true;
        m_mode = LcdMode::OAM_SEARCH;
        //std::fill(m_oam_table.begin(), m_oam_table.end(), 0);
        m_oam_table.clear();
        // TODO - check when the interrupt is fired
        stat_int = m_lcd.stat_get_oam_int_enabled();
        break;
    }
    if (stat_int) {
        m_int_observer->schedule_interrupt(InterruptSource::LCD_STAT);
    }
}

int Ppu::ppu_mode_hblank(int cycles) {

    int remaining_cycles = SCAN_LINE_CYCLES - (PIXEL_TRANSFER_CYCLES + OAM_CYCLES + m_dots);
    if (cycles >= remaining_cycles) {
        cycles -= remaining_cycles;
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
    } else {
        m_dots += cycles;
        cycles = 0;
    }

    return cycles;
}

int Ppu::ppu_mode_vblank(int cycles) {
    int remaining_cycles = SCAN_LINE_CYCLES - m_dots;
    if (cycles >= remaining_cycles) {
        cycles -= remaining_cycles;
        m_dots = 0;
        if (++m_lcd.LY > dmg::HEIGHT + VBLANK_LINES) {
            m_lcd.LY = 0;
            WLY = 0;
            ppu_switch_mode(LcdMode::OAM_SEARCH);
        }
    } else {
        m_dots += cycles;
        cycles = 0;
    }
    return cycles;
}

int Ppu::ppu_mode_oam_search(int cycles) {
    int remaining_cycles = OAM_CYCLES - m_dots;
    if (cycles >= remaining_cycles) {
        // we have more cycles to run
        // consume the cycles remaining and return the rest for other modes to run
        cycles -= remaining_cycles;
        m_dots = 0;
        // DO OAM search here
        for(size_t i = 0; i < m_oam.size(); i += 4) {
            m_oam_table.emplace_back(m_oam[i], m_oam[i+1], m_oam[i+2], m_oam[i+3]);
        }
        ppu_switch_mode(LcdMode::DATA_TRANSFER);
    } else {
        // consume the cycles
        m_dots += cycles;
        cycles = 0;
    }
    return cycles;
}

int Ppu::ppu_mode_data_xfer(int cycles) {
    int remaining_cycles = PIXEL_TRANSFER_CYCLES - m_dots;
    if (cycles >= remaining_cycles) {
        // consume the remaining cycles
        cycles -= remaining_cycles;
        m_dots = 0;
        while(LX <= dmg::WIDTH) {
            // this function will change modes for us
            render_background();
            render_window();
            render_sprites();
            ++LX;
        }
        m_sprites_visible = 0;
        LX = 0;
        ppu_switch_mode(LcdMode::HBLANK);
    } else {
        // consume some cycles
        m_dots += cycles;
        cycles = 0;
    }
    return cycles;
    // TODO - do other stuff here for window and sprite
}

// This is rendering the background
void Ppu::render_background() {
    // m_vram_blocked = false;
    // if background enable bit is not set we don't render
    if (m_lcd.lcdc_bg_enable_pri() == 0) {
        return;
    }

    auto bus = m_bus.lock();
    uint16_t tilemap = m_lcd.lcdc_bg_tilemap() ? TILEMAP_1 : TILEMAP_2;
    uint16_t tiledata = m_lcd.lcdc_bg_tile_data() ? TILE_DATA_BASE_1 : TILE_DATA_BASE_2;

    // select which tilemap to get data from
    uint8_t tilemap_row = ((m_lcd.LY + m_lcd.SCY) / 8) % 32;
    uint8_t tilemap_col = ((LX + m_lcd.SCX) / 8) % 32;

    // tile index
    uint16_t tilemap_addr = tilemap + (tilemap_row * 32) + tilemap_col;

    uint16_t tile_addr = 0;
    if (tiledata == 0x8000) {
        uint8_t tile_index = bus->read_byte(tilemap_addr);
        tile_addr = tiledata + (tile_index * 16);
    } else {
        // 0x8800 addressing uses 0x9000 as a base with range -128 to 127
        int8_t tile_index = bus->read_byte(tilemap_addr);
        tile_addr = tiledata + (tile_index * 16);
    }

    // get the tile data bytes here we choose which byte from the tile we want for drawing 
    // ie: tile_addr + (pixel_y (0-7) * 2) (0 - 14) will choose which byte out of the 16 bytes 
    // since we are reading 8 bit we get low data then high data
    uint8_t pixel_y = (m_lcd.LY + m_lcd.SCY) % 8;
    uint8_t tile_row_data_low = bus->read_byte(tile_addr + (pixel_y * 2));
    uint8_t tile_row_data_high = bus->read_byte(tile_addr + (pixel_y * 2) + 1);

    // we choose the x pixel within (0-7)
    uint8_t pixel_x = (LX + m_lcd.SCX) % 8;

    // we get the value we need to get from the palette here
    // since we draw from left to right we get the leftmost bits from the tile_row_data
    uint8_t color_val = (((tile_row_data_high >> (7 - pixel_x)) << 1) | (tile_row_data_low >> (7 - pixel_x))) & 0x03;

    // grab a color from the palette
    // BGP holds the value we need 00-11 for which color to use
    uint32_t color = gPalette[(m_lcd.BGP >> (2 * color_val)) & 3];

    // output the pixel to the buffer
    m_frame_buffer[m_lcd.LY * dmg::WIDTH + LX] = color;
}

void Ppu::render_window() {

    if (m_lcd.lcdc_window_enable() == 0) {
        // return early since the window is not enabled
        return;
    }

    // only render if we are past the WY set
    if (WLY < m_lcd.WY) {
        return;
    }

    // if lx has not reached the wx we skip
    if (m_lcd.WX > LX + 7) {
        return;
    }

    // wx+7 is the same as LX = 0
    // TODO - fixme
    uint16_t WLX = LX;//+ 7; 
    auto bus = m_bus.lock();

    uint16_t tilemap = m_lcd.lcdc_window_tilemap() ? TILEMAP_1 : TILEMAP_2;
    uint16_t tiledata = m_lcd.lcdc_bg_tile_data() ? TILE_DATA_BASE_1 : TILE_DATA_BASE_2;

    uint8_t tilemap_row = (WLY / 8) % 32;
    uint8_t pixel_y = (WLY) % 8;

    uint8_t tilemap_col = (WLX / 8) % 32;

    // tile index
    uint16_t tilemap_addr = tilemap + (tilemap_row * 32) + tilemap_col;

    uint16_t tile_addr = 0;
    if (tiledata == 0x8000) {
        uint8_t tile_index = bus->read_byte(tilemap_addr);
        tile_addr = tiledata + (tile_index * 16);
    } else {
        int8_t tile_index = bus->read_byte(tilemap_addr);
        // tilesize = 16
        tile_addr = tiledata + (tile_index * 16);
    }

    // get the tile data bytes
    uint8_t tile_row_data_high = bus->read_byte(tile_addr + (pixel_y * 2) + 1);
    uint8_t tile_row_data_low = bus->read_byte(tile_addr + (pixel_y * 2));

    uint8_t pixel_x = (WLX) % 8;
    // The data is 
    uint8_t color_val = (((tile_row_data_high >> (7 - pixel_x)) << 1)| (tile_row_data_low >> (7 - pixel_x))) & 0x03;

    // grab a color from the palette
    uint32_t color = gPalette[(m_lcd.BGP >> (2 * color_val)) & 3];
    // output the pixel to the buffer
    m_frame_buffer[m_lcd.LY * dmg::WIDTH + WLX] = color;
}

void Ppu::render_sprites() {
    if (m_lcd.lcdc_obj_enable() == 0) {
        // return early since the object drawing is not enabled
        return;
    }
    if (m_sprites_visible == 10) {
        return;
    }

    auto bus = m_bus.lock();
    int sprite_visible = 0;
    uint16_t tiledata = TILE_DATA_BASE_1;
    uint8_t pixel_x;
    uint8_t pixel_y;
    uint16_t tile_addr = 0;
    uint8_t tile_row_data_high;
    uint8_t tile_row_data_low;
    uint8_t color_val;
    uint8_t palette;
    uint32_t color;

    OamAttribute *attr = nullptr;
    // find an object
    for (auto it = m_oam_table.begin(); it != m_oam_table.end(); ++it) {
        attr = &(*it);
        // if object is 8x8 and not at y=16 it's hidden
        if (attr->y_pos < 16 && !m_lcd.lcdc_obj_size()) {
            continue;
        }

        // if ly is already ahead of this object or we still havent reached it skip it
        if (attr->y_pos + 16 < m_lcd.LY || attr->y_pos > m_lcd.LY) {
            continue;
        }

        // if lx is 0 and attr->xpos = 8 then we draw the object at 0
        if (LX >= attr->x_pos - 8 && LX <= attr->x_pos) {
            ++sprite_visible;
            break;
        }
    }
    if (!sprite_visible) {
        return;
    }

    tile_addr = tiledata + (attr->tile_index * 16);
    pixel_y = m_lcd.LY % 8; // TODO thisis probably wrong
    palette = is_bit_set(attr->attributes, 4) ?  m_lcd.OBP1 : m_lcd.OBP1;
    
    // get the tile data bytes
    tile_row_data_high = bus->read_byte(tile_addr + (pixel_y * 2) + 1);
    tile_row_data_low = bus->read_byte(tile_addr + (pixel_y * 2));

    pixel_x = LX % 8;
    // TODO Maybe do the flipping here
    color_val = (((tile_row_data_high >> (7 - pixel_x)) << 1)| (tile_row_data_low >> (7 - pixel_x))) & 0x03;

    // grab a color from the palette
    color = gPalette[(palette >> (2 * color_val)) & 3];

    // output the pixel to the buffer
    // TODO - priority
    // check priority if set it will only draw above bgp 0
    // so grab the underlying color and check what bgp position it is. if 0 draw else skip
    m_frame_buffer[m_lcd.LY * dmg::WIDTH + LX] = color;
    ++m_sprites_visible;
}