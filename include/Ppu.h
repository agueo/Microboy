#ifndef PPU_H
#define PPU_H

#include <memory>
#include <vector>

#include "common.h"
#include "InterruptObserver.h"
#include "Lcd.h"
#include "Oam.h"

class MemoryBus;

class Ppu {
public:
    Ppu();
    ~Ppu() = default;

    bool step(int);
    void reset();

	uint8_t read_byte(uint16_t addr);
	uint16_t read_word(uint16_t addr);
	void write_byte(uint16_t addr, uint8_t value);
	void write_word(uint16_t addr, uint16_t value);

    void connect_interrupt_observer(std::shared_ptr<InterruptObserver> observer) { m_int_observer = observer; }
    void connect_bus(std::weak_ptr<MemoryBus> bus) { m_bus = bus; }
    uint32_t * get_frame_buffer() { return m_frame_buffer.data(); }

private:
    void request_dma_transfer(uint8_t addr);
    void ppu_switch_mode(LcdMode);
    void ppu_mode_hblank();
    void ppu_mode_vblank();
    void ppu_mode_data_xfer();
    void ppu_mode_oam_search();
    void pixel_fetcher_tick();

    bool frame_ready{false};
    uint16_t LX{0};
    uint16_t WLY{0};
    bool m_vram_blocked{false};
    bool m_oam_blocked{false};
    uint32_t m_dots{0};
    LcdMode m_mode{LcdMode::HBLANK};
    Lcd m_lcd{};

	std::vector<uint8_t> m_vram{};
	std::vector<uint8_t> m_oam{};

    std::vector<uint8_t> m_pixel_fifo{};
    std::vector<OamAttribute> m_oam_fifo{};

    std::weak_ptr<MemoryBus> m_bus{};
    std::vector<uint32_t> m_frame_buffer{};

    // Interrupt observer so we can schedule interrupts
    std::shared_ptr<InterruptObserver> m_int_observer{};
};

// TODO Pixel FIFO 
#endif
