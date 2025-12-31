#pragma once

#include <cstdint>
#include <vector>

namespace nes {

// Mirror modes for nametables
enum class MirrorMode {
    Horizontal,
    Vertical,
    SingleScreen0,
    SingleScreen1,
    FourScreen
};

// Base class for cartridge mappers
class Mapper {
public:
    virtual ~Mapper() = default;

    // CPU memory access ($4020-$FFFF)
    virtual uint8_t cpu_read(uint16_t address) = 0;
    virtual void cpu_write(uint16_t address, uint8_t value) = 0;

    // PPU memory access ($0000-$1FFF)
    virtual uint8_t ppu_read(uint16_t address) = 0;
    virtual void ppu_write(uint16_t address, uint8_t value) = 0;

    // Get current mirror mode
    virtual MirrorMode get_mirror_mode() const = 0;

    // IRQ support (some mappers generate IRQs)
    virtual bool irq_pending() { return false; }
    virtual void irq_clear() {}

    // Scanline counter (for MMC3 and similar)
    virtual void scanline() {}

    // Reset mapper state
    virtual void reset() {}

    // Save state
    virtual void save_state(std::vector<uint8_t>& data) {}
    virtual void load_state(const uint8_t*& data, size_t& remaining) {}

protected:
    std::vector<uint8_t>* m_prg_rom = nullptr;
    std::vector<uint8_t>* m_chr_rom = nullptr;
    std::vector<uint8_t>* m_prg_ram = nullptr;
    MirrorMode m_mirror_mode = MirrorMode::Horizontal;
    bool m_has_chr_ram = false;
};

// Factory function to create mapper by number
Mapper* create_mapper(int mapper_number,
                      std::vector<uint8_t>& prg_rom,
                      std::vector<uint8_t>& chr_rom,
                      std::vector<uint8_t>& prg_ram,
                      MirrorMode initial_mirror,
                      bool has_chr_ram);

} // namespace nes
