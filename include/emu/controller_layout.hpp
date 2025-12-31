#pragma once

#include "input_types.hpp"
#include <cstdint>

namespace emu {

// Visual position of a button on the controller image
// Coordinates are normalized (0.0 to 1.0) relative to controller bounds
struct ButtonLayout {
    VirtualButton button;       // Which button this represents
    const char* label;          // Display label ("A", "Start", etc.)
    float x;                    // Normalized X position (0.0 = left, 1.0 = right)
    float y;                    // Normalized Y position (0.0 = top, 1.0 = bottom)
    float width;                // Normalized width of clickable area
    float height;               // Normalized height of clickable area
    bool is_dpad;               // True if this is part of a D-pad (for special rendering)
};

// Shape hints for controller rendering
enum class ControllerShape {
    Rectangle,      // NES-style rectangular controller
    DogBone,        // NES dogbone / SNES controller shape
    Handheld,       // Game Boy style
    Modern          // Modern gamepad with grips
};

// Complete controller layout for a platform
// Each emulator plugin defines and returns its own layout
struct ControllerLayoutInfo {
    const char* platform_name;          // "NES", "SNES", etc.
    const char* controller_name;        // "NES Controller", "SNES Controller"
    ControllerShape shape;              // Visual shape hint
    float aspect_ratio;                 // Width / Height ratio for rendering
    const ButtonLayout* buttons;        // Array of button layouts
    int button_count;                   // Number of buttons
    int controller_count;               // Number of controllers supported (1-4)
};

} // namespace emu
