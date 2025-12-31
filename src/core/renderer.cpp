#include "renderer.hpp"
#include "window_manager.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <cstring>

namespace emu {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(WindowManager& window_manager) {
    m_window_manager = &window_manager;

    // Create initial texture for game display
    if (!create_texture(256, 240)) {  // NES default resolution
        return false;
    }

    std::cout << "Renderer initialized" << std::endl;
    return true;
}

void Renderer::shutdown() {
    if (m_texture_id) {
        glDeleteTextures(1, &m_texture_id);
        m_texture_id = 0;
    }
    // VAO, VBO, and shaders are not used - ImGui handles all rendering
    m_vao = 0;
    m_vbo = 0;
    m_shader_program = 0;
}

bool Renderer::create_texture(int width, int height) {
    // Delete old texture if exists
    if (m_texture_id) {
        glDeleteTextures(1, &m_texture_id);
    }

    // Generate new texture
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // Set texture parameters for pixel-perfect rendering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    m_texture_width = width;
    m_texture_height = height;

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Renderer::update_texture(const uint32_t* pixels, int width, int height) {
    if (!pixels) return;

    // Recreate texture if size changed
    if (width != m_texture_width || height != m_texture_height) {
        create_texture(width, height);
    }

    // Upload pixel data
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::render_game_texture(int x, int y, int width, int height) {
    // This is handled by ImGui now - we just provide the texture
    // The actual rendering is done in game_view.cpp using ImGui::Image
}

void Renderer::clear(float r, float g, float b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace emu
