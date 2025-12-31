#include "speedrun_panel.hpp"
#include "core/speedrun_manager.hpp"

#include <imgui.h>
#include <cstdio>

namespace emu {

SpeedrunPanel::SpeedrunPanel() = default;
SpeedrunPanel::~SpeedrunPanel() = default;

void SpeedrunPanel::render(SpeedrunManager& manager, bool& visible) {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(280, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Speedrun Timer", &visible)) {
        render_timer(manager);
        ImGui::Separator();
        render_splits(manager);
        ImGui::Separator();
        render_controls(manager);
    }
    ImGui::End();
}

void SpeedrunPanel::render_timer(SpeedrunManager& manager) {
    // Current time display
    uint64_t current_time = manager.get_current_time_ms();
    std::string time_str = format_time(current_time);

    // Large timer display
    ImGui::PushFont(nullptr);  // Would use large font here
    float font_size = ImGui::GetFontSize();

    ImVec4 timer_color = manager.is_timer_running() ?
        ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :   // Green when running
        ImVec4(0.8f, 0.8f, 0.8f, 1.0f);    // Grey when stopped

    ImGui::PushStyleColor(ImGuiCol_Text, timer_color);

    // Center the timer
    float text_width = ImGui::CalcTextSize(time_str.c_str()).x;
    float window_width = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX((window_width - text_width) * 0.5f);

    ImGui::TextUnformatted(time_str.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();

    // Game and category info
    if (!manager.get_game_name().empty()) {
        ImGui::TextDisabled("%s - %s",
            manager.get_game_name().c_str(),
            manager.get_category().c_str());
    }

    // PB and Sum of Best
    if (manager.get_personal_best()) {
        ImGui::Text("PB: %s", format_time(manager.get_personal_best()->total_time_ms).c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("SoB: %s", format_time(manager.get_sum_of_best_ms()).c_str());
    }
}

void SpeedrunPanel::render_splits(SpeedrunManager& manager) {
    const auto& splits = manager.get_splits();
    int current_split = manager.get_current_split_index();

    if (splits.empty()) {
        ImGui::TextDisabled("No splits loaded");
        ImGui::TextDisabled("Load a game with a speedrun plugin");
        return;
    }

    // Splits table
    if (ImGui::BeginTable("Splits", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Split", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Delta", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 70);
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(splits.size()); i++) {
            const auto& split = splits[i];

            ImGui::TableNextRow();

            // Highlight current split
            if (i == current_split) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                    ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.5f, 0.5f)));
            }

            // Split name
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(split.name.c_str());

            // Delta vs PB
            ImGui::TableNextColumn();
            if (split.completed) {
                int64_t delta = manager.get_delta_ms(i);
                ImVec4 delta_color = delta <= 0 ?
                    ImVec4(0.2f, 0.8f, 0.2f, 1.0f) :   // Green (ahead)
                    ImVec4(0.8f, 0.2f, 0.2f, 1.0f);    // Red (behind)

                ImGui::PushStyleColor(ImGuiCol_Text, delta_color);
                ImGui::TextUnformatted(format_delta(delta).c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::TextDisabled("-");
            }

            // Split time
            ImGui::TableNextColumn();
            if (split.completed) {
                ImGui::TextUnformatted(format_time(split.split_time_ms, false).c_str());
            } else {
                ImGui::TextDisabled("-");
            }
        }

        ImGui::EndTable();
    }
}

void SpeedrunPanel::render_controls(SpeedrunManager& manager) {
    float button_width = 60;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float total_width = button_width * 4 + spacing * 3;
    float offset = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;
    if (offset > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    if (manager.is_timer_running()) {
        if (ImGui::Button("Split", ImVec2(button_width, 0))) {
            manager.split();
        }
    } else {
        if (ImGui::Button("Start", ImVec2(button_width, 0))) {
            manager.start_timer();
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Undo", ImVec2(button_width, 0))) {
        manager.undo_split();
    }

    ImGui::SameLine();
    if (ImGui::Button("Skip", ImVec2(button_width, 0))) {
        manager.skip_split();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(button_width, 0))) {
        manager.reset_timer();
    }

    // Keyboard shortcuts hint
    ImGui::TextDisabled("Numpad1=Split, Numpad3=Reset");
}

std::string SpeedrunPanel::format_time(uint64_t ms, bool show_ms) const {
    uint64_t total_seconds = ms / 1000;
    uint64_t hours = total_seconds / 3600;
    uint64_t minutes = (total_seconds % 3600) / 60;
    uint64_t seconds = total_seconds % 60;
    uint64_t millis = ms % 1000;

    char buf[32];
    if (hours > 0) {
        if (show_ms) {
            snprintf(buf, sizeof(buf), "%llu:%02llu:%02llu.%03llu",
                (unsigned long long)hours, (unsigned long long)minutes,
                (unsigned long long)seconds, (unsigned long long)millis);
        } else {
            snprintf(buf, sizeof(buf), "%llu:%02llu:%02llu",
                (unsigned long long)hours, (unsigned long long)minutes,
                (unsigned long long)seconds);
        }
    } else if (minutes > 0) {
        if (show_ms) {
            snprintf(buf, sizeof(buf), "%llu:%02llu.%03llu",
                (unsigned long long)minutes, (unsigned long long)seconds,
                (unsigned long long)millis);
        } else {
            snprintf(buf, sizeof(buf), "%llu:%02llu",
                (unsigned long long)minutes, (unsigned long long)seconds);
        }
    } else {
        if (show_ms) {
            snprintf(buf, sizeof(buf), "%llu.%03llu",
                (unsigned long long)seconds, (unsigned long long)millis);
        } else {
            snprintf(buf, sizeof(buf), "%llu",
                (unsigned long long)seconds);
        }
    }

    return buf;
}

std::string SpeedrunPanel::format_delta(int64_t ms) const {
    bool negative = ms < 0;
    uint64_t abs_ms = negative ? -ms : ms;

    uint64_t total_seconds = abs_ms / 1000;
    uint64_t minutes = total_seconds / 60;
    uint64_t seconds = total_seconds % 60;
    uint64_t millis = abs_ms % 1000;

    char buf[32];
    if (minutes > 0) {
        snprintf(buf, sizeof(buf), "%c%llu:%02llu.%01llu",
            negative ? '-' : '+',
            (unsigned long long)minutes,
            (unsigned long long)seconds,
            (unsigned long long)(millis / 100));
    } else {
        snprintf(buf, sizeof(buf), "%c%llu.%01llu",
            negative ? '-' : '+',
            (unsigned long long)seconds,
            (unsigned long long)(millis / 100));
    }

    return buf;
}

} // namespace emu
