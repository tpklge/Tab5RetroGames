#include "render.h"
#include <cstdio>
#include <algorithm>
namespace arcade::racer {
namespace {
void road_rect(Canvas &canvas, int x, int y, int width, int height, uint32_t color,
               int radius = 0) {
    const int bottom = std::min(675, y + height);
    y = std::max(75, y);
    canvas.rect(x, y, width, bottom - y, color, radius);
}
void car(Canvas &canvas, int x, int y, uint32_t paint, bool turbo) {
    road_rect(canvas, x - 21, y - 25, 5, 17, 0x050b15, 2);
    road_rect(canvas, x + 16, y - 25, 5, 17, 0x050b15, 2);
    road_rect(canvas, x - 21, y + 12, 5, 17, 0x050b15, 2);
    road_rect(canvas, x + 16, y + 12, 5, 17, 0x050b15, 2);
    road_rect(canvas, x - 18, y - 32, 36, 64, paint, 7);
    road_rect(canvas, x - 13, y - 19, 26, 15, 0x223b57, 4);
    road_rect(canvas, x - 12, y + 9, 24, 12, 0x223b57, 3);
    road_rect(canvas, x - 14, y - 29, 7, 4, 0xfff1a8, 1);
    road_rect(canvas, x + 7, y - 29, 7, 4, 0xfff1a8, 1);
    if (turbo) {
        road_rect(canvas, x - 10, y + 32, 7, 24, 0xff9944, 4);
        road_rect(canvas, x + 3, y + 32, 7, 24, 0xffdc64, 4);
    }
}
} // namespace
void RacerGame::render(Canvas &canvas) const {
    const auto &state = model.state;
    constexpr int road_x = 450, top = 75, height = 600;
    bool night = (unsigned(state.distance) / 2000) % 2;
    canvas.rect(300, top, 660, height, night ? 0x10261f : 0x28624b);
    canvas.rect(road_x - 18, top, 18, height, color::Text);
    canvas.rect(road_x + 360, top, 18, height, color::Text);
    canvas.rect(road_x, top, 360, height, night ? 0x1c2236 : 0x414a60);
    for (int y = -120; y < height; y += 120) {
        int pos = y + int(state.scroll);
        road_rect(canvas, road_x + 118, top + pos, 4, 56, 0xeadab7);
        road_rect(canvas, road_x + 238, top + pos, 4, 56, 0xeadab7);
        for (int side = 0; side < 2; ++side) {
            int tree_x = side ? 865 : 345;
            road_rect(canvas, tree_x + 17, top + pos + 20, 8, 34, 0x6b4f36);
            road_rect(canvas, tree_x, top + pos, 42, 35, night ? 0x1c5845 : 0x46b075, 16);
        }
    }
    for (const auto &traffic : state.traffic)
        if (traffic.active)
            car(canvas, road_x + int(traffic.x), top + int(traffic.y),
                Vehicles[traffic.color].color, false);
    if (!state.invulnerable || (state.invulnerable / 100) % 2)
        car(canvas, road_x + int(state.x), top + int(PlayerY),
            Vehicles[state.options.vehicle].color, state.boosting);
    if (state.flash_ms)
        canvas.rect(road_x, top, 360, height, 0xff4e5d, 0, uint8_t(state.flash_ms / 4));
    char text[140];
    canvas.text(55, 105, "RETRO RACER", color::Gold, Font::Title);
    std::snprintf(text, sizeof(text),
                  "Velocidade\n%.0f km/h\n\nPontos: %lu\nDistancia: %.0f m\nUltrapassagens: %lu",
                  double(state.speed), (unsigned long)state.score, double(state.distance),
                  (unsigned long)state.overtakes);
    canvas.text(55, 180, text, color::Text);
    canvas.text(995, 120, "INTEGRIDADE", color::Cyan);
    canvas.rect(995, 175, 220, 18, color::Panel, 4);
    canvas.rect(995, 175, int(220 * state.health / Vehicles[state.options.vehicle].health), 18,
                0x48e895, 4);
    canvas.text(995, 240, "TURBO [ESPACO]", color::Gold);
    canvas.rect(995, 295, 220, 18, color::Panel, 4);
    canvas.rect(995, 295, int(state.turbo * 2.2f), 18, color::Gold, 4);
    canvas.text(995, 365, night ? "CENARIO NOTURNO" : "CENARIO DIURNO", color::Muted, Font::Small);
    std::snprintf(text, sizeof(text), "Tempo: %lu s\nNivel: %u",
                  (unsigned long)(state.elapsed / 1000), state.level);
    canvas.text(995, 450, text, color::Text);
    if (state.options.mode == 1) {
        std::snprintf(text, sizeof(text), "META: 3000 m\nRestam: %lu s",
                      (unsigned long)((60000 - std::min<uint32_t>(60000, state.elapsed)) / 1000));
        canvas.text(995, 550, text, color::Gold);
    }
}
Game *create() {
    static RacerGame game;
    return &game;
}
} // namespace arcade::racer
