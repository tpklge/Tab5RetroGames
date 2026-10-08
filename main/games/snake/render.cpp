#include "render.h"
#include <cstdio>
#include <cmath>
namespace arcade::snake {
void SnakeGame::render(Canvas &canvas) const {
    const auto &state = model.state;
    constexpr int bx = 235, by = 110, cell = 24;
    canvas.rect(bx - 4, by - 4, Width * cell + 8, Height * cell + 8, color::Cyan, 4);
    canvas.rect(bx, by, Width * cell, Height * cell, 0x0f1924);
    for (int i = 0; i < Capacity; ++i) {
        int x = bx + (i % Width) * cell, y = by + (i / Width) * cell;
        if (state.walls[i])
            canvas.rect(x + 1, y + 1, 22, 22, 0x4b658c, 3);
        else if (((i % Width) + (i / Width)) % 2)
            canvas.rect(x, y, 24, 24, 0x142032);
    }
    for (const auto &food : state.food)
        if (food.active) {
            int x = bx + (food.cell % Width) * cell, y = by + (food.cell / Width) * cell;
            canvas.rect(x + 4, y + 4, 16, 16, FoodColor[food.type], 7);
            if (food.type && state.rules[food.type].lifetime_ms)
                canvas.rect(x + 2, y + 21,
                            int(20u * food.remaining / state.rules[food.type].lifetime_ms), 2,
                            FoodColor[food.type]);
        }
    float fraction = std::min(1.f, float(state.clock) / model.interval());
    if(state.previous_head==model.segment(0))fraction=1.f;
    for (int i = state.length - 1; i >= 0; --i) {
        uint16_t current = model.segment(i);
        uint16_t previous = i == 0                 ? state.previous_head
                            : i + 1 < state.length ? model.segment(i + 1)
                                                   : state.previous_tail;
        float x = current % Width, y = current / Width;
        int dx = int(current % Width) - int(previous % Width),
            dy = int(current / Width) - int(previous / Width);
        if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
            x = previous % Width + dx * fraction;
            y = previous / Width + dy * fraction;
        }
        int px = bx + int(x * cell), py = by + int(y * cell);
        canvas.rect(px + 2, py + 2, 20, 20,
                    i == 0           ? 0x9dff73
                    : state.flash_ms ? color::Gold
                                     : 0x43cc8c,
                    5);
        if (i == 0) {
            int eye_x = state.direction == 1 ? 15 : 5, eye_y = state.direction == 2 ? 15 : 5;
            canvas.rect(px + eye_x, py + eye_y, 4, 4, 0x102a24);
            canvas.rect(px + (state.direction % 2 ? eye_x : eye_x + 8),
                        py + (state.direction % 2 ? eye_y + 8 : eye_y), 4, 4, 0x102a24);
        }
    }
    char text[150];
    canvas.text(40, 120, "SNAKE", color::Gold, Font::Title);
    std::snprintf(text, sizeof(text), "Pontos\n%lu\n\nTamanho\n%u\n\nTempo\n%lu s",
                  (unsigned long)state.score, state.length, (unsigned long)(state.elapsed / 1000));
    canvas.text(40, 195, text, color::Text);
    canvas.text(1040, 140, "ALIMENTOS", color::Cyan);
    const char *names[6] = {"Normal", "Especial", "Raro", "Gelo", "Bonus x2", "Super bonus"};
    for (int i = 0; i < 6; ++i) {
        canvas.rect(1040, 200 + i * 50, 14, 14, FoodColor[i], 4);
        canvas.text(1065, 197 + i * 50, names[i], color::Text, Font::Small);
    }
    if (state.slow_ms)
        canvas.text(1040, 540, "VELOCIDADE\nREDUZIDA", FoodColor[3], Font::Small);
    if (state.multiplier_ms)
        canvas.text(1040, 590, "PONTOS x2", FoodColor[4], Font::Small);
}
Game *create() {
    static SnakeGame game;
    return &game;
}
} // namespace arcade::snake
