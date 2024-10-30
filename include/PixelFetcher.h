#ifndef _PIXEL_FETCHER_H_
#define _PIXEL_FETCHER_H_

#include <cstdint>

/*
 * each mode takes 2 cycles
 * except push which takes continues until it succeeds
 */
enum class PixelFetcherMode {
    GET_TILE,
    GET_TILE_LOW,
    GET_TILE_HIGH,
    SLEEP,
    PUSH,
};

struct PixelFetcher {
    uint16_t fetcher_x;
    uint16_t fetcher_y;
    uint16_t drawn_x;
    uint16_t drawn_y;
    bool fetching_window;
    uint16_t tile_map_addr;
    uint16_t tile_data_addr;
};


#endif