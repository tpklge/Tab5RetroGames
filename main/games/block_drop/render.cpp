#include "render.h"
#include <cstdio>
namespace arcade::block {
void BlockGame::render(Canvas &canvas) const {
    const auto &state = model.state;
    constexpr int cell = 28, bx = 490, by = 90;
    canvas.rect(bx - 5, by - 5, 290, 570, color::Cyan, 5);
    canvas.rect(bx, by, 280, 560, 0x0a0c18);
    for (int y = Hidden; y < Rows; ++y)
        for (int x = 0; x < Columns; ++x) {
            uint8_t value = state.board[y][x];
            if (value) {
                bool clearing = state.clear_mask & (1u << y);
                uint32_t paint = clearing && (state.clear_ms / 35) % 2 ? 0xffffff : Colors[value];
                canvas.rect(bx + x * cell + 1, by + (y - Hidden) * cell + 1, cell - 2, cell - 2,
                            paint, 3);
            } else if (state.grid)
                canvas.rect(bx + x * cell + 1, by + (y - Hidden) * cell + 1, cell - 2, cell - 2,
                            0x151b2c, 1);
        }
    if (!state.clear_ms) {
        int ghost = model.ghost_y();
        for (auto piece : model.cells(state.piece, state.rotation)) {
            if (ghost + piece.y >= Hidden)
                canvas.rect(bx + (state.x + piece.x) * cell + 3,
                            by + (ghost + piece.y - Hidden) * cell + 3, cell - 6, cell - 6,
                            Colors[state.piece + 1], 2, 65);
            if (state.y + piece.y >= Hidden)
                canvas.rect(bx + (state.x + piece.x) * cell + 1,
                            by + (state.y + piece.y - Hidden) * cell + 1, cell - 2, cell - 2,
                            Colors[state.piece + 1], 3);
        }
    }
    char text[100];
    canvas.text(100, 120, "BLOCK DROP", color::Gold, Font::Title);
    std::snprintf(text, sizeof(text), "Pontos: %lu\nNivel: %u\nLinhas: %u",
                  (unsigned long)state.score, state.level, state.lines);
    canvas.text(100, 190, text, color::Text);
    std::snprintf(text, sizeof(text), "Tempo: %lu:%02lu", (unsigned long)(state.elapsed / 60000),
                  (unsigned long)(state.elapsed / 1000 % 60));
    canvas.text(100, 300, text, color::Muted);
    canvas.text(100, 370, "RESERVA  [C]", color::Cyan);
    if (state.hold != None)
        for (auto piece : model.cells(state.hold, 0))
            canvas.rect(120 + piece.x * 24, 425 + piece.y * 24, 22, 22, Colors[state.hold + 1], 3);
    canvas.text(850, 100, "PROXIMAS", color::Cyan);
    for (int index = 0; index < 5; ++index)
        for (auto piece : model.cells(state.queue[index], 0))
            canvas.rect(890 + piece.x * 22, 160 + index * 85 + piece.y * 22, 20, 20,
                        Colors[state.queue[index] + 1], 3);
    if (state.feedback_ms && (state.last_clear || state.spin)) {
        const char *spin = state.spin == 2         ? "T-SPIN"
                           : state.spin == 1       ? "T-SPIN MINI"
                           : state.last_clear == 4 ? "TETRIS!"
                                                   : "LINHAS!";
        std::snprintf(text, sizeof(text), "%s\nCombo: %d%s", spin, state.combo,
                      state.back_to_back ? "  B2B" : "");
        canvas.text(100, 525, text, color::Gold, Font::Body);
    }
    if (state.options.mode == 1)
        canvas.text(100, 600, "SPRINT: 40 LINHAS", color::Muted);
    if (state.options.mode == 2) {
        std::snprintf(text, sizeof(text), "ULTRA: %lu s",
                      (unsigned long)((120000 - std::min<uint32_t>(120000, state.elapsed)) / 1000));
        canvas.text(100, 600, text, color::Muted);
    }
}
Game *create() {
    static BlockGame game;
    return &game;
}
} // namespace arcade::block
