#include "window_manager.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>

namespace emu {

WindowManager::WindowManager() = default;

WindowManager::~WindowManager() {
    shutdown();
}

bool WindowManager::initialize(const WindowConfig& config) {
    m_width = config.width;
    m_height = config.height;
    m_fullscreen = config.fullscreen;
    m_vsync = config.vsync;

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create window
    uint32_t window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (m_fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width,
        m_height,
        window_flags
    );

    if (!m_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create OpenGL context
    m_gl_context = SDL_GL_CreateContext(m_window);
    if (!m_gl_context) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        return false;
    }

    // Make context current
    SDL_GL_MakeCurrent(m_window, m_gl_context);

    // Set vsync
    SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);

    // Initialize OpenGL loader (using glad)
    // Note: In a real implementation, we'd use glad or similar
    // For now, we rely on SDL's built-in OpenGL loading

    std::cout << "Window created: " << m_width << "x" << m_height << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return true;
}

void WindowManager::shutdown() {
    if (m_gl_context) {
        SDL_GL_DeleteContext(m_gl_context);
        m_gl_context = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void WindowManager::set_title(const std::string& title) {
    if (m_window) {
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

void WindowManager::set_size(int width, int height) {
    if (m_window) {
        SDL_SetWindowSize(m_window, width, height);
        m_width = width;
        m_height = height;
    }
}

void WindowManager::toggle_fullscreen() {
    if (m_window) {
        m_fullscreen = !m_fullscreen;
        SDL_SetWindowFullscreen(m_window,
            m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

void WindowManager::set_vsync(bool enabled) {
    m_vsync = enabled;
    SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);
}

void WindowManager::swap_buffers() {
    if (m_window) {
        SDL_GL_SwapWindow(m_window);
    }
}

uint64_t WindowManager::get_ticks() {
    return SDL_GetPerformanceCounter();
}

uint64_t WindowManager::get_performance_frequency() {
    return SDL_GetPerformanceFrequency();
}

} // namespace emu
