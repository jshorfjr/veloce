#include "gui_manager.hpp"
#include "speedrun_panel.hpp"
#include "debug_panel.hpp"
#include "input_config_panel.hpp"
#include "core/application.hpp"
#include "core/window_manager.hpp"
#include "core/renderer.hpp"
#include "core/plugin_manager.hpp"
#include "core/audio_manager.hpp"
#include "core/input_manager.hpp"
#include "core/speedrun_manager.hpp"

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <filesystem>
#include <algorithm>

namespace emu {

GuiManager::GuiManager() = default;

GuiManager::~GuiManager() {
    shutdown();
}

bool GuiManager::initialize(WindowManager& window_manager) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window_manager.get_window(),
                                  window_manager.get_gl_context());
    ImGui_ImplOpenGL3_Init("#version 330");

    // Set current directory for ROM browser
    m_current_directory = std::filesystem::current_path().string();

    // Create GUI panels
    m_speedrun_panel = std::make_unique<SpeedrunPanel>();
    m_debug_panel = std::make_unique<DebugPanel>();
    m_input_config_panel = std::make_unique<InputConfigPanel>();

    m_initialized = true;
    std::cout << "GUI manager initialized" << std::endl;
    return true;
}

void GuiManager::shutdown() {
    if (m_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        m_initialized = false;
    }
}

void GuiManager::process_event(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void GuiManager::begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void GuiManager::render(Application& app, Renderer& renderer) {
    // Render main menu bar
    render_main_menu(app);

    // Render game view
    render_game_view(app, renderer);

    // Render optional windows
    if (m_show_rom_browser) {
        render_rom_browser(app);
    }

    // Clear input capture mode by default - only set when Input settings tab is active
    app.get_input_manager().set_input_capture_mode(false);

    if (m_show_settings) {
        render_settings(app);
    }

    // Render speedrun panel
    if (m_show_speedrun_panel && m_speedrun_panel) {
        m_speedrun_panel->render(app.get_speedrun_manager(), m_show_speedrun_panel);
    }

    // Render debug panel (also shown when debug mode is enabled)
    if ((m_show_debug_panel || app.is_debug_mode()) && m_debug_panel) {
        bool visible = m_show_debug_panel || app.is_debug_mode();
        m_debug_panel->render(app, visible);
        if (!visible) {
            m_show_debug_panel = false;
            app.set_debug_mode(false);
        }
    }

    // Demo window for development
    if (m_show_demo_window) {
        ImGui::ShowDemoWindow(&m_show_demo_window);
    }
}

void GuiManager::end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool GuiManager::wants_keyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool GuiManager::wants_mouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

void GuiManager::render_main_menu(Application& app) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ROM...", "Ctrl+O")) {
                m_show_rom_browser = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                app.request_quit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulation")) {
            bool rom_loaded = app.get_plugin_manager().is_rom_loaded();

            if (ImGui::MenuItem("Reset", "Ctrl+R", false, rom_loaded)) {
                app.reset();
            }
            if (ImGui::MenuItem(app.is_paused() ? "Resume" : "Pause", "Escape", false, rom_loaded)) {
                app.toggle_pause();
            }
            if (ImGui::MenuItem("Frame Advance", "F", false, rom_loaded && app.is_paused())) {
                app.frame_advance();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Speed", rom_loaded)) {
                if (ImGui::MenuItem("50%", nullptr, app.get_speed() == 0.5f)) {
                    app.set_speed(0.5f);
                }
                if (ImGui::MenuItem("100%", nullptr, app.get_speed() == 1.0f)) {
                    app.set_speed(1.0f);
                }
                if (ImGui::MenuItem("200%", nullptr, app.get_speed() == 2.0f)) {
                    app.set_speed(2.0f);
                }
                if (ImGui::MenuItem("Unlimited", nullptr, app.get_speed() == 0.0f)) {
                    app.set_speed(0.0f);
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Video...")) {
                m_show_settings = true;
            }
            if (ImGui::MenuItem("Audio...")) {
                m_show_settings = true;
            }
            if (ImGui::MenuItem("Input...")) {
                m_show_settings = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Speedrun Timer", nullptr, m_show_speedrun_panel)) {
                m_show_speedrun_panel = !m_show_speedrun_panel;
            }
            if (ImGui::MenuItem("Debug Panel", "F12", m_show_debug_panel || app.is_debug_mode())) {
                m_show_debug_panel = !m_show_debug_panel;
            }
            if (ImGui::MenuItem("RAM Watch")) {
                m_show_ram_watch = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("ImGui Demo")) {
                m_show_demo_window = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // TODO: Show about dialog
            }
            ImGui::EndMenu();
        }

        // Show status on the right side of menu bar
        auto& pm = app.get_plugin_manager();
        if (pm.is_rom_loaded()) {
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            ImGui::Text("%s | %s",
                       app.is_paused() ? "PAUSED" : "RUNNING",
                       pm.get_active_plugin() ?
                           pm.get_active_plugin()->get_info().name : "");
        }

        ImGui::EndMainMenuBar();
    }
}

