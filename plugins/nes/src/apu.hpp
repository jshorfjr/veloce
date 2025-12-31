#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

namespace nes {

class Bus;

// NES APU (Audio Processing Unit) - 2A03
class APU {
public:
    explicit APU(Bus& bus);
    ~APU();

    void reset();
    void step(int cpu_cycles);

    uint8_t cpu_read(uint16_t address);
    void cpu_write(uint16_t address, uint8_t value);

    // Get audio samples (stereo, interleaved)
    size_t get_samples(float* buffer, size_t max_samples);

    // Save state
    void save_state(std::vector<uint8_t>& data);
    void load_state(const uint8_t*& data, size_t& remaining);

private:
    void clock_frame_counter();
    void clock_length_counters();
    void clock_envelopes();
    void clock_sweeps();

    float mix_output();

    Bus& m_bus;

    // Frame counter
    int m_frame_counter_mode = 0;
    int m_frame_counter_step = 0;
    int m_frame_counter_cycles = 0;
    bool m_irq_inhibit = false;
    bool m_frame_irq = false;

    // Pulse channels
    struct Pulse {
        bool enabled = false;
        uint8_t duty = 0;
        bool length_halt = false;
        bool constant_volume = false;
        uint8_t volume = 0;
        bool sweep_enabled = false;
        uint8_t sweep_period = 0;
        bool sweep_negate = false;
        uint8_t sweep_shift = 0;
        uint16_t timer_period = 0;
        uint16_t timer = 0;
        uint8_t sequence_pos = 0;
        uint8_t length_counter = 0;
        uint8_t envelope_counter = 0;
        uint8_t envelope_divider = 0;
        bool envelope_start = false;
        uint8_t sweep_divider = 0;
        bool sweep_reload = false;
    };
    Pulse m_pulse[2];

    // Triangle channel
    struct Triangle {
        bool enabled = false;
        bool control_flag = false;
        uint8_t linear_counter_reload = 0;
        uint16_t timer_period = 0;
        uint16_t timer = 0;
        uint8_t sequence_pos = 0;
        uint8_t length_counter = 0;
        uint8_t linear_counter = 0;
        bool linear_counter_reload_flag = false;
    };
    Triangle m_triangle;

    // Noise channel
    struct Noise {
        bool enabled = false;
        bool length_halt = false;
        bool constant_volume = false;
        uint8_t volume = 0;
        bool mode = false;
        uint16_t timer_period = 0;
        uint16_t timer = 0;
        uint16_t shift_register = 1;
        uint8_t length_counter = 0;
        uint8_t envelope_counter = 0;
        uint8_t envelope_divider = 0;
        bool envelope_start = false;
    };
    Noise m_noise;

    // DMC channel (simplified)
    struct DMC {
        bool enabled = false;
        bool irq_enabled = false;
        bool loop = false;
        uint16_t rate = 0;
        uint8_t output_level = 0;
        uint16_t sample_address = 0;
        uint16_t sample_length = 0;
    };
    DMC m_dmc;

    // Audio output buffer
    static constexpr size_t AUDIO_BUFFER_SIZE = 8192;
    std::array<float, AUDIO_BUFFER_SIZE * 2> m_audio_buffer;
    size_t m_audio_write_pos = 0;

    // Timing
    int m_cycles = 0;
    int m_sample_counter = 0;
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int CPU_FREQ = 1789773;

    // Low-pass filter state (simple first-order IIR)
    float m_filter_state = 0.0f;
    static constexpr float FILTER_ALPHA = 0.6f;  // Cutoff ~14kHz

    // Sample accumulator for averaging
    float m_sample_accumulator = 0.0f;
    int m_sample_count = 0;

    // Lookup tables
    static const uint8_t s_length_table[32];
    static const uint16_t s_noise_period_table[16];
    static const uint8_t s_duty_table[4][8];
    static const uint8_t s_triangle_table[32];
};

} // namespace nes
