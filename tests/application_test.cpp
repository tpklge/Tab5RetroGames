#include "application.h"
#include "lvgl_canvas.h"
#include "games/block_drop/render.h"
#include "games/breakout/render.h"
#include "games/frogger/render.h"
#include "games/pacman/render.h"
#include "games/bomberman/render.h"
#include "games/sokoban/render.h"
#include "games/asteroids/render.h"
#include "games/snake/render.h"
#include "games/retro_racer/render.h"
#include "games/space_invaders/render.h"
#include "games/space_impact/render.h"
#include <map>
#include <algorithm>
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
static uint16_t draw_pixels[1280 * 720];
class ReferenceCanvas : public Canvas {
  public:
    uint16_t *target;
    explicit ReferenceCanvas(uint16_t *target) : target(target) {}
    void rect(int x, int y, int w, int h, uint32_t paint, int = 0, uint8_t = 255) override {
        auto pixel = lv_color_to_u16(lv_color_hex(paint));
        for (int r = std::max(0, y); r < std::min(720, y + h); ++r)
            for (int c = std::max(0, x); c < std::min(1280, x + w); ++c) target[r * 1280 + c] = pixel;
    }
    void text(int, int, const char *, uint32_t, Font) override {}
};
static uint32_t scene_dots = 0x515;
void batch_scene(Canvas &canvas) {
    canvas.rect(0, 0, 1280, 720, 0x080c18);
    canvas.checkerboard(10, 80, 32, 22, 24, 0x0f1924, 0x142032);
    canvas.dot_row(800, 500, scene_dots, 12, 12, 0xffdca3);
    constexpr uint8_t rows[] = {0x24, 0x7e, 0xdb, 0xff, 0x24, 0x5a};
    for (int i = 0; i < 6; ++i)
        canvas.sprite(820 + i * 55, 120 + i * 55, rows, 6, 4, i % 2 ? 0x27d9ef : 0xffd166, i % 2);
}
void verify_batches(lv_display_t *display) {
    static uint16_t expected[1280 * 720];
    ReferenceCanvas reference(expected); batch_scene(reference);
    auto *surface = lv_obj_create(lv_screen_active());
    lv_obj_set_size(surface, 1280, 720); lv_obj_set_pos(surface, 0, 0);
    lv_obj_set_style_border_width(surface, 0, 0); lv_obj_set_style_pad_all(surface, 0, 0);
    lv_obj_add_event_cb(surface, [](lv_event_t *event) {
        LvglCanvas canvas(lv_event_get_layer(event)); batch_scene(canvas);
    }, LV_EVENT_DRAW_MAIN, nullptr);
    lv_refr_now(display);
    assert(std::equal(std::begin(expected), std::end(expected), std::begin(pixels)));
    // The second render uses warmed caches and must remain pixel-identical.
    lv_obj_invalidate(surface); lv_refr_now(display);
    assert(std::equal(std::begin(expected), std::end(expected), std::begin(pixels)));
    scene_dots ^= 3;
    batch_scene(reference);
    lv_obj_invalidate(surface); lv_refr_now(display);
    assert(std::equal(std::begin(expected), std::end(expected), std::begin(pixels)));
    lv_obj_delete(surface);
    puts("PASS renderer: cached background and recolored/animated masks match reference pixels, cold and warm cache");
}
class RenderCost : public Canvas {
  public:
    bool batching;
    unsigned operations = 0;
    explicit RenderCost(bool batching) : batching(batching) {}
    void rect(int, int, int, int, uint32_t, int = 0, uint8_t = 255) override { ++operations; }
    void text(int, int, const char *, uint32_t, Font) override { ++operations; }
    void line(int x, int y, int ex, int ey, uint32_t paint, int width = 2) override {
        if (batching) ++operations; else Canvas::line(x, y, ex, ey, paint, width);
    }
    void checkerboard(int x, int y, int columns, int rows, int cell, uint32_t a, uint32_t b) override {
        if (batching) ++operations;
        else Canvas::checkerboard(x, y, columns, rows, cell, a, b);
    }
    void dot_row(int x, int y, uint32_t mask, unsigned cols, int pitch, uint32_t paint) override {
        if (batching) { if (mask) ++operations; }
        else Canvas::dot_row(x, y, mask, cols, pitch, paint);
    }
    void sprite(int x, int y, const uint8_t *rows, unsigned h, int scale, uint32_t paint, bool feet) override {
        if (batching) { ++operations; return; }
        // Previous renderer submitted one rectangle per lit pixel.
        for (unsigned r = 0; r < h; ++r) for (int c = 0; c < 8; ++c)
            if (rows[r] & (1 << c)) rect(x + c * scale, y + int(r) * scale, scale, scale, paint);
        if (feet) operations += 2;
    }
};
void profile_batches() {
    for (const auto &entry : registry()) {
        Game *game = entry.create(); game->init(Options{});
        RenderCost old(false), batched(true);
        game->render(old); game->render(batched);
        if (entry.id == GameId::Snake || entry.id == GameId::SpaceInvaders)
            assert(batched.operations < old.operations / 2);
        std::printf("DRAW submissions %s: previous=%u batched=%u (native scene; not hardware FPS)\n",
                    entry.name, old.operations, batched.operations);
        game->cleanup();
    }
}
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
    lv_display_set_buffers(display, draw_pixels, nullptr, sizeof(draw_pixels),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, [](lv_display_t *display, const lv_area_t *area, uint8_t *data) {
        // Compose partial flushes at their screen coordinates, as a panel does.
        const int width = area->x2 - area->x1 + 1;
        const auto *source = reinterpret_cast<const uint16_t *>(data);
        for (int y = area->y1; y <= area->y2; ++y)
            std::memcpy(pixels + y * 1280 + area->x1, source + (y - area->y1) * width,
                        size_t(width) * sizeof(uint16_t));
        lv_display_flush_ready(display);
    });
    verify_batches(display);
    profile_batches();
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
    tap(app, key::Enter);
    tap(app, key::Down); tap(app, key::Down); tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing);
    auto *invader = static_cast<const invaders::InvadersGame *>(app.active_game());
    float initial_x = invader->model.state.player;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::X, true, uint32_t(now / 1000));
    tick(app, 12);
    assert(invader->model.state.player > initial_x && invader->model.state.cooldown);
    app.input_event(key::Right, false, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    // Capture a visible phase of the initial invulnerability blink.
    while (invader->model.state.invulnerable && !(invader->model.state.invulnerable / 100 % 2))
        tick(app);
    capture("tests/build/space-invaders.ppm");
    tap(app, key::Escape);
    assert(journal.read("save3", 3, saved));
    tap(app, key::Escape);
    assert(app.screen() == Screen::Playing);
    app.show(Screen::Home);
    tap(app, key::Enter);
    capture("tests/build/games.ppm");
    for (int i = 0; i < 4; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing);
    auto *impact_game = static_cast<const impact::ImpactGame *>(app.active_game());
    float impact_x = impact_game->model.state.x, impact_y = impact_game->model.state.y;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Up, true, uint32_t(now / 1000));
    app.input_event(key::X, true, uint32_t(now / 1000));
    tick(app, 12);
    assert(impact_game->model.state.x > impact_x && impact_game->model.state.y < impact_y && impact_game->model.state.cooldown);
    app.input_event(key::Right, false, uint32_t(now / 1000));
    app.input_event(key::Up, false, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    auto *editable_impact = const_cast<impact::ImpactGame *>(impact_game);
    editable_impact->model.start_boss();
    editable_impact->model.state.invulnerable = 0;
    tick(app, 3);
    capture("tests/build/space-impact.ppm");
    tap(app, key::Z);
    assert(impact_game->model.state.bombs == 2);
    tap(app, key::Escape);
    assert(journal.read("save4", 3, saved));
    impact::Model restored_impact;
    assert(restored_impact.load(saved) && restored_impact.state.boss_active);
    tap(app, key::Escape);
    assert(app.screen() == Screen::Playing);
    // Input/simulation must run even when no LVGL tick/draw is allowed to run.
    app.independent_updates(true);
    app.poll_game(now);
    auto sample = [&](unsigned count) {
        for (unsigned i = 0; i < count; ++i) {
            now += 4000;
            lv_tick_inc(4);
            app.poll_game(now);
        }
    };
    float before_x = impact_game->model.state.x;
    uint32_t before_time = impact_game->model.state.elapsed;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    sample(30); // 120 ms without UI/render work.
    assert(impact_game->model.state.x > before_x + 35);
    assert(impact_game->model.state.elapsed >= before_time + 110);
    app.input_event(key::Right, false, uint32_t(now / 1000));
    sample(5);
    float stopped = impact_game->model.state.x;
    sample(10);
    assert(impact_game->model.state.x == stopped);
    // A short press/release before a sample moves once and never sticks.
    app.input_event(key::Left, true, uint32_t(now / 1000));
    app.input_event(key::Left, false, uint32_t(now / 1000));
    sample(5);
    assert(impact_game->model.state.x < stopped);
    stopped = impact_game->model.state.x;
    sample(10); assert(impact_game->model.state.x == stopped);
    editable_impact->model.state.cooldown = 0;
    app.input_event(key::X, true, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    sample(3); assert(impact_game->model.state.cooldown);
    auto bombs_before = impact_game->model.state.bombs;
    app.input_event(key::Z, true, uint32_t(now / 1000));
    app.input_event(key::Z, false, uint32_t(now / 1000));
    sample(10); assert(impact_game->model.state.bombs == bombs_before - 1);
    // Pause freezes immediately, before the UI can open its menu/autosave.
    before_time = impact_game->model.state.elapsed;
    app.input_event(key::Escape, true, uint32_t(now / 1000));
    sample(30); assert(impact_game->model.state.elapsed == before_time);
    app.tick(now); assert(app.screen() == Screen::Pause);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    app.independent_updates(false);
    app.show(Screen::Home);
    tap(app, key::Enter); tap(app, key::Down); tap(app, key::Enter); tap(app, key::Enter);
    if (app.screen() == Screen::Confirm) { tap(app, key::Down); tap(app, key::Enter); }
    assert(app.screen() == Screen::Playing && app.active_game() == snake::create());
    auto *snake_game = static_cast<const snake::SnakeGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    auto snake_head = snake_game->model.segment(0);
    app.input_event(key::Up, true, uint32_t(now / 1000));
    app.input_event(key::Up, false, uint32_t(now / 1000));
    sample(3);
    assert(snake_game->model.state.pending_count == 1);
    sample(40);
    assert(snake_game->model.state.direction == 0 && snake_game->model.segment(0) < snake_head);
    app.independent_updates(false);
    app.show(Screen::Home); tap(app, key::Enter);
    for (int i = 0; i < 3; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    if (app.screen() == Screen::Confirm) { tap(app, key::Down); tap(app, key::Enter); }
    assert(app.screen() == Screen::Playing && app.active_game() == invaders::create());
    invader = static_cast<const invaders::InvadersGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    initial_x = invader->model.state.player;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Right, false, uint32_t(now / 1000));
    app.input_event(key::X, true, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    sample(3);
    assert(invader->model.state.player > initial_x && invader->model.state.cooldown);
    initial_x = invader->model.state.player;
    sample(10); assert(invader->model.state.player == initial_x);
    app.independent_updates(false);
    app.show(Screen::Home);
    tap(app, key::Enter);
    for (int i = 0; i < 5; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == breakout::create());
    auto *breaker = static_cast<const breakout::BreakoutGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::X, true, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    sample(10);
    assert(!breaker->model.state.waiting && breaker->model.state.paddle > 400);
    app.input_event(key::Right, false, uint32_t(now / 1000));
    auto *editable_breaker = const_cast<breakout::BreakoutGame *>(breaker);
    editable_breaker->model.apply(breakout::Power::Expand);
    editable_breaker->model.apply(breakout::Power::Multiball);
    editable_breaker->model.drop(600, 320, breakout::Power::Multiball);
    sample(30);
    tick(app, 3);
    capture("tests/build/breakout.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000));
    sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save5", 3, saved));
    breakout::Model restored_breaker;
    assert(restored_breaker.load(saved) && restored_breaker.state.expanded_ms);
    app.independent_updates(false);
    app.show(Screen::Home);
    tap(app, key::Enter);
    for (int i = 0; i < 6; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == frogger::create());
    auto *frog = static_cast<const frogger::FroggerGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    app.input_event(key::Up, true, uint32_t(now / 1000));
    app.input_event(key::Up, false, uint32_t(now / 1000));
    sample(30); assert(frog->model.state.row == 11);
    tick(app, 3); capture("tests/build/frogger.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000)); sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save6", 3, saved));
    frogger::Model restored_frog; assert(restored_frog.load(saved));
    app.independent_updates(false);
    app.show(Screen::Home); tap(app, key::Enter);
    for (int i = 0; i < 7; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == pacman::create());
    auto *maze = static_cast<const pacman::PacmanGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    auto initial_dots = maze->model.state.remaining;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Right, false, uint32_t(now / 1000));
    sample(50); assert(maze->model.state.remaining < initial_dots);
    auto *editable_maze = const_cast<pacman::PacmanGame *>(maze);
    editable_maze->model.state.invulnerable_ms = 0;
    for (auto &g : editable_maze->model.state.ghosts) g.respawn_ms = 0;
    sample(70); tick(app, 3); capture("tests/build/pacman.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000)); sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save7", 3, saved));
    pacman::Model restored_maze; assert(restored_maze.load(saved));
    app.independent_updates(false);
    app.show(Screen::Home); tap(app, key::Enter);
    for (int i = 0; i < 8; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == bomberman::create());
    auto *bomber = static_cast<const bomberman::BomberGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    app.input_event(key::X, true, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000));
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Right, false, uint32_t(now / 1000));
    sample(3);
    assert(bomber->model.state.bombs[0].active && bomber->model.state.bombs[0].cell == bomberman::Start);
    assert(bomber->model.state.player == bomberman::Start+1);
    sample(40);
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Right, false, uint32_t(now / 1000)); sample(3); sample(40);
    app.input_event(key::Down, true, uint32_t(now / 1000));
    app.input_event(key::Down, false, uint32_t(now / 1000)); sample(3);
    assert(bomber->model.state.player == bomberman::Start+2+bomberman::Width);
    auto *editable_bomber = const_cast<bomberman::BomberGame *>(bomber);
    editable_bomber->model.state.invulnerable_ms = 0;
    editable_bomber->model.state.bombs[0].fuse = 10;
    sample(3); sample(25); tick(app, 3); capture("tests/build/bomberman.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000)); sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save8", 3, saved));
    bomberman::Model restored_bomber; assert(restored_bomber.load(saved) && restored_bomber.state.flames[bomberman::Start]);
    app.independent_updates(false);
    app.show(Screen::Home); tap(app, key::Enter);
    for (int i = 0; i < 9; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == sokoban::create());
    auto *soko = static_cast<const sokoban::SokobanGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    auto worker_start = soko->model.state.player;
    app.input_event(key::Left, true, uint32_t(now / 1000));
    app.input_event(key::Left, false, uint32_t(now / 1000)); sample(3);
    assert(soko->model.state.moves == 1 && soko->model.state.player != worker_start);
    app.input_event(key::Z, true, uint32_t(now / 1000));
    app.input_event(key::Z, false, uint32_t(now / 1000)); sample(3);
    assert(!soko->model.state.moves && soko->model.state.player == worker_start);
    app.input_event(key::Left, true, uint32_t(now / 1000));
    app.input_event(key::Left, false, uint32_t(now / 1000)); sample(3);
    app.input_event(key::X, true, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000)); sample(3);
    assert(!soko->model.state.moves && soko->model.state.level == 1);
    app.input_event(key::Left, true, uint32_t(now / 1000));
    app.input_event(key::Left, false, uint32_t(now / 1000)); sample(30);
    tick(app, 3); capture("tests/build/sokoban.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000)); sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save9", 3, saved));
    sokoban::Model restored_soko; assert(restored_soko.load(saved) && restored_soko.state.undo_count);
    assert(restored_soko.undo() && !restored_soko.state.moves);
    app.independent_updates(false);
    app.show(Screen::Home); tap(app, key::Enter);
    for (int i = 0; i < 10; ++i) tap(app, key::Down);
    tap(app, key::Enter); tap(app, key::Enter);
    assert(app.screen() == Screen::Playing && app.active_game() == asteroids::create());
    auto *asteroid = static_cast<const asteroids::AsteroidsGame *>(app.active_game());
    app.independent_updates(true); app.poll_game(now);
    float initial_angle = asteroid->model.state.angle;
    app.input_event(key::Right, true, uint32_t(now / 1000));
    app.input_event(key::Up, true, uint32_t(now / 1000));
    app.input_event(key::X, true, uint32_t(now / 1000)); sample(150);
    app.input_event(key::Right, false, uint32_t(now / 1000));
    app.input_event(key::Up, false, uint32_t(now / 1000));
    app.input_event(key::X, false, uint32_t(now / 1000)); sample(10);
    assert(asteroid->model.state.angle != initial_angle);
    assert(asteroid->model.state.vx != 0 || asteroid->model.state.vy != 0);
    bool shot = false; for (auto &bullet : asteroid->model.state.shots) shot |= bullet.active;
    assert(shot);
    app.input_event(key::Z, true, uint32_t(now / 1000));
    app.input_event(key::Z, false, uint32_t(now / 1000)); sample(10);
    assert(asteroid->model.state.warp_ms);
    tick(app, 3); capture("tests/build/asteroids.ppm");
    app.input_event(key::Escape, true, uint32_t(now / 1000)); sample(2); app.tick(now);
    app.input_event(key::Escape, false, uint32_t(now / 1000));
    assert(app.screen() == Screen::Pause && journal.read("save10", 3, saved));
    asteroids::Model restored_asteroids;
    assert(restored_asteroids.load(saved) && restored_asteroids.state.warp_ms);
    app.independent_updates(false);
    app.show(Screen::Home);
    lv_display_delete(display);
    puts("PASS application: real LVGL menu navigation, start/drop, pause/autosave, continue, "
         "restart confirmation, keyboard name/TOP10, deletion, simultaneous racing inputs and all "
         "menu layouts; independent simulation, short taps, release, shot/bomb and immediate pause without rendering");
}