void GuiManager::render_game_view(Application& app, Renderer& renderer) {
    ImGuiIO& io = ImGui::GetIO();

    // Create a window that fills the entire viewport (behind menu bar)
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;
    ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

    // Offset for menu bar
    float menu_height = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(viewport_pos.x, viewport_pos.y + menu_height));
    ImGui::SetNextWindowSize(ImVec2(viewport_size.x, viewport_size.y - menu_height));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("GameView", nullptr, window_flags);
    ImGui::PopStyleVar();

    // Get window size for scaling
    ImVec2 window_size = ImGui::GetContentRegionAvail();

    // Calculate scaled size maintaining aspect ratio
    int tex_width = renderer.get_texture_width();
    int tex_height = renderer.get_texture_height();

    if (tex_width > 0 && tex_height > 0) {
        float aspect = static_cast<float>(tex_width) / tex_height;
        float window_aspect = window_size.x / window_size.y;

        float display_width, display_height;
        if (window_aspect > aspect) {
            // Window is wider - fit to height
            display_height = window_size.y;
            display_width = display_height * aspect;
        } else {
            // Window is taller - fit to width
            display_width = window_size.x;
            display_height = display_width / aspect;
        }

        // Center the image
        float offset_x = (window_size.x - display_width) * 0.5f;
        float offset_y = (window_size.y - display_height) * 0.5f;

        ImGui::SetCursorPos(ImVec2(offset_x, offset_y));

        // Render the game texture
        ImGui::Image(
            reinterpret_cast<ImTextureID>(static_cast<intptr_t>(renderer.get_texture_id())),
            ImVec2(display_width, display_height),
            ImVec2(0, 0), ImVec2(1, 1)
        );
    } else {
        // No texture - show placeholder
        ImGui::SetCursorPos(ImVec2(window_size.x / 2 - 100, window_size.y / 2));
        ImGui::Text("No ROM loaded. File > Open ROM...");
    }

    ImGui::End();
}

void GuiManager::render_rom_browser(Application& app) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Open ROM", &m_show_rom_browser)) {
        // Current path
        ImGui::Text("Path: %s", m_current_directory.c_str());
        ImGui::Separator();

        // File list
        if (ImGui::BeginChild("FileList", ImVec2(0, -30))) {
            namespace fs = std::filesystem;

            try {
                // Parent directory entry
                if (fs::path(m_current_directory).has_parent_path()) {
                    if (ImGui::Selectable("..")) {
                        m_current_directory = fs::path(m_current_directory).parent_path().string();
                    }
                }

                // Directory entries
                std::vector<fs::directory_entry> entries;
                for (const auto& entry : fs::directory_iterator(m_current_directory)) {
                    entries.push_back(entry);
                }

                // Sort: directories first, then files
                std::sort(entries.begin(), entries.end(),
                    [](const fs::directory_entry& a, const fs::directory_entry& b) {
                        if (a.is_directory() != b.is_directory()) {
                            return a.is_directory();
                        }
                        return a.path().filename() < b.path().filename();
                    });

                for (const auto& entry : entries) {
                    std::string name = entry.path().filename().string();

                    if (entry.is_directory()) {
                        name = "[DIR] " + name;
                        if (ImGui::Selectable(name.c_str())) {
                            m_current_directory = entry.path().string();
                        }
                    } else {
                        // Check if it's a supported ROM file
                        std::string ext = entry.path().extension().string();
                        bool is_rom = (ext == ".nes" || ext == ".NES" ||
                                      ext == ".sfc" || ext == ".smc" ||
                                      ext == ".gb" || ext == ".gbc");

                        if (is_rom) {
                            if (ImGui::Selectable(name.c_str())) {
                                if (app.load_rom(entry.path().string())) {
                                    m_show_rom_browser = false;
                                }
                            }
                        } else {
                            ImGui::TextDisabled("%s", name.c_str());
                        }
                    }
                }
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }
        }
        ImGui::EndChild();

        // Buttons
        if (ImGui::Button("Cancel")) {
            m_show_rom_browser = false;
        }
    }
    ImGui::End();
}

void GuiManager::render_settings(Application& app) {
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Settings", &m_show_settings)) {
        if (ImGui::BeginTabBar("SettingsTabs")) {
            if (ImGui::BeginTabItem("Video")) {
                // Video settings
                static int scale = 2;
                ImGui::SliderInt("Scale", &scale, 1, 5);

                static bool fullscreen = false;
                if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
                    app.get_window_manager().toggle_fullscreen();
                }

                static bool vsync = true;
                if (ImGui::Checkbox("VSync", &vsync)) {
                    app.get_window_manager().set_vsync(vsync);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Audio")) {
                // Audio settings
                static float volume = 1.0f;
                if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
                    app.get_audio_manager().set_volume(volume);
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Input")) {
                // Render the input configuration panel
                if (m_input_config_panel) {
                    if (m_input_config_panel->render(app)) {
                        // Panel requested to close settings window
                        m_show_settings = false;
                    }
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

} // namespace emu
