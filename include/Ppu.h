#ifndef PPU_H
#define PPU_H

#include <memory>
#include <vector>

#include "common.h"
#include "InterruptObserver.h"
#include "Lcd.h"
#include "Oam.h"

class MemoryBus;

constexpr int SCAN_LINE_CYCLES = 456;
constexpr int OAM_CYCLES = 80;
constexpr int VBLANK_LINES = 10;
constexpr int PIXEL_TRANSFER_CYCLES = 172;
constexpr int HBLANK_CYCLES = 204;

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
    int ppu_mode_hblank(int cycles);
    int ppu_mode_vblank(int cycles);
    int ppu_mode_data_xfer(int cycles);
    int ppu_mode_oam_search(int cycles);
    void pixel_fetcher_tick();

    bool m_frame_ready{false};
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
    // TODO see if we need this
    std::vector<OamAttribute> m_oam_fifo{};

    std::weak_ptr<MemoryBus> m_bus{};
    std::vector<uint32_t> m_frame_buffer{};

    // Interrupt observer so we can schedule interrupts
    std::shared_ptr<InterruptObserver> m_int_observer{};
};

#endif
