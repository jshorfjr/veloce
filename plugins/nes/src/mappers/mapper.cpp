#include "mapper.hpp"
#include "mapper_000.hpp"
#include "mapper_001.hpp"
#include "mapper_002.hpp"
#include "mapper_003.hpp"
#include "mapper_004.hpp"
#include "mapper_007.hpp"
#include "mapper_009.hpp"
#include "mapper_010.hpp"
#include "mapper_011.hpp"
#include "mapper_034.hpp"
#include "mapper_066.hpp"
#include "mapper_071.hpp"
#include "mapper_079.hpp"
#include "mapper_206.hpp"
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
        case 2:
            return new Mapper002(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 3:
            return new Mapper003(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 4:
            return new Mapper004(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 7:
            return new Mapper007(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 9:
            return new Mapper009(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 10:
            return new Mapper010(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 11:
            return new Mapper011(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 34:
            return new Mapper034(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 66:
            return new Mapper066(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 71:
            return new Mapper071(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 79:
            return new Mapper079(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        case 206:
            return new Mapper206(prg_rom, chr_rom, prg_ram, initial_mirror, has_chr_ram);
        default:
            std::cerr << "Unsupported mapper: " << mapper_number << std::endl;
            return nullptr;
    }
}

} // namespace nes
