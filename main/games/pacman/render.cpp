#include "render.h"
#include <algorithm>
#include <cstdio>
namespace arcade::pacman {
namespace {
constexpr int X = 335, Y = 95, BoardWidth = Width * Cell, BoardHeight = Height * Cell;
constexpr uint32_t Background = 0x080f25;
void box(Canvas &c, int x, int y, int w, int h, uint32_t paint, int radius = 0) {
    int right = std::min(BoardWidth, x + w), bottom = std::min(BoardHeight, y + h);
    x = std::max(0, x); y = std::max(0, y);
    if (right > x && bottom > y) c.rect(X + x, Y + y, right - x, bottom - y, paint, radius);
}
}
void PacmanGame::render(Canvas &c) const {
    const auto &s = model.state;
    constexpr uint32_t themes[] = {0x356de5, 0x9865d7, 0x298e85};
    auto paint = themes[(s.level - 1) % 3];
    c.rect(X - 4, Y - 4, BoardWidth + 8, BoardHeight + 8, paint, 7);
    c.rect(X, Y, BoardWidth, BoardHeight, Background);
    // Merge wall cells into horizontal runs instead of one task per wall tile.
    for (int row = 0; row < Height; ++row) for (int col = 0; col < Width;) {
        int i = row * Width + col;
        if (s.walls[i]) {
            int first = col++;
            while (col < Width && s.walls[row * Width + col]) ++col;
            box(c, first * Cell + 2, row * Cell + 2, (col - first) * Cell - 4, Cell - 4, paint, 5);
        } else {

            if (s.dots[i] == 2) box(c, col * Cell + 7, row * Cell + 7, 10, 10, s.elapsed / 180 % 2 ? 0xffed9d : 0xf497a6, 5);
            ++col;
        }
    }
    for (int row = 0; row < Height; ++row) {
        uint32_t dots = 0;
        for (int col = 0; col < Width; ++col) if (s.dots[row * Width + col] == 1) dots |= uint32_t(1) << col;
        c.dot_row(X, Y + row * Cell + 10, dots, Width, Cell, 0xffdca3);
    }
    if (s.fruit_ms) {
        int x = (Homes[0] % Width) * Cell + 12, y = (Homes[0] / Width) * Cell + 12;
        box(c, x - 9, y - 6, 18, 17, 0xff5d86, 7);
        box(c, x + 1, y - 11, 8, 5, 0x73e994, 2);
    }
    float px, py; model.position(s.player, model.player_interval(), px, py);
    int x = int((px + .5f) * Cell), y = int((py + .5f) * Cell);
    if (!s.invulnerable_ms || (s.invulnerable_ms / 100) % 2) {
        box(c, x - 10, y - 10, 20, 20, color::Gold, 10);
        int d = s.player.direction == 4 ? 1 : s.player.direction;
        int mouth = 3 + int(s.elapsed / 70 % 3);
        for (int k = 0; k < 11; ++k) {
            int h = k * mouth / 10;
            if (d == 1 || d == 3) box(c, x + (d == 1 ? k : -k), y - h, 1, h * 2 + 1, Background);
            else box(c, x - h, y + (d == 2 ? k : -k), h * 2 + 1, 1, Background);
        }
    }
    for (int i = 0; i < GhostCount; ++i) {
        const auto &g = s.ghosts[i]; float gx, gy;
        model.position(g.actor, model.ghost_interval(), gx, gy);
        int xx = int((gx + .5f) * Cell), yy = int((gy + .5f) * Cell);
        auto color = s.power_ms && !g.respawn_ms ? (s.power_ms < 1800 && s.elapsed / 150 % 2 ? 0xf2f5ffu : 0x6382ffu) : GhostColors[i];
        if (!g.respawn_ms || g.respawn_ms < 700) {
            box(c, xx - 10, yy - 10, 20, 19, color, 9);
            for (int foot = 0; foot < 3; ++foot) box(c, xx - 10 + foot * 7, yy + 5, 6, 6, color, 2);
        }
        box(c, xx - 7, yy - 5, 6, 7, 0xffffff, 3); box(c, xx + 2, yy - 5, 6, 7, 0xffffff, 3);
        int eye = g.actor.direction == 1 ? 2 : 0;
        box(c, xx - 6 + eye, yy - 3, 3, 4, 0x173758, 1); box(c, xx + 3 + eye, yy - 3, 3, 4, 0x173758, 1);
    }
    c.text(40, 110, "PAC-MAN", color::Gold, Font::Title);
    char text[180];
    std::snprintf(text, sizeof(text), "PONTOS\n%08lu\n\nLABIRINTO\n%u\n\nVIDAS\n%u\n\nPONTOS NO MAPA\n%u", (unsigned long)s.score, s.level, s.lives, s.remaining);
    c.text(40, 190, text, color::Text);
    c.text(975, 120, MapNames[(s.level - 1) % 3], color::Cyan, Font::Small);
    const char *names[] = {"Rastro: persegue", "Rosa: antecipa", "Ciano: patrulha", "Laranja: recua"};
    for (int i = 0; i < GhostCount; ++i) c.text(975, 200 + i * 38, names[i], GhostColors[i], Font::Small);
    c.text(975, 400, "Setas: mover\nCurvas antecipadas\nP / Esc: pausa\n\nOrbes: poder\nFruta: bonus", color::Muted, Font::Small);
    std::snprintf(text, sizeof(text), "PODER: %lu s", (unsigned long)((s.power_ms + 999) / 1000));
    c.text(975, 570, text, s.power_ms ? color::Gold : color::Muted, Font::Small);
    if (s.options.mode == 2) {
        std::snprintf(text, sizeof(text), "Restam %lu s", (unsigned long)((180000 - std::min<uint32_t>(180000, s.elapsed)) / 1000));
        c.text(975, 625, text, color::Gold, Font::Small);
    }
    if (s.transition) {
        c.rect(X + 85, Y + 230, 430, 70, color::Panel, 9);
        c.text(X + 110, Y + 250, "LABIRINTO CONCLUIDO!", color::Gold, Font::Title);
    }
}
Game *create() { static PacmanGame game; return &game; }
}
