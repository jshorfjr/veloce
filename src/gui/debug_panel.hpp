#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace emu {

class Application;
class IEmulatorPlugin;

// Debug panel for showing emulator state
class DebugPanel {
public:
    DebugPanel();
    ~DebugPanel();

    // Render the panel
    void render(Application& app, bool& visible);

private:
    void render_cpu_state(IEmulatorPlugin* plugin);
    void render_memory_viewer(IEmulatorPlugin* plugin);
    void render_ppu_state(IEmulatorPlugin* plugin);
    void render_timing_info(IEmulatorPlugin* plugin);

    // Memory viewer state
    uint16_t m_memory_start_address = 0x0000;
    int m_memory_columns = 16;
    bool m_show_ascii = true;

    // Watch list for specific addresses
    struct WatchEntry {
        uint16_t address;
        std::string label;
        bool show_hex;
    };
    std::vector<WatchEntry> m_watches;
};

} // namespace emu
