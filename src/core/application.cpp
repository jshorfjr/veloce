#include "application.hpp"
#include "window_manager.hpp"
#include "renderer.hpp"
#include "input_manager.hpp"
#include "audio_manager.hpp"
#include "plugin_manager.hpp"
#include "speedrun_manager.hpp"
#include "savestate_manager.hpp"
#include "gui/gui_manager.hpp"
#include "emu/controller_layout.hpp"

#include <SDL.h>
#include <iostream>
#include <fstream>
#include <cstring>

namespace emu {

// Global application instance
static Application* g_application = nullptr;

Application& get_application() {
    return *g_application;
}

Application::Application() {
    g_application = this;
}

Application::~Application() {
    if (g_application == this) {
        g_application = nullptr;
    }
}

void Application::print_usage(const char* program_name) {
    std::cout << "Veloce - A plugin-based emulator framework for speedrunners\n\n";
    std::cout << "Usage: " << program_name << " [OPTIONS] [ROM_FILE]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help       Show this help message and exit\n";
    std::cout << "  -v, --version    Show version information and exit\n";
    std::cout << "  -d, --debug      Enable debug mode (show CPU/PPU state)\n";
    std::cout << "\n";
    std::cout << "ROM_FILE:\n";
    std::cout << "  Optional path to a ROM file to load on startup.\n";
    std::cout << "  Supported formats: .nes (NES)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " game.nes         # Load and run a NES ROM\n";
    std::cout << "  " << program_name << " --debug game.nes # Load with debug mode\n";
    std::cout << "  " << program_name << "                  # Start without loading a ROM\n";
}

void Application::print_version() {
    std::cout << "Veloce v0.1.0\n";
    std::cout << "Built for speedrunners with cycle-accurate emulation.\n";
    std::cout << "Supported systems: NES\n";
}

bool Application::parse_command_line(int argc, char* argv[], std::string& rom_path) {
    rom_path.clear();

    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return false;  // Signal to exit
        }
        else if (std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--version") == 0) {
            print_version();
            return false;  // Signal to exit
        }
        else if (std::strcmp(arg, "-d") == 0 || std::strcmp(arg, "--debug") == 0) {
            m_debug_mode = true;
            std::cout << "Debug mode enabled\n";
        }
        else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information.\n";
            return false;
        }
        else {
            // Assume it's a ROM path
            rom_path = arg;
        }
    }

    return true;
}

bool Application::initialize(int argc, char* argv[]) {
    // Parse command line arguments
    std::string rom_path;
    if (!parse_command_line(argc, argv, rom_path)) {
        m_running = false;
        return true;  // Not an error, just exit gracefully
    }
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create subsystems
    m_window_manager = std::make_unique<WindowManager>();
    m_renderer = std::make_unique<Renderer>();
    m_input_manager = std::make_unique<InputManager>();
    m_audio_manager = std::make_unique<AudioManager>();
    m_plugin_manager = std::make_unique<PluginManager>();
    m_gui_manager = std::make_unique<GuiManager>();
    m_speedrun_manager = std::make_unique<SpeedrunManager>();
    m_savestate_manager = std::make_unique<SavestateManager>();

    // Initialize window
    WindowConfig window_config;
    window_config.title = "Veloce";
    window_config.width = 1024;
    window_config.height = 768;

    if (!m_window_manager->initialize(window_config)) {
        std::cerr << "Failed to initialize window manager" << std::endl;
        return false;
    }

    // Initialize renderer
    if (!m_renderer->initialize(*m_window_manager)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    // Initialize input
    if (!m_input_manager->initialize()) {
        std::cerr << "Failed to initialize input manager" << std::endl;
        return false;
    }

    // Initialize audio
    if (!m_audio_manager->initialize()) {
        std::cerr << "Failed to initialize audio manager" << std::endl;
        return false;
    }

    // Initialize plugin manager
    if (!m_plugin_manager->initialize()) {
        std::cerr << "Failed to initialize plugin manager" << std::endl;
        return false;
    }

    // Initialize GUI
    if (!m_gui_manager->initialize(*m_window_manager)) {
        std::cerr << "Failed to initialize GUI manager" << std::endl;
        return false;
    }

    // Initialize speedrun manager
    m_speedrun_manager->initialize(m_plugin_manager.get());

    // Initialize savestate manager
    m_savestate_manager->initialize(m_plugin_manager.get());

    // Load ROM if provided on command line
    if (!rom_path.empty()) {
        load_rom(rom_path);
    }

    m_running = true;
    m_last_frame_time = WindowManager::get_ticks();

    std::cout << "Veloce initialized successfully" << std::endl;
    return true;
}

void Application::run() {
    const double target_frame_time = 1.0 / 60.0988;  // NES NTSC frame rate

    while (m_running && !m_quit_requested) {
        uint64_t frame_start = WindowManager::get_ticks();
        double frequency = static_cast<double>(WindowManager::get_performance_frequency());

        // Process events
        process_events();

        // Update input
        m_input_manager->update();

        // Run emulation if not paused
        if (!m_paused || m_frame_advance_requested) {
            run_emulation_frame();
            m_frame_advance_requested = false;
        }

        // Render
        render();

        // Frame timing
        uint64_t frame_end = WindowManager::get_ticks();
        double frame_time = static_cast<double>(frame_end - frame_start) / frequency;

        // Sleep to maintain target frame rate
        if (m_speed_multiplier == 1.0f && frame_time < target_frame_time) {
            double sleep_time = (target_frame_time - frame_time) * 1000.0;
            SDL_Delay(static_cast<uint32_t>(sleep_time));
        }
    }
}

void Application::shutdown() {
    // Save input config before shutdown
    m_input_manager->save_platform_config(m_input_manager->get_current_platform());

    m_gui_manager->shutdown();
    m_plugin_manager->shutdown();
    m_audio_manager->shutdown();
    m_input_manager->shutdown();
    m_renderer->shutdown();
    m_window_manager->shutdown();

    SDL_Quit();

    std::cout << "Veloce shutdown complete" << std::endl;
}

void Application::process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Let GUI handle events first
        m_gui_manager->process_event(event);

        // Process input events
        m_input_manager->process_event(event);

        switch (event.type) {
            case SDL_QUIT:
                m_quit_requested = true;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    m_quit_requested = true;
                }
                break;

            case SDL_KEYDOWN:
                // Handle hotkeys (only if GUI doesn't want keyboard)
                if (!m_gui_manager->wants_keyboard()) {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            toggle_pause();
                            break;
                        case SDLK_r:
                            if (event.key.keysym.mod & KMOD_CTRL) {
                                reset();
                            }
                            break;
                        case SDLK_f:
                            if (!(event.key.keysym.mod & KMOD_CTRL)) {
                                frame_advance();
                            }
                            break;
                        case SDLK_F11:
                            m_window_manager->toggle_fullscreen();
                            break;
                        case SDLK_F12:
                            toggle_debug_mode();
                            std::cout << "Debug mode: " << (m_debug_mode ? "ON" : "OFF") << std::endl;
                            break;

                        // Savestate hotkeys: F1-F10 to save, Shift+F1-F10 to load
                        case SDLK_F1:
                        case SDLK_F2:
                        case SDLK_F3:
                        case SDLK_F4:
                        case SDLK_F5:
                        case SDLK_F6:
                        case SDLK_F7:
                        case SDLK_F8:
                        case SDLK_F9:
                        case SDLK_F10: {
                            int slot = event.key.keysym.sym - SDLK_F1;  // 0-9
                            if (event.key.keysym.mod & KMOD_SHIFT) {
                                // Load state
                                if (m_savestate_manager->load_state(slot)) {
                                    std::cout << "Loaded state from slot " << (slot + 1) << std::endl;
                                } else {
                                    std::cout << "Failed to load state from slot " << (slot + 1) << std::endl;
                                }
                            } else {
                                // Save state
                                if (m_savestate_manager->save_state(slot)) {
                                    std::cout << "Saved state to slot " << (slot + 1) << std::endl;
                                } else {
                                    std::cout << "Failed to save state to slot " << (slot + 1) << std::endl;
                                }
                            }
                            break;
                        }
                    }
                }
                break;

            case SDL_DROPFILE:
                load_rom(event.drop.file);
                SDL_free(event.drop.file);
                break;
        }
    }
}

