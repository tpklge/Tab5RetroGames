#pragma once
#include "../../main/audio/audio_manager.h"
namespace arcade::audio {
inline void play(Effect) {}
inline void configure(uint8_t, uint8_t, uint8_t, bool) {}
inline void menu_music(bool) {}
inline void engine(float) {}
inline bool available() { return true; }
} // namespace arcade::audio
