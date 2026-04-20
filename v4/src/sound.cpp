#include "sound.h"

#include <windows.h>
#include <mmsystem.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>

// WAV header for PCM data (44 bytes).
struct WavHeader {
    char     riff[4]      = {'R','I','F','F'};
    uint32_t file_size    = 0;   // filled in below
    char     wave[4]      = {'W','A','V','E'};
    char     fmt_id[4]    = {'f','m','t',' '};
    uint32_t fmt_size     = 16;
    uint16_t format       = 1;   // PCM
    uint16_t channels     = 1;   // mono
    uint32_t sample_rate  = 44100;
    uint32_t byte_rate    = 44100 * 2;
    uint16_t block_align  = 2;
    uint16_t bits         = 16;
    char     data_id[4]   = {'d','a','t','a'};
    uint32_t data_size    = 0;   // filled in below
};

static constexpr double PI = 3.14159265358979323846;
static constexpr uint32_t SAMPLE_RATE = 44100;

// Render a tone burst into the sample buffer.
// freq_hz  — pitch of the tone
// duration — length in seconds
// volume   — 0.0 to 1.0
static void render_tone(std::vector<int16_t>& buf, double freq_hz,
                         double duration, double volume) {
    auto samples = static_cast<size_t>(SAMPLE_RATE * duration);
    auto fade    = static_cast<size_t>(SAMPLE_RATE * 0.01); // 10 ms fade

    size_t offset = buf.size();
    buf.resize(offset + samples);

    for (size_t i = 0; i < samples; ++i) {
        double t   = static_cast<double>(i) / SAMPLE_RATE;
        double env = 1.0;

        // Fade in
        if (i < fade) env = static_cast<double>(i) / static_cast<double>(fade);
        // Fade out
        if (i >= samples - fade)
            env = static_cast<double>(samples - 1 - i) / static_cast<double>(fade);

        double sample = std::sin(2.0 * PI * freq_hz * t) * volume * env;
        buf[offset + i] = static_cast<int16_t>(sample * 32767.0);
    }
}

// Render silence.
static void render_silence(std::vector<int16_t>& buf, double duration) {
    auto samples = static_cast<size_t>(SAMPLE_RATE * duration);
    buf.resize(buf.size() + samples, 0);
}

// Build the alarm waveform: three short descending pulses.
static std::vector<uint8_t> build_alarm_wav() {
    std::vector<int16_t> pcm;

    for(int i = 0; i < 3; i++) {
      // Ascending major triad: C5 – E5 – G5.
      render_tone(pcm, 523.0, 0.15, 0.9);
      render_silence(pcm, 0.08);
      render_tone(pcm, 659.0, 0.15, 0.9);
      render_silence(pcm, 0.08);
      render_tone(pcm, 784.0, 0.25, 0.9);
      render_silence(pcm, 0.5);
    }

    // Pack into a WAV buffer.
    uint32_t data_bytes = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));

    WavHeader hdr;
    hdr.data_size = data_bytes;
    hdr.file_size = sizeof(WavHeader) - 8 + data_bytes;

    std::vector<uint8_t> wav(sizeof(WavHeader) + data_bytes);
    std::memcpy(wav.data(), &hdr, sizeof(hdr));
    std::memcpy(wav.data() + sizeof(hdr), pcm.data(), data_bytes);

    return wav;
}

void play_alarm() {
    // Detach so the caller (render loop) is never blocked.
    std::thread([] {
        auto wav = build_alarm_wav();
        PlaySoundA(reinterpret_cast<const char*>(wav.data()),
                   nullptr,
                   SND_MEMORY | SND_SYNC);
    }).detach();
}
