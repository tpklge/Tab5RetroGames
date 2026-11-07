#include "render.h"
#include <cstdio>
namespace arcade::invaders {
namespace {
constexpr int X = 285, Y = 100;
void sprite(Canvas &canvas, int x, int y, const uint8_t *rows, int count, uint32_t paint, bool flip = false) {
    canvas.sprite(x - 16, y - count * 2, rows, unsigned(count), 4, paint, flip);
}
}
void InvadersGame::render(Canvas &canvas) const {
    const auto &s = model.state;
    canvas.rect(X - 4, Y - 4, Width + 8, Height + 8, 0x26385a, 8);
    canvas.rect(X, Y, Width, Height, 0x050b19);
    for (int i = 0; i < 65; ++i) {
        int x = (i * 137 + 19) % Width;
        int y = (i * 89 + int(s.elapsed / 80)) % Height;
        canvas.rect(X + x, Y + y, 2, 2, i % 3 ? 0x26385a : 0x6177a0);
    }
    constexpr uint8_t aliens[3][6] = {{0x24,0x7e,0xdb,0xff,0x24,0x5a},
                                      {0x18,0x3c,0x7e,0xdb,0xff,0x24},
                                      {0x3c,0x7e,0xdb,0xff,0x5a,0x81}};
    constexpr uint32_t paints[] = {0xff788e, 0xffd166, 0x63e6be};
    for (int i = 0; i < Count; ++i) if (s.aliens[i]) {
        int type = (i / Columns) / 2;
        sprite(canvas, X + int(s.formation_x) + (i % Columns) * 52,
               Y + int(s.formation_y) + (i / Columns) * 43, aliens[type], 6,
               paints[type], ((s.elapsed / 350) % 2));
    }
    for (int i = 0; i < 128; ++i) if (s.shields[i])
        canvas.rect(X + 80 + (i / 32) * 165 + (i % 8) * 8,
                    Y + 451 + ((i % 32) / 8) * 8, 7, 7, 0x44d9a0);
    constexpr uint8_t ship[] = {0x18,0x18,0x3c,0x7e,0xff,0xff};
    if (!s.invulnerable || (s.invulnerable / 100) % 2)
        sprite(canvas, X + int(s.player), Y + int(PlayerY), ship, 6, color::Cyan);
    if (s.ufo_active) {
        constexpr uint8_t ufo[] = {0x3c,0x7e,0xff,0xa5};
        sprite(canvas, X + int(s.ufo), Y + 25, ufo, 4, 0xf18bff);
    }
    for (const auto &shot : s.shots) if (shot.active)
        canvas.rect(X + int(shot.x) - 2, Y + int(shot.y) - 5, 4, 10,
                    shot.enemy ? 0xff788e : color::Cyan);
    canvas.text(35, 110, "SPACE\nINVADERS", color::Cyan, Font::Title);
    char text[160];
    std::snprintf(text, sizeof(text), "PONTOS\n%08lu\n\nONDA\n%u\n\nVIDAS\n%u",
                  (unsigned long)s.score, s.level, s.lives);
    canvas.text(40, 245, text, color::Text);
    canvas.text(1035, 130, "DEFENDA\nA TERRA", color::Gold, Font::Title);
    canvas.text(1035, 265, "Setas: mover\nX: atirar\nP / Esc: pausa", color::Muted, Font::Small);
    std::snprintf(text, sizeof(text), "TEMPO\n%lu s", (unsigned long)(s.elapsed / 1000));
    canvas.text(1035, 415, text, color::Text);
    if (s.options.mode == 1) canvas.text(1035, 525, "META\n5 ondas", color::Gold);
    if (s.options.mode == 2) {
        std::snprintf(text, sizeof(text), "RESTANTE\n%lu s", (unsigned long)((120000 - s.elapsed) / 1000));
        canvas.text(1035, 525, text, color::Gold);
    }
    if (s.wave_delay) canvas.text(X + 225, Y + 285, "ONDA CONCLUIDA!", color::Gold, Font::Title);
}
Game *create() { static InvadersGame game; return &game; }
}
