#pragma once

#include <string>
#include <cstdint>

struct SDL_Window;
typedef void* SDL_GLContext;

namespace emu {

struct WindowConfig {
    std::string title = "Veloce";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool vsync = true;
    int scale = 2;
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    // Disable copy
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    // Initialize SDL and create window
    bool initialize(const WindowConfig& config = WindowConfig{});

    // Shutdown and cleanup
    void shutdown();

    // Window management
    void set_title(const std::string& title);
    void set_size(int width, int height);
    void toggle_fullscreen();
    void set_vsync(bool enabled);

    // Getters
    SDL_Window* get_window() const { return m_window; }
    SDL_GLContext get_gl_context() const { return m_gl_context; }
    int get_width() const { return m_width; }
    int get_height() const { return m_height; }
    bool is_fullscreen() const { return m_fullscreen; }

    // Frame management
    void swap_buffers();

    // Get high-resolution timer
    static uint64_t get_ticks();
    static uint64_t get_performance_frequency();

private:
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;
    int m_width = 800;
    int m_height = 600;
    bool m_fullscreen = false;
    bool m_vsync = true;
};

} // namespace emu
