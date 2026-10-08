#pragma once
#include "journal.h"
#include "types.h"
#include "input_state.h"
#include "games/snake/config.h"
namespace arcade {
enum class Binding : uint8_t {
    Left,
    Right,
    Up,
    Down,
    Confirm,
    Back,
    Primary,
    Pause,
    Restart,
    Mute,
    CounterRotate,
    Hold,
    Count
};
constexpr uint8_t DefaultKeys[] = {key::Left,  key::Right,  key::Up,    key::Down,
                                   key::Enter, key::Escape, key::Space, key::P,
                                   key::R,     key::M,      key::Z,     key::C};
struct Settings {
    uint8_t brightness = 75, master = 65, music = 35, effects = 65, music_on = 1, effects_on = 1,
            mute = 0, theme = 0, fps = 0, grid = 1;
    char player[16] = "Jogador";
    uint8_t bindings[size_t(Binding::Count)]{};
    snake::FoodRule food[6]{};
};
Settings default_settings();
bool valid_settings(const Settings &settings);
bool load_settings(Journal &journal, Settings &settings);
bool save_settings(Journal &journal, const Settings &settings);
} // namespace arcade
