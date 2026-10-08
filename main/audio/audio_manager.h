#pragma once
#include <cstdint>
namespace arcade::audio {
enum class Effect : uint8_t {
    Navigate,
    Confirm,
    Move,
    Rotate,
    Drop,
    Line,
    Combo,
    Eat,
    Special,
    Bonus,
    Crash,
    Overtake,
    Turbo,
    GameOver,
    Level
};
bool init();
void cleanup();
void play(Effect effect);
void configure(uint8_t master, uint8_t music, uint8_t effects, bool mute);
void menu_music(bool enabled);
void engine(float speed);
bool available();
} // namespace arcade::audio
