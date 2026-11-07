#include "render.h"
#include <algorithm>
#include <cstdio>
namespace arcade::sokoban {
void SokobanGame::render(Canvas &c) const {
    const auto &s = model.state;
    constexpr int X = 335, Y = 115;
    c.rect(X - 4, Y - 4, Width * Cell + 8, Height * Cell + 8, 0x9a80d8, 9);
    c.rect(X, Y, Width * Cell, Height * Cell, 0x17283a);
    unsigned placed = 0, boxes = 0;
    for (int i = 0; i < Cells; ++i) {
        int x = X + i % Width * Cell, y = Y + i / Width * Cell;
        if (s.walls[i]) {
            c.rect(x + 2, y + 2, Cell - 4, Cell - 4, 0x586a86, 5);
            c.rect(x + 8, y + 8, Cell - 16, 4, 0x91a1b9, 2);
        } else {
            c.rect(x + 3, y + 3, Cell - 6, Cell - 6, (i % Width + i / Width) % 2 ? 0x1c3044 : 0x20364a, 3);
            if (s.goals[i]) {
                c.rect(x + 15, y + 15, 26, 26, color::Gold, 13);
                c.rect(x + 20, y + 20, 16, 16, 0x20364a, 8);
            }
            if (s.boxes[i]) {
                ++boxes; placed += s.goals[i];
                c.rect(x + 8, y + 8, 40, 40, s.goals[i] ? 0x73d7a8 : 0xd5a066, 5);
                c.rect(x + 13, y + 13, 30, 30, s.goals[i] ? 0x306f61 : 0x885b3c, 3);
                c.rect(x + 18, y + 18, 20, 20, s.goals[i] ? 0x92f2b8 : 0xe8bb85, 3);
            }
        }
    }
    float f = 1.f - std::min(1.f, s.anim_ms / 90.f);
    float xx = s.previous % Width + (int(s.player % Width) - int(s.previous % Width)) * f;
    float yy = s.previous / Width + (int(s.player / Width) - int(s.previous / Width)) * f;
    int px = X + int((xx + .5f) * Cell), py = Y + int((yy + .5f) * Cell);
    c.rect(px - 13, py - 4, 26, 23, 0x8ca9ff, 6);
    c.rect(px - 11, py - 17, 22, 21, 0xffdbad, 9);
    c.rect(px - 15, py - 22, 30, 11, 0xffd166, 5);
    c.rect(px - 6, py - 7, 3, 4, 0x152e43, 1); c.rect(px + 4, py - 7, 3, 4, 0x152e43, 1);
    c.text(40, 115, "SOKOBAN", 0xc3a1ff, Font::Title);
    char text[180];
    std::snprintf(text, sizeof(text), "FASE\n%u / %u\n\nMOVIMENTOS\n%lu\n\nEMPURROES\n%lu\n\nCAIXAS NO ALVO\n%u / %u", s.level, LevelsPerPack,
                  (unsigned long)s.moves, (unsigned long)s.pushes, placed, boxes);
    c.text(40, 195, text, color::Text);
    c.text(995, 120, "Setas: mover\nZ: desfazer\nX: reiniciar fase\nR: nova campanha\nP / Esc: pausa", color::Muted, Font::Small);
    std::snprintf(text, sizeof(text), "HISTORICO\n%u %s\n\nTEMPO\n%lu s\n\nPONTOS\n%lu", s.undo_count, s.undo_count == 1 ? "passo" : "passos", (unsigned long)(s.elapsed / 1000), (unsigned long)s.score);
    c.text(995, 290, text, color::Text, Font::Small);
    if (s.options.mode == 1 || s.options.mode == 2) {
        std::snprintf(text, sizeof(text), "LIMITE\n%u %s", s.options.mode == 1 ? model.move_limit() : model.push_limit(), s.options.mode == 1 ? "movimentos" : "empurroes");
        c.text(995, 520, text, color::Gold, Font::Small);
    }
    const char *hint = model.limited() ? "Limite atingido: Z desfaz / X recomeca."
                     : model.deadlocked() ? "Caixa presa num canto: Z desfaz / X recomeca."
                                          : "Empurre as caixas ate os circulos dourados.";
    c.text(X + 10, 640, hint, model.limited() || model.deadlocked() ? color::Gold : color::Muted, Font::Small);
    if (s.transition) {
        c.rect(X + 135, Y + 210, 350, 70, color::Panel, 9);
        c.text(X + 160, Y + 232, "FASE RESOLVIDA!", color::Gold, Font::Title);
    }
}
Game *create() { static SokobanGame game; return &game; }
}
