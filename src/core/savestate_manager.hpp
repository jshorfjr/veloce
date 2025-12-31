#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace emu {

class PluginManager;

// Metadata stored with each savestate
struct SavestateInfo {
    std::string rom_name;           // Name of the ROM
    uint32_t rom_crc32;             // ROM checksum for validation
    uint64_t frame_count;           // Frame when saved
    int64_t timestamp;              // Unix timestamp when saved
    bool valid;                     // Whether this slot has a valid savestate
};

class SavestateManager {
public:
    static constexpr int NUM_SLOTS = 10;  // Slots 0-9 (F1-F10 hotkeys)

    SavestateManager();
    ~SavestateManager();

    // Initialize with plugin manager reference
    void initialize(PluginManager* plugin_manager);

    // Save current state to slot (0-9)
    bool save_state(int slot);

    // Load state from slot (0-9)
    bool load_state(int slot);

    // Quick save/load (uses slot 0)
    bool quick_save();
    bool quick_load();

    // Get info about a slot
    SavestateInfo get_slot_info(int slot) const;

    // Check if slot has a valid savestate
    bool is_slot_valid(int slot) const;

    // Get/set save directory
    void set_save_directory(const std::string& dir);
    const std::string& get_save_directory() const { return m_save_directory; }

    // Get current ROM's save path
    std::string get_savestate_path(int slot) const;

private:
    bool write_savestate_file(const std::string& path, const std::vector<uint8_t>& data,
                              const SavestateInfo& info);
    std::optional<std::vector<uint8_t>> read_savestate_file(const std::string& path,
                                                             SavestateInfo& info);

    PluginManager* m_plugin_manager = nullptr;
    std::string m_save_directory = "savestates";
    std::string m_current_rom_name;
};

} // namespace emu
