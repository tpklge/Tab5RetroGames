#include "render.h"
#include <cstdio>
#include <algorithm>
namespace arcade::breakout {
void BreakoutGame::render(Canvas &canvas) const {
    const auto &s = model.state;
    constexpr int X = 240, Y = 100;
    const uint32_t accent = BrickColors[(s.level - 1) % Rows];
    canvas.rect(X - 4, Y - 4, Width + 8, Height + 8, 0x294261, 8);
    canvas.rect(X, Y, Width, Height, 0x091321);
    for (int i = 0; i < 20; ++i) {
        int x = X + 15 + (i * 137) % (Width - 30), y = Y + 25 + (i * 89) % 390;
        canvas.rect(x, y, 2, 2, 0x213047);
    }
    canvas.text(X + 22, Y + 14, LayoutNames[(s.level - 1) % CampaignLevels], accent, Font::Small);
    for (int i = 0; i < BrickCount; ++i) if (s.bricks[i]) {
        int x = X + BrickLeft + i % Columns * BrickPitchX;
        int y = Y + BrickTop + i / Columns * BrickPitchY;
        auto paint = BrickColors[i / Columns];
        canvas.rect(x, y, BrickWidth, BrickHeight, paint, 4);
        canvas.rect(x + 3, y + 3, BrickWidth - 6, 3, 0xffffff, 0, 100);
        if (s.bricks[i] > 1) canvas.rect(x + 22, y + 10, 8, 5, 0x27334b, 1);
        if (s.bricks[i] > 2) canvas.rect(x + 40, y + 10, 8, 5, 0x27334b, 1);
    }
    for (const auto &p : s.pickups) if (p.active && p.y >= 12 && p.y < Height - 12) {
        const uint32_t paints[] = {0x63e6be, 0xffd166, 0x7da6ff};
        const char *letters[] = {"E", "M", "L"};
        canvas.rect(X + int(p.x) - 12, Y + int(p.y) - 12, 24, 24, paints[p.type], 5);
        canvas.text(X + int(p.x) - 7, Y + int(p.y) - 9, letters[p.type], 0x0b1020, Font::Small);
    }
    int width = int(model.paddle_width());
    canvas.rect(X + int(s.paddle) - width / 2, Y + int(PaddleY - PaddleHeight / 2), width, int(PaddleHeight),
                s.flash_ms ? color::Gold : color::Cyan, 6);
    canvas.rect(X + int(s.paddle) - width / 2 + 12, Y + int(PaddleY) - 4, width - 24, 3, 0xe1ffff, 1);
    for (const auto &b : s.balls) if (b.active && b.y <= Height - Radius) {
        if (!s.waiting) canvas.rect(X + int(b.x - b.vx * .014f) - 4, Y + int(b.y - b.vy * .014f) - 4, 8, 8, 0x45647f, 4);
        canvas.rect(X + int(b.x) - int(Radius), Y + int(b.y) - int(Radius), int(Radius * 2), int(Radius * 2),
                    s.slow_ms ? 0x9abdff : 0xfff2cd, 7);
    }
    canvas.text(35, 110, "QUEBRA-\nBLOCOS", accent, Font::Title);
    char text[180];
    std::snprintf(text, sizeof(text), "PONTOS\n%08lu\n\nFASE\n%u\n\nBLOCOS\n%u",
                  (unsigned long)s.score, s.level, s.remaining);
    canvas.text(40, 225, text, color::Text);
    std::snprintf(text, sizeof(text), "VIDAS\n%s", s.options.mode == 3 ? "ILIMITADAS" : "");
    if (s.options.mode != 3) std::snprintf(text, sizeof(text), "VIDAS\n%u", s.lives);
    canvas.text(1070, 120, text, color::Gold, Font::Small);
    canvas.text(1070, 210, "BONUS\n\nE: expandir\nM: multibola\nL: lentidao", color::Muted, Font::Small);
    std::snprintf(text, sizeof(text), "TEMPO\n%lu s\n\nEXPANSAO\n%lu s\n\nLENTIDAO\n%lu s",
                  (unsigned long)(s.elapsed / 1000), (unsigned long)((s.expanded_ms + 999) / 1000),
                  (unsigned long)((s.slow_ms + 999) / 1000));
    canvas.text(1070, 385, text, color::Text, Font::Small);
    canvas.text(265, 655, "Esquerda / Direita: mover    X: lancar    P / Esc: pausar", color::Muted, Font::Small);
    if (s.waiting) {
        canvas.rect(X + 195, Y + 350, 410, 76, 0x192a43, 9);
        canvas.text(X + 232, Y + 363, "PRESSIONE X PARA LANCAR", color::Gold);
        canvas.text(X + 235, Y + 395, "Aponte a bola com as bordas da plataforma", color::Muted, Font::Small);
    }
    if (s.transition) {
        canvas.rect(X + 225, Y + 325, 350, 65, 0x192a43, 9);
        canvas.text(X + 245, Y + 343, "FASE CONCLUIDA!", color::Gold, Font::Title);
    }
    if (s.options.mode == 2) {
        std::snprintf(text, sizeof(text), "Restam %lu s", (unsigned long)((180000 - std::min<uint32_t>(180000, s.elapsed)) / 1000));
        canvas.text(50, 580, text, color::Gold, Font::Small);
    }
}
Game *create() { static BreakoutGame game; return &game; }
}
