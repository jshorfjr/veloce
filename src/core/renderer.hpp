#pragma once

#include <cstdint>

namespace emu {

class WindowManager;

// Handles OpenGL rendering of the game framebuffer
class Renderer {
public:
    Renderer();
    ~Renderer();

    // Disable copy
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Initialize OpenGL resources
    bool initialize(WindowManager& window_manager);

    // Shutdown and cleanup
    void shutdown();

    // Upload framebuffer data to GPU texture
    void update_texture(const uint32_t* pixels, int width, int height);

    // Render the game texture (called by GUI)
    void render_game_texture(int x, int y, int width, int height);

    // Clear the screen
    void clear(float r = 0.1f, float g = 0.1f, float b = 0.1f);

    // Get texture ID for ImGui rendering
    uint32_t get_texture_id() const { return m_texture_id; }
    int get_texture_width() const { return m_texture_width; }
    int get_texture_height() const { return m_texture_height; }

private:
    bool create_texture(int width, int height);
    bool compile_shaders();

    WindowManager* m_window_manager = nullptr;
    uint32_t m_texture_id = 0;
    int m_texture_width = 0;
    int m_texture_height = 0;
    uint32_t m_shader_program = 0;
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
};

} // namespace emu
