#include "mapper.hpp"
#include "mapper_000.hpp"
#include "mapper_001.hpp"
#include "mapper_004.hpp"
#include <iostream>

namespace nes {

Mapper* create_mapper(int mapper_number,
                      std::vector<uint8_t>& prg_rom,
                      std::vector<uint8_t>& chr_rom,
                      std::vector<uint8_t>& prg_ram,
                      MirrorMode initial_mirror,
                      bool has_chr_ram)
{
    switch (mapper_number) {
        case 0:
            return new Mapper000(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 1:
            return new Mapper001(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 4:
            return new Mapper004(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        default:
            std::cerr << "Unsupported mapper: " << mapper_number << std::endl;
            return nullptr;
    }
}

} // namespace nes
