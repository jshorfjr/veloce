#pragma once

#include <string>
#include <memory>

struct SDL_Window;
typedef void* SDL_GLContext;
union SDL_Event;

namespace emu {

class Application;
class WindowManager;
class Renderer;
class SpeedrunPanel;
class DebugPanel;
class InputConfigPanel;

class GuiManager {
public:
    GuiManager();
    ~GuiManager();

    // Disable copy
    GuiManager(const GuiManager&) = delete;
    GuiManager& operator=(const GuiManager&) = delete;

    // Initialize Dear ImGui
    bool initialize(WindowManager& window_manager);

    // Shutdown
    void shutdown();

    // Process SDL events
    void process_event(const SDL_Event& event);

    // Begin new frame
    void begin_frame();

    // Render all GUI elements
    void render(Application& app, Renderer& renderer);

    // End frame and render
    void end_frame();

    // GUI state
    bool wants_keyboard() const;
    bool wants_mouse() const;

    // Window visibility
    void show_rom_browser(bool show) { m_show_rom_browser = show; }
    void show_settings(bool show) { m_show_settings = show; }
    void show_ram_watch(bool show) { m_show_ram_watch = show; }
    void show_speedrun_panel(bool show) { m_show_speedrun_panel = show; }

private:
    void render_main_menu(Application& app);
    void render_game_view(Application& app, Renderer& renderer);
    void render_rom_browser(Application& app);
    void render_settings(Application& app);

    bool m_initialized = false;

    // Window visibility flags
    bool m_show_rom_browser = false;
    bool m_show_settings = false;
    bool m_show_ram_watch = false;
    bool m_show_speedrun_panel = true;  // Show by default
    bool m_show_debug_panel = false;
    bool m_show_demo_window = false;

    // ROM browser state
    std::string m_current_directory;

    // Panels
    std::unique_ptr<SpeedrunPanel> m_speedrun_panel;
    std::unique_ptr<DebugPanel> m_debug_panel;
    std::unique_ptr<InputConfigPanel> m_input_config_panel;
};

} // namespace emu
