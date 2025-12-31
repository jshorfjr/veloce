#pragma once

#include <cstdint>
#include <vector>

// DLL export macro for Windows
#ifdef _WIN32
    #define EMU_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define EMU_PLUGIN_EXPORT
#endif

#define EMU_SPEEDRUN_PLUGIN_API_VERSION 1

namespace emu {

// Split trigger conditions
enum class SplitCondition {
    Equals,         // Trigger when value equals target
    NotEquals,      // Trigger when value does not equal target
    GreaterThan,    // Trigger when value > target
    LessThan,       // Trigger when value < target
    ChangesTo,      // Trigger when value changes to target
    ChangesFrom,    // Trigger when value changes from target
    Increases,      // Trigger when value increases
    Decreases       // Trigger when value decreases
};

// Definition of a single split
struct SplitDefinition {
    const char* name;           // Display name ("Enter 1-2", "Bowser Fight")
    uint16_t watch_address;     // Memory address to watch
    uint8_t trigger_value;      // Value for comparison
    SplitCondition condition;   // Trigger condition
};

// Information about a speedrun category
struct SpeedrunInfo {
    const char* game_name;      // "Super Mario Bros."
    const char* category;       // "Any%", "Warpless"
    const char* platform;       // "NES" - must match emulator plugin
    uint32_t game_crc32;        // ROM checksum for identification
};

// Callback interface provided by core to speedrun plugins
class ISpeedrunHost {
public:
    virtual ~ISpeedrunHost() = default;

    // Memory access
    virtual uint8_t read_memory(uint16_t address) = 0;

    // Timer control
    virtual void start_timer() = 0;
    virtual void stop_timer() = 0;
    virtual void reset_timer() = 0;
    virtual void split() = 0;
    virtual void undo_split() = 0;
    virtual void skip_split() = 0;

    // Timer state
    virtual bool is_timer_running() const = 0;
    virtual uint64_t get_current_time_ms() const = 0;
    virtual int get_current_split_index() const = 0;
};

// Speedrun plugin interface
class ISpeedrunPlugin {
public:
    virtual ~ISpeedrunPlugin() = default;

    // Get plugin info
    virtual SpeedrunInfo get_info() = 0;

    // Get split definitions
    virtual std::vector<SplitDefinition> get_splits() = 0;

    // Called each frame - plugin can check memory and trigger actions
    virtual void on_frame(ISpeedrunHost* host) = 0;

    // Called when ROM loads - return true if this plugin handles it
    virtual bool matches_rom(uint32_t crc32, const char* rom_name) = 0;

    // Optional: Called when timer is reset
    virtual void on_reset() {}

    // Optional: Called when run is completed
    virtual void on_run_complete(uint64_t final_time_ms) {}
};

// C interface for plugin loading
extern "C" {
    EMU_PLUGIN_EXPORT ISpeedrunPlugin* create_speedrun_plugin();
    EMU_PLUGIN_EXPORT void destroy_speedrun_plugin(ISpeedrunPlugin* plugin);
    EMU_PLUGIN_EXPORT uint32_t get_speedrun_plugin_api_version();
}

} // namespace emu
