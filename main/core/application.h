#pragma once
#include "types.h"
#include "game.h"
#include "settings.h"
#include "records.h"
#include "input_state.h"
#include "menu.h"
#include <functional>
namespace arcade {
enum class Screen {
    Home,
    Games,
    Setup,
    Controls,
    Records,
    Settings,
    Remap,
    Food,
    About,
    Playing,
    Pause,
    Confirm,
    Name
};
class Application {
  public:
    explicit Application(Backend &backend, std::function<std::string()> backup = {})
        : journal(backend), records(journal), backup(std::move(backup)) {}
    void init();
    void tick(uint64_t now_us);
    void input_event(uint8_t key, bool pressed, uint32_t now) { input.event(key, pressed, now); }
    Screen screen() const { return current; }
    const Settings &preferences() const { return settings; }
    const Game *active_game() const { return active; }
    void show(Screen screen, unsigned selection = 0);

  private:
    Journal journal;
    Records records;
    Settings settings = default_settings();
    InputState input;
    Menu menu;
    Screen current = Screen::Home, return_screen = Screen::Home;
    unsigned selected_game = 0;
    Options options{};
    Game *active = nullptr;
    lv_obj_t *surface = nullptr;
    lv_obj_t *fps_label = nullptr;
    uint64_t previous_us = 0, accumulator = 0, last_frame = 0, fps_since = 0, finished_since = 0;
    uint32_t step_fraction = 0, frame_count = 0;
    uint32_t visual_crc = 0;
    bool visual_valid = false;
    float fps = 0;
    Controls pending{};
    std::string notice;
    std::function<void()> confirmed;
    std::function<std::string()> backup;
    bool naming_record = false, record_pending = false;
    int remap = -1;
    char name[16]{};
    void start(bool resume);
    void game_screen();
    bool save_active();
    bool save_exists(unsigned game);
    std::string save_key(unsigned game) const;
    void pause_game();
    void confirm(const char *message, std::function<void()> action, Screen back);
    void apply_settings();
    void persist_settings();
    void name_editor(bool record);
    void submit_name();
    void navigation(uint32_t now);
    void draw(lv_layer_t *layer);
    void handle_sounds();
    uint8_t binding(Binding action) const { return settings.bindings[size_t(action)]; }
};
} // namespace arcade
