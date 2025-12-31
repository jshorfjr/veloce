#include "audio_manager.hpp"

#include <SDL.h>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace emu {

AudioManager::AudioManager() = default;

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize(int sample_rate, int buffer_size) {
    m_sample_rate = sample_rate;
    m_buffer_size = buffer_size;

    // Set up audio specification
    SDL_AudioSpec desired, obtained;
    std::memset(&desired, 0, sizeof(desired));

    desired.freq = sample_rate;
    desired.format = AUDIO_F32SYS;
    desired.channels = 2;  // Stereo
    desired.samples = buffer_size;
    desired.callback = audio_callback;
    desired.userdata = this;

    // Open audio device
    m_device_id = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (m_device_id == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    // Verify we got what we wanted
    if (obtained.format != AUDIO_F32SYS) {
        std::cerr << "Warning: Audio format mismatch" << std::endl;
    }

    m_sample_rate = obtained.freq;
    m_buffer_size = obtained.samples;

    // Clear ring buffer
    std::memset(m_ring_buffer, 0, sizeof(m_ring_buffer));
    m_read_pos = 0;
    m_write_pos = 0;

    // Start audio paused
    m_paused = true;

    m_initialized = true;
    std::cout << "Audio manager initialized: " << m_sample_rate << " Hz, "
              << m_buffer_size << " sample buffer" << std::endl;

    return true;
}

void AudioManager::shutdown() {
    if (m_device_id) {
        SDL_CloseAudioDevice(m_device_id);
        m_device_id = 0;
    }
    m_initialized = false;
}

void AudioManager::push_samples(const float* samples, size_t count) {
    if (!m_initialized || !samples || count == 0) return;

    const size_t buffer_capacity = RING_BUFFER_SIZE * 2;  // Stereo samples
    size_t write_pos = m_write_pos.load(std::memory_order_relaxed);
    size_t read_pos = m_read_pos.load(std::memory_order_acquire);

    for (size_t i = 0; i < count; i++) {
        size_t next_write = (write_pos + 1) % buffer_capacity;
        // Check for buffer overflow - drop samples if full
        if (next_write == read_pos) {
            break;  // Buffer full, drop remaining samples
        }
        m_ring_buffer[write_pos] = samples[i] * m_volume;
        write_pos = next_write;
    }

    m_write_pos.store(write_pos, std::memory_order_release);
}

void AudioManager::audio_callback(void* userdata, uint8_t* stream, int len) {
    AudioManager* self = static_cast<AudioManager*>(userdata);
    float* buffer = reinterpret_cast<float*>(stream);
    size_t samples = len / sizeof(float);

    self->fill_audio_buffer(buffer, samples);
}

void AudioManager::fill_audio_buffer(float* buffer, size_t samples) {
    if (m_paused.load(std::memory_order_relaxed)) {
        // Fade out smoothly when paused
        for (size_t i = 0; i < samples; i += 2) {
            float fade = 1.0f - static_cast<float>(i) / samples;
            buffer[i] = m_last_sample_left * fade;
            buffer[i + 1] = m_last_sample_right * fade;
        }
        m_last_sample_left = 0.0f;
        m_last_sample_right = 0.0f;
        return;
    }

    const size_t buffer_capacity = RING_BUFFER_SIZE * 2;
    size_t read_pos = m_read_pos.load(std::memory_order_relaxed);
    size_t write_pos = m_write_pos.load(std::memory_order_acquire);

    for (size_t i = 0; i < samples; i += 2) {
        if (read_pos != write_pos) {
            m_last_sample_left = m_ring_buffer[read_pos];
            read_pos = (read_pos + 1) % buffer_capacity;
            m_last_sample_right = m_ring_buffer[read_pos];
            read_pos = (read_pos + 1) % buffer_capacity;
            buffer[i] = m_last_sample_left;
            buffer[i + 1] = m_last_sample_right;
        } else {
            // Underrun - fade out smoothly instead of abrupt silence
            m_last_sample_left *= 0.95f;
            m_last_sample_right *= 0.95f;
            buffer[i] = m_last_sample_left;
            buffer[i + 1] = m_last_sample_right;
        }
    }

    m_read_pos.store(read_pos, std::memory_order_release);
}

void AudioManager::pause() {
    if (m_device_id) {
        SDL_PauseAudioDevice(m_device_id, 1);
        m_paused = true;
    }
}

void AudioManager::resume() {
    if (m_device_id) {
        SDL_PauseAudioDevice(m_device_id, 0);
        m_paused = false;
    }
}

void AudioManager::set_volume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);
}

size_t AudioManager::get_buffered_samples() const {
    size_t read_pos = m_read_pos.load();
    size_t write_pos = m_write_pos.load();
    const size_t buffer_capacity = RING_BUFFER_SIZE * 2;

    if (write_pos >= read_pos) {
        return write_pos - read_pos;
    } else {
        return buffer_capacity - read_pos + write_pos;
    }
}

void AudioManager::clear_buffer() {
    m_read_pos.store(0, std::memory_order_relaxed);
    m_write_pos.store(0, std::memory_order_relaxed);
    m_last_sample_left = 0.0f;
    m_last_sample_right = 0.0f;
    std::memset(m_ring_buffer, 0, sizeof(m_ring_buffer));
}

} // namespace emu
