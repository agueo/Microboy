
#include "common.h"
#include "Lcd.h"
#include "MemoryBus.h"
#include "Ppu.h"

#include <algorithm>
#include <vector>
#include <fmt/core.h>

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
  m_was_window_drawn{false},
  m_current_sprite{nullptr},
  m_sprite_rendering_started{false},
  m_vram(0x2000, 0),
  m_oam(0xA0, 0),
  m_oam_table{0},
  m_bus{},
  m_frame_buffer(dmg::WIDTH * dmg::HEIGHT, 0),
  m_int_observer{nullptr} {}

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
    uint16_t source_addr = addr * 0x100;
    auto p = m_bus.lock();
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
        m_current_sprite = nullptr;
        m_sprite_rendering_started = false;
        break;
    case LcdMode::OAM_SEARCH:
        //m_oam_blocked = true;
        m_mode = LcdMode::OAM_SEARCH;
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

void Ppu::search_oam() {
    // DO OAM search here
    uint8_t pos_x;
    int real_pos_y;
    int real_pos_x;
    uint8_t obj_size;
    m_sprites_visible = 0;
    for(size_t i = 0; i < m_oam.size(); i += 4) {
        // if we already have 10 sprites skip
        if (m_sprites_visible == 10) { 
            break; 
        }
        pos_x = m_oam[i+1];
        real_pos_y = m_oam[i] - 16;
        real_pos_x = m_oam[i+1] - 8;
        obj_size = is_bit_set(m_lcd.lcdc_obj_size(), 2) ? 16 : 8;

        // if our current LY is within the object we want to display it
        if (is_between(m_lcd.LY, real_pos_y, real_pos_y + obj_size)) {
            // we will skip on the objects that are hidden in the x axis
            if (pos_x == 0 || pos_x >= 168) {
                continue;
            }

            // save other attributes related to drawing 
            m_oam_table.emplace_back(real_pos_y, real_pos_x, m_oam[i+2], m_oam[i+3]);
            ++m_sprites_visible;
        }
    }

    // if table is empty we don't do anything else
    if (m_oam_table.empty()) {
        return;
    }

    // sort the table to be from smallest to biggest
    std::sort(m_oam_table.begin(), m_oam_table.end(), [] (const OamAttribute &a, const OamAttribute &b) {
        return a.x_pos < b.x_pos;
    });
}

int Ppu::ppu_mode_oam_search(int cycles) {
    int remaining_cycles = OAM_CYCLES - m_dots;
    if (cycles >= remaining_cycles) {
        // we have more cycles to run
        // consume the cycles remaining and return the rest for other modes to run
        cycles -= remaining_cycles;
        m_dots = 0;
        m_oam_table.clear();
        search_oam();
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
        if (m_was_window_drawn) {
            ++WLY;
        }
        m_was_window_drawn = false;
        m_sprites_visible = 0;
        LX = 0;
        ppu_switch_mode(LcdMode::HBLANK);
    } else {
        // consume some cycles
        m_dots += cycles;
        cycles = 0;
    }
    return cycles;
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
    uint8_t tile_row_data_high = bus->read_byte(tile_addr + (pixel_y * 2));
    uint8_t tile_row_data_low = bus->read_byte(tile_addr + (pixel_y * 2) + 1);

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

    // return early since the window is not enabled
    if (m_lcd.lcdc_window_enable() == 0) {
        return;
    }

    // only render if we are past the WY set
    if (m_lcd.LY < m_lcd.WY) {
        return;
    }

    // if lx has not reached the wx we skip
    if (LX + 7 < m_lcd.WX) {
        return;
    }

    m_was_window_drawn = true;

    uint16_t WLX = LX + 7 - m_lcd.WX; 
    auto bus = m_bus.lock();

    uint16_t tilemap = m_lcd.lcdc_window_tilemap() ? TILEMAP_1 : TILEMAP_2;
    uint16_t tiledata = m_lcd.lcdc_bg_tile_data() ? TILE_DATA_BASE_1 : TILE_DATA_BASE_2;

    uint8_t tilemap_row = (WLY / 8) % 32;
    uint8_t pixel_y = (WLY) % 8;
    uint8_t pixel_x = (WLX) % 8;

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
    uint8_t tile_row_data_high = bus->read_byte(tile_addr + (pixel_y * 2));
    uint8_t tile_row_data_low = bus->read_byte(tile_addr + (pixel_y * 2) + 1);

    // The data is 
    uint8_t color_val = (((tile_row_data_high >> (7 - pixel_x)) << 1)| (tile_row_data_low >> (7 - pixel_x))) & 0x03;

    // grab a color from the palette
    uint32_t color = gPalette[(m_lcd.BGP >> (2 * color_val)) & 3];
    // output the pixel to the buffer
    m_frame_buffer[m_lcd.LY * dmg::WIDTH + LX] = color;
}

