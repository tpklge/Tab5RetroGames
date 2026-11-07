#include "render.h"
#include <algorithm>
#include <cstdio>
namespace arcade::bomberman {
namespace {
constexpr int X = 310, Y = 120;
void actor(Canvas &c, int x, int y, uint32_t paint, bool player) {
    c.rect(x - 13, y - 15, 26, 30, paint, 8);
    c.rect(x - 9, y - 10, 18, 13, player ? 0xffe1af : 0x183149, 5);
    c.rect(x - 6, y - 6, 3, 5, 0x18263b, 1); c.rect(x + 3, y - 6, 3, 5, 0x18263b, 1);
    if (player) {
        c.rect(x - 16, y + 9, 12, 9, 0xf19de8, 4); c.rect(x + 4, y + 9, 12, 9, 0xf19de8, 4);
        c.rect(x - 3, y - 19, 6, 6, 0xf19de8, 3);
    }
}
}
void BomberGame::render(Canvas &c) const {
    const auto &s = model.state;
    constexpr uint32_t accents[] = {0x67dcb4, 0x78b6ef, 0xc18ae9, 0xffb46d, 0xf5849e};
    uint32_t accent = accents[(s.level - 1) % CampaignLevels];
    c.rect(X - 4, Y - 4, Width * Cell + 8, Height * Cell + 8, accent, 8);
    c.rect(X, Y, Width * Cell, Height * Cell, 0x143336);
    for (int col = 1; col < Width; ++col) c.rect(X + col * Cell, Y, 1, Height * Cell, 0x204346);
    for (int row = 1; row < Height; ++row) c.rect(X, Y + row * Cell, Width * Cell, 1, 0x204346);
    for (int i = 0; i < Cells; ++i) {
        int x = X + (i % Width) * Cell, y = Y + (i / Width) * Cell;
        if (s.walls[i]) {
            c.rect(x + 2, y + 2, Cell - 4, Cell - 4, 0x527189, 5);
            c.rect(x + 6, y + 6, Cell - 12, 4, 0x87a5b5, 2);
        } else if (s.crates[i]) {
            c.rect(x + 4, y + 4, Cell - 8, Cell - 8, 0xa3784d, 5);
            c.rect(x + 8, y + 9, Cell - 16, 5, 0xdfb57f, 2);
        } else if (i == Exit) {
            c.rect(x + 6, y + 6, Cell - 12, Cell - 12, model.enemy_count() ? 0x687786 : 0x7df0ab, 5);
            c.text(x + 16, y + 12, "S", 0x153c36, Font::Small);
        } else if (s.items[i]) {
            constexpr uint32_t colors[] = {0, 0xffd166, 0x94b7ff, 0xf698dc};
            const char *labels[] = {"", "F", "B", "V"};
            c.rect(x + 10, y + 10, 24, 24, colors[s.items[i]], 5);
            c.text(x + 16, y + 12, labels[s.items[i]], 0x142b3a, Font::Small);
        }
        if (s.flames[i]) {
            c.rect(x + 4, y + 4, Cell - 8, Cell - 8, 0xff914c, 9);
            c.rect(x + 12, y + 10, Cell - 24, Cell - 20, 0xffef9c, 6);
        }
    }
    for (const auto &b : s.bombs) if (b.active) {
        int x = X + b.cell % Width * Cell + Cell / 2, y = Y + b.cell / Width * Cell + Cell / 2;
        c.rect(x - 12, y - 12, 24, 24, b.fuse < 650 && s.elapsed / 90 % 2 ? 0xf58b8b : 0x142131, 12);
        c.rect(x + 3, y - 17, 6, 8, 0xffd166, 2);
        c.rect(x - 7, y - 7, 6, 5, 0x60728d, 2);
    }
    for (const auto &e : s.enemies) if (e.active) {
        float f = 1.f - std::min(1.f, float(e.clock) / model.enemy_interval(e.type));
        float x = (e.previous % Width) + (int(e.cell % Width) - int(e.previous % Width)) * f;
        float y = (e.previous / Width) + (int(e.cell / Width) - int(e.previous / Width)) * f;
        constexpr uint32_t paints[] = {0xf28f68, 0xa991ed, 0xf183aa};
        actor(c, X + int((x + .5f) * Cell), Y + int((y + .5f) * Cell), paints[e.type], false);
    }
    if (!s.invulnerable_ms || s.invulnerable_ms / 100 % 2) {
        float f = std::min(1.f, (float(model.movement_interval()) - std::min(float(s.cooldown), float(model.movement_interval()))) / 90.f);
        float x = s.previous % Width + (int(s.player % Width) - int(s.previous % Width)) * f;
        float y = s.previous / Width + (int(s.player / Width) - int(s.previous / Width)) * f;
        actor(c, X + int((x + .5f) * Cell), Y + int((y + .5f) * Cell), 0xd8f4ff, true);
    }
    c.text(40, 115, "BOMBERMAN", accent, Font::Title);
    char text[180];
    std::snprintf(text, sizeof(text), "PONTOS\n%08lu\n\nARENA\n%u\n\nVIDAS\n%u\n\nINIMIGOS\n%u", (unsigned long)s.score, s.level, s.lives, model.enemy_count());
    c.text(40, 195, text, color::Text);
    std::snprintf(text, sizeof(text), "TEMPO\n%lu s\n\nALCANCE\n%u\n\nBOMBAS\n%u", (unsigned long)((s.remaining_ms + 999) / 1000), s.range, s.capacity);
    c.text(1010, 120, text, color::Text);
    c.text(1010, 385, "Setas: mover\nX: bomba\nP / Esc: pausa\n\nITENS\nF: alcance\nB: mais bombas\nV: velocidade", color::Muted, Font::Small);
    c.text(X + 20, 630, model.enemy_count() ? "Elimine os inimigos e revele a saida S." : "Encontre a saida S escondida sob uma caixa.", accent, Font::Small);
    if (s.transition) {
        c.rect(X + 145, Y + 205, 370, 70, color::Panel, 9);
        c.text(X + 170, Y + 225, "ARENA CONCLUIDA!", color::Gold, Font::Title);
    }
}
Game *create() { static BomberGame game; return &game; }
}
