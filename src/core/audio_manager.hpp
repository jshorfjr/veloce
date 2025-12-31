#pragma once

#include <cstdint>
#include <cstddef>
#include <atomic>

namespace emu {

// Default audio settings
constexpr int DEFAULT_SAMPLE_RATE = 44100;
constexpr int DEFAULT_BUFFER_SIZE = 2048;

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Disable copy
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Initialize audio system
    bool initialize(int sample_rate = DEFAULT_SAMPLE_RATE,
                   int buffer_size = DEFAULT_BUFFER_SIZE);

    // Shutdown
    void shutdown();

    // Push audio samples from emulator
    void push_samples(const float* samples, size_t count);

    // Control
    void pause();
    void resume();
    void set_volume(float volume);  // 0.0 - 1.0
    float get_volume() const { return m_volume; }

    // Status
    bool is_initialized() const { return m_initialized; }
    int get_sample_rate() const { return m_sample_rate; }
    size_t get_buffered_samples() const;

    // Clear audio buffer
    void clear_buffer();

private:
    // SDL audio callback
    static void audio_callback(void* userdata, uint8_t* stream, int len);
    void fill_audio_buffer(float* buffer, size_t samples);

    // Ring buffer for audio samples (lock-free SPSC)
    static constexpr size_t RING_BUFFER_SIZE = 65536;  // ~1.5 seconds at 44kHz
    float m_ring_buffer[RING_BUFFER_SIZE * 2];  // Stereo
    std::atomic<size_t> m_read_pos{0};
    std::atomic<size_t> m_write_pos{0};

    // Last sample for smooth underrun handling
    float m_last_sample_left = 0.0f;
    float m_last_sample_right = 0.0f;

    // State
    bool m_initialized = false;
    uint32_t m_device_id = 0;
    int m_sample_rate = DEFAULT_SAMPLE_RATE;
    int m_buffer_size = DEFAULT_BUFFER_SIZE;
    float m_volume = 1.0f;
    std::atomic<bool> m_paused{false};
};

} // namespace emu
