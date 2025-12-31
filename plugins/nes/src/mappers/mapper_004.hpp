#pragma once

#include "mapper.hpp"

namespace nes {

// Mapper 4: MMC3 (Nintendo MMC3)
// Used by: Super Mario Bros. 3, Mega Man 3-6, many others
// - PRG ROM: Up to 512KB (8KB switchable banks)
// - CHR ROM/RAM: Up to 256KB (1KB/2KB switchable banks)
// - Scanline counter for IRQ
// - Switchable mirroring
class Mapper004 : public Mapper {
public:
    Mapper004(std::vector<uint8_t>& prg_rom,
              std::vector<uint8_t>& chr_rom,
              std::vector<uint8_t>& prg_ram,
              MirrorMode mirror,
              bool has_chr_ram);

    uint8_t cpu_read(uint16_t address) override;
    void cpu_write(uint16_t address, uint8_t value) override;

    uint8_t ppu_read(uint16_t address) override;
    void ppu_write(uint16_t address, uint8_t value) override;

    MirrorMode get_mirror_mode() const override { return m_mirror_mode; }

    bool irq_pending() override { return m_irq_pending; }
    void irq_clear() override { m_irq_pending = false; }
    void scanline() override;

    void reset() override;
    void save_state(std::vector<uint8_t>& data) override;
    void load_state(const uint8_t*& data, size_t& remaining) override;

private:
    void update_banks();

    // Bank select register
    uint8_t m_bank_select = 0;
    bool m_prg_mode = false;    // PRG ROM bank mode
    bool m_chr_mode = false;    // CHR ROM bank mode

    // Bank registers R0-R7
    uint8_t m_registers[8] = {0};

    // PRG bank offsets (in bytes)
    uint32_t m_prg_bank[4] = {0};

    // CHR bank offsets (in bytes)
    uint32_t m_chr_bank[8] = {0};

    // IRQ counter
    uint8_t m_irq_counter = 0;
    uint8_t m_irq_latch = 0;
    bool m_irq_enabled = false;
    bool m_irq_pending = false;
    bool m_irq_reload = false;

    // A12 tracking for scanline detection
    bool m_last_a12 = false;
    int m_a12_low_cycles = 0;
};

} // namespace nes
