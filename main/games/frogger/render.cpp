#include "render.h"
#include <algorithm>
#include <cstdio>
namespace arcade::frogger {
namespace {
constexpr int X = 280, Y = 110;
void box(Canvas &c, int x, int y, int w, int h, uint32_t paint, int radius = 0) {
    int right = std::min(Width, x + w), bottom = std::min(Height, y + h);
    x = std::max(0, x); y = std::max(0, y);
    if (right > x && bottom > y) c.rect(X + x, Y + y, right - x, bottom - y, paint, radius);
}
void frog(Canvas &c, int x, int y, uint32_t paint) {
    box(c, x - 15, y - 10, 7, 21, paint, 3); box(c, x + 8, y - 10, 7, 21, paint, 3);
    box(c, x - 10, y - 11, 20, 23, paint, 5);
    box(c, x - 10, y - 13, 7, 8, 0xc9ff99, 3); box(c, x + 3, y - 13, 7, 8, 0xc9ff99, 3);
    box(c, x - 8, y - 12, 3, 4, 0x132b24); box(c, x + 5, y - 12, 3, 4, 0x132b24);
}
}
void FroggerGame::render(Canvas &c) const {
    const auto &s = model.state;
    c.rect(X - 4, Y - 4, Width + 8, Height + 8, 0x2e6857, 8);
    c.rect(X, Y, Width, Height, 0x254c3c);
    box(c, 0, 0, Width, CellHeight, 0x103b5e);
    for (int row = 1; row <= 4; ++row) {
        box(c, 0, row * CellHeight, Width, CellHeight, row % 2 ? 0x103b5e : 0x16476a);
        for (int x = 0; x < Width; x += 95) box(c, x + int(s.elapsed / 160 % 40), row * CellHeight + 33, 23, 2, 0x367c9a);
    }
    box(c, 0, 6 * CellHeight, Width, 5 * CellHeight, 0x263247);
    for (int row = 7; row <= 10; ++row)
        for (int x = 8; x < Width; x += 72) box(c, x, row * CellHeight, 34, 2, 0x73839b);
    for (int i = 0; i < HomeCount; ++i) {
        box(c, int(HomeX[i]) - 22, 2, 44, 35, s.homes & (1 << i) ? 0x3b8759 : 0x112b2b, 7);
        if (s.homes & (1 << i)) frog(c, int(HomeX[i]), 20, 0xffd166);
    }
    constexpr uint32_t paints[] = {0xff708f, 0xffcf68, 0x8f9eff, 0x5ee1d1, 0xff9d5f};
    for (int i = 0; i < LaneCount; ++i) {
        const auto &l = s.lanes[i]; int row = i < 4 ? i + 1 : i + 2;
        for (int n = -1; n < 5; ++n) {
            int x = int(l.offset) + n * l.period, y = row * CellHeight + 7;
            if (i < 4) {
                box(c, x, y, l.length, 26, 0x986d45, 10);
                box(c, x + 9, y + 6, l.length - 18, 3, 0xcc9b68, 2);
            } else {
                box(c, x, y, l.length, 27, paints[i - 4], 6);
                box(c, x + 9, y + 4, 17, 19, 0x23354c, 3);
                box(c, x + l.length - 15, y + 3, 8, 21, 0x23354c, 2);
            }
        }
    }
    float fraction = std::min(1.f, (140.f - s.cooldown) / 80.f);
    float x = s.previous_x + (s.x - s.previous_x) * fraction;
    float row = s.previous_row + (float(s.row) - s.previous_row) * fraction;
    if (!s.respawn_ms || (s.respawn_ms / 100) % 2) frog(c, int(x), int(row * CellHeight) + 20, 0x8af58b);
    c.text(40, 110, "FROGGER", 0x8af58b, Font::Title);
    char text[160];
    std::snprintf(text, sizeof(text), "PONTOS\n%08lu\n\nRODADA\n%u\n\nVIDAS\n%u", (unsigned long)s.score, s.level, s.lives);
    c.text(40, 210, text, color::Text);
    c.text(1040, 130, "TRAVESSIA", color::Cyan);
    std::snprintf(text, sizeof(text), "%lu s\n\nChegadas: %lu", (unsigned long)((s.remaining_ms + 999) / 1000), (unsigned long)s.crossings);
    c.text(1040, 180, text, color::Text, Font::Small);
    c.text(1040, 300, "Setas: saltar\nSegure: repetir\nP / Esc: pausa\n\nTroncos carregam\no sapo.\n\nOcupe os cinco\nabrigos vazios.", color::Muted, Font::Small);
    if (s.respawn_ms && s.reason) {
        const char *reasons[] = {"", "ATROPELAMENTO", "CAIU NA AGUA", "TEMPO ESGOTADO"};
        c.text(X + 200, Y + 470, reasons[s.reason], color::Gold);
    }
    if (s.transition) c.text(X + 205, Y + 220, "RODADA CONCLUIDA!", color::Gold, Font::Title);
    c.rect(X, 650, Width, 6, 0x182c3c, 3);
    c.rect(X, 650, int(Width * float(s.remaining_ms) / model.crossing_time()), 6, 0x8af58b, 3);
}
Game *create() { static FroggerGame game; return &game; }
}
