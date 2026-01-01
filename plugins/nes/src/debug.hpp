#pragma once

#include <cstdlib>

namespace nes {

// Single debug mode check - caches result of DEBUG environment variable
inline bool is_debug_mode() {
    static bool checked = false;
    static bool debug = false;
    if (!checked) {
        const char* env = std::getenv("DEBUG");
        debug = (env != nullptr && env[0] != '0');
        checked = true;
    }
    return debug;
}

} // namespace nes
