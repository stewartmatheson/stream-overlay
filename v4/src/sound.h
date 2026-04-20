#pragma once

// Synthesises and plays a short alarm tone using the Win32 multimedia API.
// The waveform is generated in memory — no external audio files are needed.
void play_alarm();
