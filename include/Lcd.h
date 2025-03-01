#ifndef _LCD_H_
#define _LCD_H_
#include <cstdint>

#include "common.h"

enum class LcdMode {
    HBLANK,
    VBLANK,
    OAM_SEARCH,
    DATA_TRANSFER
};

// LCD
constexpr int LCDC_ADDR = 0xFF40;
constexpr int STAT_ADDR = 0xFF41;
constexpr int SCY_ADDR = 0xFF42;
constexpr int SCX_ADDR = 0xFF43;
constexpr int LY_ADDR = 0xFF44;
constexpr int LYC_ADDR = 0xFF45; 
constexpr int DMA_ADDR = 0xFF46;
constexpr int BGP_ADDR = 0xFF47;
constexpr int OBJ0_ADDR = 0xFF48;
constexpr int OBJ1_ADDR = 0xFF49;
constexpr int WY_ADDR = 0xFF4A;
constexpr int WX_ADDR = 0xFF4B;

// reference PANDOCS on whether registers are R/W
struct Lcd {
    // LCDC functions
    bool lcdc_lcd_enabled()    { return is_bit_set(LCDC, 7); }// ((LCDC >> 7) & 1) == 1; } // bit 7
    bool lcdc_window_tilemap() { return is_bit_set(LCDC, 6); }// ((LCDC >> 6) & 1) == 1; } // bit 6
    bool lcdc_window_enable()  { return is_bit_set(LCDC, 5); }// ((LCDC >> 6) & 1) == 1; } // bit 5
    bool lcdc_bg_tile_data()   { return is_bit_set(LCDC, 4); }// ((LCDC >> 4) & 1) == 1; } // bit 4
    bool lcdc_bg_tilemap()     { return is_bit_set(LCDC, 3); }// ((LCDC >> 3) & 1) == 1; } // bit 3
    bool lcdc_obj_size()       { return is_bit_set(LCDC, 2); }// ((LCDC >> 2) & 1) == 1; } // bit 2
    bool lcdc_obj_enable()     { return is_bit_set(LCDC, 1); }// ((LCDC >> 1) & 1) == 1; } // bit 1
    bool lcdc_bg_enable_pri()  { return is_bit_set(LCDC, 0); }// ((LCDC) & 1) == 1; } // bit 0

    // Stat register
    LcdMode stat_get_lcd_mode() { return static_cast<LcdMode>(STAT & 0x3); }
    void stat_set_lcd_mode(LcdMode mode) { 
        STAT &= 0xFC;
        STAT |= static_cast<uint8_t>(mode);
    } 
    bool stat_get_lyc_int_enabled() { return is_bit_set(STAT, 6); }// ((STAT >> 6) & 1) == 1; }
    bool stat_get_oam_int_enabled() { return is_bit_set(STAT, 5); }// ((STAT >> 5) & 1) == 1; }
    bool stat_get_vblank_int_enabled() { return is_bit_set(STAT, 4); }//((STAT >> 4) & 1) == 1; }
    bool stat_get_hblank_int_enabled() { return is_bit_set(STAT, 3); }//((STAT >> 3) & 1) == 1; }
    bool stat_ly_compare() { 
        if (LY == LYC) {
            STAT = set_bit(STAT, 2); //|= 1 << 2;
            return true;
        }
        STAT &= ~(1 << 2);
        return false;
    }

    /* LCDC 0xFF40
     * 7	LCD and PPU enable	0=Off, 1=On
     * 6	Window tile map area	0=9800-9BFF, 1=9C00-9FFF
     * 5	Window enable	0=Off, 1=On
     * 4	BG and Window tile data area	0=8800-97FF, 1=8000-8FFF
     * 3	BG tile map area	0=9800-9BFF, 1=9C00-9FFF
     * 2	OBJ size	0=8x8, 1=8x16
     * 1	OBJ enable	0=Off, 1=On
     * 0	BG and Window enable/priority	0=Off, 1=On
     */
    uint8_t LCDC {0};
    /* LCD Stat register 0xFF41
     * Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
     * Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
     * Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
     * Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
     * Bit 2 - LYC=LY Flag                          (0=Different, 1=Equal) (Read Only)
     * Bit 1-0 - Mode Flag                          (Mode 0-3, see below) (Read Only)
     *           0: HBlank
     *           1: VBlank
     *           2: Searching OAM
     *           3: Transferring Data to LCD Controller
     */
    uint8_t STAT {0};
    uint8_t SCY  {0};   // 0xFF40    
    uint8_t SCX  {0};   // 0xFF41
    uint8_t LY   {0};   // 0xFF44 [Readonly] holds values 0-153, 144-153 indicate vblank
    uint8_t LYC  {0};   // 0xFF45 Sets LYC, when LY and LYC are the same a STAT interrupt is requested
    uint8_t BGP  {0};   // 0xFF47 Background palette
    uint8_t OBP0 {0};   // 0xFF48
    uint8_t OBP1 {0};   // 0xFF49
    uint8_t WY   {0};   // 0xFF4A
    uint8_t WX   {0};   // 0xFF4B
};


#endif
