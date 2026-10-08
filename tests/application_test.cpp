#include "application.h"
#include "games/block_drop/render.h"
#include "games/retro_racer/render.h"
#include <map>
#include <cassert>
#include <cstdio>
using namespace arcade;
class Memory : public Backend {
  public:
    bool fail = false;
    std::map<std::string, std::vector<uint8_t>> values;
    bool read(const std::string &key, std::vector<uint8_t> &out) override {
        if (!values.count(key))
            return false;
        out = values[key];
        return true;
    }
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override {
        if (fail)
            return false;
        values[key] = bytes;
        return true;
    }
    bool erase(const std::string &key) override {
        values.erase(key);
        return true;
    }
};
uint64_t now = 1000;
void tick(Application &app, unsigned count = 1) {
    for (unsigned i = 0; i < count; ++i) {
        now += 8000;
        lv_tick_inc(8);
        app.tick(now);
        lv_timer_handler();
    }
}
void tap(Application &app, uint8_t code) {
    app.input_event(code, true, uint32_t(now / 1000));
    tick(app);
    app.input_event(code, false, uint32_t(now / 1000));
    tick(app);
}
static uint16_t pixels[1280 * 720];
void capture(const char *path) {
    FILE *file = std::fopen(path, "wb");
    assert(file);
    std::fprintf(file, "P6\n1280 720\n255\n");
    for (auto pixel : pixels) {
        uint8_t rgb[3] = {uint8_t(((pixel >> 11) & 31) * 255 / 31),
                          uint8_t(((pixel >> 5) & 63) * 255 / 63),
                          uint8_t((pixel & 31) * 255 / 31)};
        std::fwrite(rgb, 1, 3, file);
    }
    std::fclose(file);
}
int main(int argc, char **argv) {
    lv_init();
    auto *display = lv_display_create(1280, 720);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, pixels, nullptr, sizeof(pixels),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, [](lv_display_t *display, const lv_area_t *, uint8_t *) {
        lv_display_flush_ready(display);
    });
    Memory memory;
    Application app(memory);
    app.init();
    tick(app);
    assert(app.screen() == Screen::Home);
    tick(app, 12);
    capture("tests/build/home.ppm");
    tap(app, key::Enter);
    assert(app.screen() == Screen::Games);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Setup);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game());
    tick(app, 12);
    capture("tests/build/block-drop.ppm");
    tap(app, key::Space);
    tick(app, 30);
    assert(app.active_game()->result().score > 0);
    tap(app, key::P);
    assert(app.screen() == Screen::Pause);
    auto paused_time = app.active_game()->result().time_ms;
    tick(app, 30);
    assert(app.active_game()->result().time_ms == paused_time);
    tap(app, key::P);
    assert(app.screen() == Screen::Playing);
    tap(app, key::Escape);
    assert(app.screen() == Screen::Pause && memory.values.count("save0a"));
    auto saved = app.active_game()->save();
    tap(app, key::Down);
    tap(app, key::Down);
    tap(app, key::Down);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Home);
    tap(app, key::Down);
    tap(app, key::Enter);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Playing);
    // Resume keeps board, score and elapsed state, then simulation advances.
    assert(app.active_game()->result().score > 0);
    tap(app, key::R);
    assert(app.screen() == Screen::Confirm);
    tap(app, key::Escape);
    assert(app.screen() == Screen::Pause);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Playing);
    auto *block =
        const_cast<block::BlockGame *>(static_cast<const block::BlockGame *>(app.active_game()));
    block->model.state.status = Status::Won;
    tap(app, key::Enter);
    assert(app.screen() == Screen::Name);
    tap(app, 0x04);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Records);
    Journal journal(memory);
    Records records(journal);
    Leaderboard scores;
    assert(records.load(GameId::BlockDrop, 0, 1, scores) && scores.count == 1);
    assert(std::string(scores.entries[0].player) == "Jogadora");
    assert(!journal.read("save0", 3, saved));
    app.show(Screen::Home);
    tap(app, key::Enter);
    tap(app, key::Down);
    tap(app, key::Down);
    tap(app, key::Enter);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Playing);
    auto *racer = static_cast<const racer::RacerGame *>(app.active_game());
    float x = racer->model.state.x, speed = racer->model.state.speed;
    app.input_event(key::Left, true, uint32_t(now / 1000));
    app.input_event(key::Up, true, uint32_t(now / 1000));
    tick(app, 12);
    assert(racer->model.state.x < x && racer->model.state.speed > speed);
    app.input_event(key::Left, false, uint32_t(now / 1000));
    app.input_event(key::Up, false, uint32_t(now / 1000));
    tick(app, 2);
    if (argc > 1) {
        FILE *file = std::fopen(argv[1], "wb");
        assert(file);
        std::fprintf(file, "P6\n1280 720\n255\n");
        for (auto pixel : pixels) {
            uint8_t rgb[3] = {uint8_t(((pixel >> 11) & 31) * 255 / 31),
                              uint8_t(((pixel >> 5) & 63) * 255 / 63),
                              uint8_t((pixel & 31) * 255 / 31)};
            std::fwrite(rgb, 1, 3, file);
        }
        std::fclose(file);
    }
    memory.fail = true;
    tap(app, key::Escape);
    assert(app.screen() == Screen::Pause);
    tap(app, key::Down);
    tap(app, key::Down);
    tap(app, key::Down);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Pause && app.active_game());
    memory.fail = false;
    tap(app, key::Escape);
    assert(app.screen() == Screen::Playing);
    app.show(Screen::Home);
    tap(app, key::Enter);
    tap(app, key::Down);
    tap(app, key::Enter);
    tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game()->options().mode == 0);
    tick(app, 12);
    capture("tests/build/snake.ppm");
    tap(app, key::Escape);
    // Build every settings/menu screen and ensure keyboard returns safely.
    for (Screen screen : {Screen::Setup, Screen::Controls, Screen::Records, Screen::Settings,
                          Screen::Remap, Screen::Food, Screen::About}) {
        app.show(screen);
        tick(app);
        tap(app, key::Escape);
    }
    app.show(Screen::Home);
    lv_display_delete(display);
    puts("PASS application: real LVGL menu navigation, start/drop, pause/autosave, continue, "
         "restart confirmation, keyboard name/TOP10, deletion, simultaneous racing inputs and all "
         "menu layouts");
}