void Application::run_emulation_frame() {
    auto* plugin = m_plugin_manager->get_active_plugin();
    if (!plugin || !plugin->is_rom_loaded()) {
        return;
    }

    // Get input state
    InputState input;
    input.buttons = m_input_manager->get_button_state();

    // Run one frame
    plugin->run_frame(input);

    // Update speedrun manager (for auto-split detection)
    m_speedrun_manager->update();

    // Get audio samples
    AudioBuffer audio = plugin->get_audio();
    if (audio.samples && audio.sample_count > 0) {
        m_audio_manager->push_samples(audio.samples, audio.sample_count * 2);  // Stereo
        plugin->clear_audio_buffer();
    }

    // Update texture with framebuffer
    FrameBuffer fb = plugin->get_framebuffer();
    if (fb.pixels) {
        m_renderer->update_texture(fb.pixels, fb.width, fb.height);
    }
}

void Application::render() {
    m_renderer->clear();
    m_gui_manager->begin_frame();
    m_gui_manager->render(*this, *m_renderer);
    m_gui_manager->end_frame();
    m_window_manager->swap_buffers();
}

bool Application::load_rom(const std::string& path) {
    std::cout << "Loading ROM: " << path << std::endl;

    // Read file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return false;
    }

    // Find appropriate plugin
    if (!m_plugin_manager->set_active_plugin_for_file(path)) {
        std::cerr << "No plugin found for file: " << path << std::endl;
        return false;
    }

    // Load ROM
    if (!m_plugin_manager->load_rom(data.data(), data.size())) {
        std::cerr << "Failed to load ROM" << std::endl;
        return false;
    }

    // Get controller layout from emulator plugin and pass to input manager
    auto* plugin = m_plugin_manager->get_active_plugin();
    if (plugin) {
        // Get controller layout from the emulator plugin
        // The plugin is responsible for defining its own layout
        const ControllerLayoutInfo* layout = plugin->get_controller_layout();

        // Set the controller layout (this also updates the platform)
        m_input_manager->set_controller_layout(layout);
    }

    // Update window title
    m_window_manager->set_title("Veloce - " + path);

    // Unpause and start emulation
    m_paused = false;
    m_audio_manager->resume();

    std::cout << "ROM loaded successfully" << std::endl;
    return true;
}

void Application::pause() {
    m_paused = true;
    m_audio_manager->pause();
}

void Application::resume() {
    m_paused = false;
    m_audio_manager->resume();
}

void Application::reset() {
    auto* plugin = m_plugin_manager->get_active_plugin();
    if (plugin && plugin->is_rom_loaded()) {
        plugin->reset();
        m_audio_manager->clear_buffer();
    }
}

void Application::toggle_pause() {
    if (m_paused) {
        resume();
    } else {
        pause();
    }
}

void Application::frame_advance() {
    if (m_paused) {
        m_frame_advance_requested = true;
    }
}

} // namespace emu
