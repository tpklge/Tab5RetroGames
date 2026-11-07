#include "render.h"
#include <algorithm>
#include <cstdio>
namespace arcade::impact {
namespace {
constexpr int X = 80, Y = 140;
constexpr uint32_t Accents[] = {0x4de6b1, 0xcf8bff, 0xffb454};
void box(Canvas &c, float x, float y, int w, int h, uint32_t color, int radius = 0) {
    int left = std::max(0, int(x)), top = std::max(0, int(y));
    int right = std::min(Width, int(x) + w), bottom = std::min(Height, int(y) + h);
    if (right > left && bottom > top) c.rect(X + left, Y + top, right - left, bottom - top, color, radius);
}
void ship(Canvas &c, float x, float y, uint32_t age) {
    box(c, x - 34 - (age / 70 % 3) * 3, y - 7, 24, 14, 0xff8b53, 6);
    box(c, x - 26, y - 4, 18, 8, 0xffed9b, 4);
    box(c, x - 17, y - 19, 18, 38, 0x5378d4, 3);
    box(c, x - 23, y - 11, 38, 22, 0x6ce7ef, 5);
    box(c, x - 8, y - 7, 33, 14, 0xe8f7ff, 4);
    box(c, x - 5, y - 6, 15, 12, 0x1f4274, 3);
    box(c, x + 19, y - 3, 12, 6, 0xffd166, 2);
}
void enemy(Canvas &c, const Enemy &e, uint32_t accent) {
    const uint32_t paint[] = {0xf875a3, 0xffa35b, accent};
    box(c, e.x - 18, e.y - 20, 30, 40, paint[e.type], e.type == 1 ? 15 : 4);
    box(c, e.x - 27, e.y - 5, 22, 10, 0xffe8b7, 3);
    box(c, e.x - 10, e.y - 9, 19, 18, 0x243250, 5);
    box(c, e.x - 13, e.y - 3, 9, 6, 0xff4e69, 2);
    if (e.type == 2) {
        box(c, e.x - 26, e.y - 26, 42, 7, paint[2], 2);
        box(c, e.x - 26, e.y + 19, 42, 7, paint[2], 2);
    }
}
void boss(Canvas &c, const State &s, unsigned type) {
    float x = s.boss_x, y = s.boss_y;
    uint32_t accent = Accents[type];
    for (int side : {-1, 1}) {
        box(c, x - 35, y + side * 44 - 9, 85, 18, accent, 6);
        box(c, x - 60, y + side * 38 - 6, 42, 12, 0xff647d, 4);
        if (type >= 1) {
            box(c, x + 10, y + side * 68 - 7, 36, 14, accent, 4);
            box(c, x - 8, y + side * 56 - 10, 18, 30, accent, 4);
        }
    }
    box(c, x - 40, y - 38, 92, 76, 0x304768, 14);
    box(c, x - 48, y - 26, 76, 52, accent, 10);
    box(c, x - 57, y - 16, 40, 32, 0x12213a, 9);
    box(c, x - 48, y - 9, 21, 18, (s.boss_age / 120 % 2) ? 0xfff0ac : 0xff596c, 5);
    for (int i = 0; i < 3; ++i) box(c, x + 28, y - 24 + i * 20, 18, 8, 0x152037, 3);
    if (type == 2) {
        box(c, x - 21, y - 22, 37, 44, 0xff754a, 8);
        box(c, x - 13, y - 13, 20, 26, 0xffed9b, 7);
    }
}
}
void ImpactGame::render(Canvas &c) const {
    const auto &s = model.state;
    unsigned stage = (s.level - 1) % 3;
    uint32_t accent = Accents[stage];
    constexpr uint32_t skies[] = {0x081e2a, 0x170e30, 0x291724};
    constexpr uint32_t hills[] = {0x123f46, 0x382652, 0x57322e};
    c.text(80, 72, "SPACE IMPACT", accent, Font::Title);
    char text[180];
    std::snprintf(text, sizeof(text), "SETOR %u   %s", s.level, StageNames[stage]);
    c.text(410, 82, text, color::Text, Font::Small);
    std::snprintf(text, sizeof(text), "PONTOS %08lu", (unsigned long)s.score);
    c.text(930, 80, text, color::Gold, Font::Small);
    c.rect(X - 3, Y - 3, Width + 6, Height + 6, accent, 7);
    c.rect(X, Y, Width, Height, skies[stage]);
    for (int i = 0; i < 90; ++i) {
        int x = (i * 137 + 2240 - int(s.scroll * (i % 2 ? .3f : .6f))) % Width;
        box(c, float(x), float((i * 89 + 17) % Height), i % 4 ? 2 : 4, 2, i % 3 ? 0x536583 : accent);
    }
    for (int i = 0; i < 16; ++i) {
        int x = i * 90 - int(s.scroll * .35f) % 90;
        int h = 22 + (i * 37) % 60;
        box(c, float(x), float(Height - h), 75, h, hills[stage], 8);
        box(c, float(x + 10), 0, 55, h / 2, hills[stage], 6);
    }
    for (const auto &e : s.enemies) if (e.active) enemy(c, e, accent);
    if (s.boss_active) boss(c, s, stage);
    for (const auto &p : s.pickups) if (p.active) {
        constexpr uint32_t paints[] = {0x57d7ff, 0x75f1a1, 0xffd166};
        box(c, p.x - 12, p.y - 12, 24, 24, paints[p.type], 5);
        const char *labels[] = {"W", "+", "B"};
        if (p.x >= 12 && p.x <= Width - 12) c.text(X + int(p.x) - 7, Y + int(p.y) - 9, labels[p.type], 0x0b1020, Font::Small);
    }
    for (const auto &shot : s.shots) if (shot.active)
        box(c, shot.x - 5, shot.y - 3, shot.enemy ? 9 : 18, 6, shot.enemy ? 0xff658b : 0x91f7ff, 3);
    if (!s.invulnerable || (s.invulnerable / 100) % 2) ship(c, s.x, s.y, s.elapsed);
    if (s.flash) c.rect(X, Y, Width, Height, 0xffe4b5, 0, uint8_t(s.flash / 3));
    if (s.boss_active) {
        c.rect(X + 310, Y + 12, 500, 42, 0x0b1020, 5);
        std::snprintf(text, sizeof(text), "%s%s", BossNames[stage], s.boss_health * 2 < s.boss_max ? "  / FURIA" : "");
        c.text(X + 325, Y + 16, text, accent, Font::Small);
        c.rect(X + 325, Y + 40, 470, 7, color::Panel, 2);
        c.rect(X + 325, Y + 40, int(470u * s.boss_health / s.boss_max), 7, 0xff658b, 2);
    } else if (!s.transition) {
        int width = int(Width * std::min<uint32_t>(StageDuration, s.stage_time) / StageDuration);
        c.rect(X, Y + Height - 4, width, 4, accent);
    }
    if (s.transition) c.text(X + 390, Y + 225, "SETOR CONCLUIDO!", color::Gold, Font::Title);
    std::snprintf(text, sizeof(text), "VIDAS %u   ARMA %u/3   BOMBAS %u     TEMPO %lu s",
                  s.lives, s.weapon, s.bombs, (unsigned long)(s.elapsed / 1000));
    c.text(85, 646, text, color::Text, Font::Small);
    c.text(740, 646, "Setas: mover   X: tiro   Z: bomba", accent, Font::Small);
    if (s.options.mode == 2) {
        std::snprintf(text, sizeof(text), "Restam %lu s", (unsigned long)((180000 - std::min<uint32_t>(180000, s.elapsed)) / 1000));
        c.text(1010, 112, text, color::Gold, Font::Small);
    }
}
Game *create() { static ImpactGame game; return &game; }
}
