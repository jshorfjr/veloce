#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "cartridge.hpp"
#include "mappers/mapper.hpp"

#include <cstring>

namespace nes {

Bus::Bus() {
    m_ram.fill(0);
}

Bus::~Bus() = default;

uint8_t Bus::cpu_read(uint16_t address) {
    if (address < 0x2000) {
        // Internal RAM (mirrored)
        return m_ram[address & 0x07FF];
    }
    else if (address < 0x4000) {
        // PPU registers (mirrored every 8 bytes)
        return m_ppu ? m_ppu->cpu_read(address & 0x0007) : 0;
    }
    else if (address < 0x4020) {
        // APU and I/O registers
        if (address == 0x4016) {
            return read_controller(0);
        }
        else if (address == 0x4017) {
            return read_controller(1);
        }
        else if (m_apu) {
            return m_apu->cpu_read(address);
        }
        return 0;
    }
    else {
        // Cartridge space
        return m_cartridge ? m_cartridge->cpu_read(address) : 0;
    }
}

void Bus::cpu_write(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        // Internal RAM (mirrored)
        m_ram[address & 0x07FF] = value;
    }
    else if (address < 0x4000) {
        // PPU registers (mirrored every 8 bytes)
        if (m_ppu) {
            m_ppu->cpu_write(address & 0x0007, value);
        }
    }
    else if (address < 0x4020) {
        // APU and I/O registers
        if (address == 0x4014) {
            // OAM DMA
            oam_dma(value);
        }
        else if (address == 0x4016) {
            // Controller strobe
            m_controller_strobe = (value & 1) != 0;
            if (m_controller_strobe) {
                m_controller_shift[0] = static_cast<uint8_t>(m_controller_state[0]);
                m_controller_shift[1] = static_cast<uint8_t>(m_controller_state[1]);
            }
        }
        else if (m_apu) {
            m_apu->cpu_write(address, value);
        }
    }
    else {
        // Cartridge space
        if (m_cartridge) {
            m_cartridge->cpu_write(address, value);
        }
    }
}

uint8_t Bus::ppu_read(uint16_t address) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        // Pattern tables (CHR ROM/RAM)
        return m_cartridge ? m_cartridge->ppu_read(address) : 0;
    }
    else {
        // Nametables and palettes handled by PPU
        return m_ppu ? m_ppu->ppu_read(address) : 0;
    }
}

void Bus::ppu_write(uint16_t address, uint8_t value) {
    address &= 0x3FFF;

    if (address < 0x2000) {
        // Pattern tables (CHR RAM)
        if (m_cartridge) {
            m_cartridge->ppu_write(address, value);
        }
    }
    else {
        // Nametables and palettes handled by PPU
        if (m_ppu) {
            m_ppu->ppu_write(address, value);
        }
    }
}

void Bus::set_controller_state(int controller, uint32_t buttons) {
    if (controller >= 0 && controller < 2) {
        // Map from VirtualButton format to NES format
        // VirtualButton: A=0x001, B=0x002, X=0x004, Y=0x008, L=0x010, R=0x020,
        //                Start=0x040, Select=0x080, Up=0x100, Down=0x200, Left=0x400, Right=0x800
        // NES order: A, B, Select, Start, Up, Down, Left, Right
        uint8_t nes_buttons = 0;
        if (buttons & 0x001) nes_buttons |= 0x01;  // A
        if (buttons & 0x002) nes_buttons |= 0x02;  // B
        if (buttons & 0x080) nes_buttons |= 0x04;  // Select (VirtualButton = 0x080)
        if (buttons & 0x040) nes_buttons |= 0x08;  // Start (VirtualButton = 0x040)
        if (buttons & 0x100) nes_buttons |= 0x10;  // Up (VirtualButton = 0x100)
        if (buttons & 0x200) nes_buttons |= 0x20;  // Down (VirtualButton = 0x200)
        if (buttons & 0x400) nes_buttons |= 0x40;  // Left (VirtualButton = 0x400)
        if (buttons & 0x800) nes_buttons |= 0x80;  // Right (VirtualButton = 0x800)

        m_controller_state[controller] = nes_buttons;
    }
}

uint8_t Bus::read_controller(int controller) {
    if (controller < 0 || controller >= 2) return 0;

    uint8_t data = m_controller_shift[controller] & 1;
    m_controller_shift[controller] >>= 1;
    m_controller_shift[controller] |= 0x80;  // Fill with 1s

    return data | 0x40;  // Open bus bits
}

void Bus::oam_dma(uint8_t page) {
    uint16_t addr = static_cast<uint16_t>(page) << 8;
    for (int i = 0; i < 256; i++) {
        uint8_t data = cpu_read(addr + i);
        if (m_ppu) {
            m_ppu->oam_write(i, data);
        }
    }
    // DMA takes 513 or 514 CPU cycles (514 if on an odd CPU cycle)
    // We use 513 as an approximation
    m_pending_dma_cycles = 513;
}

int Bus::get_pending_dma_cycles() {
    int cycles = m_pending_dma_cycles;
    m_pending_dma_cycles = 0;
    return cycles;
}

void Bus::mapper_scanline() {
    if (m_cartridge) {
        m_cartridge->scanline();
    }
}

bool Bus::mapper_irq_pending() {
    if (m_cartridge) {
        return m_cartridge->irq_pending();
    }
    return false;
}

void Bus::mapper_irq_clear() {
    if (m_cartridge) {
        m_cartridge->irq_clear();
    }
}

int Bus::get_mirror_mode() const {
    if (m_cartridge) {
        // Return the actual mirror mode value:
        // 0=Horizontal, 1=Vertical, 2=SingleScreen0, 3=SingleScreen1, 4=FourScreen
        return static_cast<int>(m_cartridge->get_mirror_mode());
    }
    return 0;  // Default to horizontal
}

// Save state serialization
namespace {
    template<typename T>
    void write_value(std::vector<uint8_t>& data, T value) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), ptr, ptr + sizeof(T));
    }

    template<typename T>
    bool read_value(const uint8_t*& data, size_t& remaining, T& value) {
        if (remaining < sizeof(T)) return false;
        std::memcpy(&value, data, sizeof(T));
        data += sizeof(T);
        remaining -= sizeof(T);
        return true;
    }

    void write_array(std::vector<uint8_t>& data, const uint8_t* arr, size_t size) {
        data.insert(data.end(), arr, arr + size);
    }

    bool read_array(const uint8_t*& data, size_t& remaining, uint8_t* arr, size_t size) {
        if (remaining < size) return false;
        std::memcpy(arr, data, size);
        data += size;
        remaining -= size;
        return true;
    }
}

void Bus::save_state(std::vector<uint8_t>& data) {
    // Save RAM
    write_array(data, m_ram.data(), m_ram.size());

    // Save controller state
    write_value(data, m_controller_state[0]);
    write_value(data, m_controller_state[1]);
    write_value(data, m_controller_shift[0]);
    write_value(data, m_controller_shift[1]);
    write_value(data, static_cast<uint8_t>(m_controller_strobe ? 1 : 0));
}

void Bus::load_state(const uint8_t*& data, size_t& remaining) {
    // Load RAM
    read_array(data, remaining, m_ram.data(), m_ram.size());

    // Load controller state
    read_value(data, remaining, m_controller_state[0]);
    read_value(data, remaining, m_controller_state[1]);
    read_value(data, remaining, m_controller_shift[0]);
    read_value(data, remaining, m_controller_shift[1]);
    uint8_t strobe;
    read_value(data, remaining, strobe);
    m_controller_strobe = strobe != 0;
}

} // namespace nes
