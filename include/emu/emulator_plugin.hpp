#pragma once

#include "controller_layout.hpp"
#include <cstdint>
#include <cstddef>
#include <vector>

// DLL export macro for Windows
#ifdef _WIN32
    #define EMU_PLUGIN_EXPORT __declspec(dllexport)
#else
    #define EMU_PLUGIN_EXPORT
#endif

namespace emu {

// Information about the emulator plugin
struct EmulatorInfo {
    const char* name;               // "NES", "SNES", etc.
    const char* version;            // "1.0.0"
    const char** file_extensions;   // {".nes", nullptr}
    double native_fps;              // 60.0988 for NES (NTSC)
    uint64_t cycles_per_second;     // 1789773 for NES CPU
    int screen_width;               // 256 for NES
    int screen_height;              // 240 for NES
};

// Framebuffer for video output
struct FrameBuffer {
    uint32_t* pixels;   // RGBA8888 format
    int width;
    int height;
};

// Audio buffer for sound output
struct AudioBuffer {
    float* samples;     // Interleaved stereo samples (-1.0 to 1.0)
    int sample_count;   // Number of sample pairs
    int sample_rate;    // Typically 44100 or 48000
};

// Input state for controllers
// Button bitmask uses VirtualButton ordering from input_types.hpp
struct InputState {
    uint32_t buttons;   // Bitmask of pressed buttons
};

// Main emulator plugin interface
class IEmulatorPlugin {
public:
    virtual ~IEmulatorPlugin() = default;

    // Plugin information
    virtual EmulatorInfo get_info() = 0;

    // Controller layout for this platform
    // Returns the visual layout for input configuration UI
    // Default implementation returns nullptr (use default layout based on platform name)
    virtual const ControllerLayoutInfo* get_controller_layout() { return nullptr; }

    // ROM loading and management
    virtual bool load_rom(const uint8_t* data, size_t size) = 0;
    virtual void unload_rom() = 0;
    virtual bool is_rom_loaded() const = 0;
    virtual uint32_t get_rom_crc32() const = 0;

    // Emulation control
    virtual void reset() = 0;
    virtual void run_frame(const InputState& input) = 0;
    virtual uint64_t get_cycle_count() const = 0;
    virtual uint64_t get_frame_count() const = 0;

    // Video output
    virtual FrameBuffer get_framebuffer() = 0;

    // Audio output
    virtual AudioBuffer get_audio() = 0;
    virtual void clear_audio_buffer() = 0;

    // Memory access (for speedrun plugins and RAM watch)
    virtual uint8_t read_memory(uint16_t address) = 0;
    virtual void write_memory(uint16_t address, uint8_t value) = 0;

    // Save states
    virtual bool save_state(std::vector<uint8_t>& data) = 0;
    virtual bool load_state(const std::vector<uint8_t>& data) = 0;
};

} // namespace emu

// C interface for dynamic loading
extern "C" {
    // Create a new plugin instance
    EMU_PLUGIN_EXPORT emu::IEmulatorPlugin* create_emulator_plugin();

    // Destroy plugin instance
    EMU_PLUGIN_EXPORT void destroy_emulator_plugin(emu::IEmulatorPlugin* plugin);

    // Get plugin API version (for compatibility checking)
    EMU_PLUGIN_EXPORT uint32_t get_plugin_api_version();
}

// Current API version
#define EMU_PLUGIN_API_VERSION 1