void Ppu::render_sprites() {
    // return early since the object drawing is not enabled
    if (m_lcd.lcdc_obj_enable() == 0) {
        return;
    }
    // if there are no sprites skip
    if (m_oam_table.empty()) {
        return;
    }

    bool needs_change = false;
    if (m_sprite_rendering_started) {
        if (m_current_sprite == nullptr) {
            return;
        }
        // we have started but we don't have a null ptr
        // check if we are still within the lx range
        if (!is_between(LX, m_current_sprite->x_pos, m_current_sprite->x_pos + 8)) {
            needs_change = true;
        }
    } else {
        // have not started and likely have a null ptr
        needs_change = true;
    }

    if (needs_change) {
        const auto sprite = std::find_if(m_oam_table.begin(), m_oam_table.end(), [*this](const OamAttribute &sprite) { 
            return is_between(LX, sprite.x_pos, sprite.x_pos + 8); 
        });

        if (sprite == m_oam_table.end()) {
            // m_current_sprite = nullptr;
            return;
        }
        m_current_sprite = &(*sprite);
        m_sprite_rendering_started = true;
    } // keep the same object on else

    // check the priority and skip if we can't draw
    if (is_bit_set(m_current_sprite->attributes, SPRITE_BG_PRI)) {
        // check the current color in the fb
        uint32_t bg_color = m_frame_buffer[(m_lcd.LY * dmg::WIDTH) + LX];
        // check whether the color corresponds to the bg 
        // if bit 0 does not == bg color we can skip writing the sprite
        if (gPalette[m_lcd.BGP & 0x3] != bg_color) {
            return;
        }
    }

    auto bus = m_bus.lock();
    uint16_t tiledata = TILE_DATA_BASE_1;
    uint16_t tile_addr = 0;
    uint8_t tile_row = 0;

    uint8_t tile_row_data_high;
    uint8_t tile_row_data_low;
    uint8_t color_val;
    uint8_t palette;
    uint32_t color;

    // we check if y flip flag
    if (is_bit_set(m_current_sprite->attributes, SPRITE_Y_FLIP)) {
        // if 1 we y flip it 
        tile_row = (m_lcd.lcdc_obj_size() ? 15 : 7) - (m_lcd.LY - m_current_sprite->y_pos);
    } else {
        tile_row = m_lcd.LY - m_current_sprite->y_pos;
    }

    // check the height flag and modify the tile_index accordingly
    if (m_lcd.lcdc_obj_size()) {
        m_current_sprite->tile_index &= 0xFE;
    }
    tile_addr = tiledata + (m_current_sprite->tile_index * 16) + tile_row * 2;


    // get the tile data bytes
    tile_row_data_low = bus->read_byte(tile_addr + 1);
    tile_row_data_high = bus->read_byte(tile_addr);

    // TODO - find out why we needthe 1
    uint8_t sprite_offset = LX - m_current_sprite->x_pos - 1;
    // drawing location offset - this is accounting for the the x-flip
    uint8_t offset = is_bit_set(m_current_sprite->attributes, SPRITE_X_FLIP) ? sprite_offset : 7 - (sprite_offset);
    color_val = (((tile_row_data_high >> (offset)) << 1)| (tile_row_data_low >> (offset))) & 0x03;

    // sprites don't have transparent color so skip if color is 0
    if (!color_val) {
        return;
    }

    // grab a color from the palette
    palette = is_bit_set(m_current_sprite->attributes, SPRITE_BGP) ?  m_lcd.OBP1 : m_lcd.OBP0;
    color = gPalette[(palette >> (2 * color_val)) & 3];

    m_frame_buffer[(m_lcd.LY * dmg::WIDTH) + LX] = color;
}