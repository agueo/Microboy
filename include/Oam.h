#ifndef _OAM_H_
#define _OAM_H_

#include <cstdint>

constexpr uint8_t SPRITE_BGP = 4;
constexpr uint8_t SPRITE_X_FLIP = 5;
constexpr uint8_t SPRITE_Y_FLIP = 6;
constexpr uint8_t SPRITE_BG_PRI = 7;

#pragma pack(push, 0)
struct OamAttribute {
    int y_pos;
    int x_pos;
    uint8_t tile_index;
    /* OAM Attributes
     * Bit7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
     * Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
     * Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
     * Bit4   Palette number  **Non CGB Mode Only** (0=OBP0 (FF48), 1=OBP1 (FF49)) 
     * Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
     * Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7) */
    uint8_t attributes;
};
#pragma pack(pop)


#endif
