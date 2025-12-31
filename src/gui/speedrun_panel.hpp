#pragma once

#include <string>
#include <cstdint>

namespace emu {

class Application;
class SpeedrunManager;

// Speedrun timer and splits panel
class SpeedrunPanel {
public:
    SpeedrunPanel();
    ~SpeedrunPanel();

    // Render the panel, pass visibility flag by ref for close button
    void render(SpeedrunManager& manager, bool& visible);

private:
    void render_timer(SpeedrunManager& manager);
    void render_splits(SpeedrunManager& manager);
    void render_controls(SpeedrunManager& manager);

    std::string format_time(uint64_t ms, bool show_ms = true) const;
    std::string format_delta(int64_t ms) const;
};

} // namespace emu
