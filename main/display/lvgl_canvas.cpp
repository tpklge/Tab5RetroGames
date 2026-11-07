#include "lvgl_canvas.h"
#include <array>
#include <memory>
#include <new>
#include <cstring>
namespace arcade {
namespace {
struct Mask {
    std::array<uint8_t, 8> rows{};
    unsigned height = 0;
    int scale = 0;
    bool feet = false;
    std::unique_ptr<uint8_t[]> pixels;
    lv_image_dsc_t image{};
};
std::array<Mask, 12> masks;
struct Board {
    int columns = 0, rows = 0, cell = 0;
    uint32_t first = 0, second = 0;
    std::unique_ptr<uint8_t[]> pixels;
    lv_image_dsc_t image{};
};
Board board;
struct DotStrip {
    int x = 0, y = 0, pitch = 0;
    unsigned columns = 0;
    uint32_t mask = 0;
    std::unique_ptr<uint8_t[]> pixels;
    lv_image_dsc_t image{};
};
std::array<DotStrip, 32> strips;
void header(lv_image_dsc_t &image, int w, int h, lv_color_format_t format, unsigned stride,
            unsigned bytes, const uint8_t *data) {
    image = {};
    image.header.magic = LV_IMAGE_HEADER_MAGIC;
    image.header.cf = format;
    image.header.w = w; image.header.h = h; image.header.stride = stride;
    image.data_size = bytes; image.data = data;
}
}
void LvglCanvas::sprite(int x, int y, const uint8_t *rows, unsigned height, int scale,
                        uint32_t paint, bool feet) {
    if (!rows || !height || height > 8 || scale < 1 || scale > 8) return;
    Mask *cached = nullptr;
    for (auto &entry : masks)
        if (entry.pixels && entry.height == height && entry.scale == scale && entry.feet == feet &&
            std::memcmp(entry.rows.data(), rows, height) == 0) { cached = &entry; break; }
    if (!cached) for (auto &entry : masks) if (!entry.pixels) {
        int w = 8 * scale, h = int(height + (feet ? 1 : 0)) * scale;
        auto pixels = std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[w * h]{});
        if (!pixels) break;
        for (unsigned r = 0; r < height; ++r) for (int c = 0; c < 8; ++c)
            if (rows[r] & (1 << c)) for (int dy = 0; dy < scale; ++dy)
                std::memset(pixels.get() + (r * scale + dy) * w + c * scale, 255, size_t(scale));
        if (feet) for (int dy = 0; dy < scale; ++dy) {
            std::memset(pixels.get() + (int(height) * scale + dy) * w, 255, size_t(scale));
            std::memset(pixels.get() + (int(height) * scale + dy) * w + 7 * scale, 255, size_t(scale));
        }
        std::memcpy(entry.rows.data(), rows, height);
        entry.height = height; entry.scale = scale; entry.feet = feet;
        entry.pixels = std::move(pixels);
        header(entry.image, w, h, LV_COLOR_FORMAT_A8, unsigned(w), unsigned(w * h), entry.pixels.get());
        cached = &entry; break;
    }
    if (!cached) { Canvas::sprite(x, y, rows, height, scale, paint, feet); return; }
    lv_draw_image_dsc_t style; lv_draw_image_dsc_init(&style);
    style.src = &cached->image; style.recolor = lv_color_hex(paint); style.recolor_opa = LV_OPA_COVER;
    lv_area_t area{x, y, x + int(cached->image.header.w) - 1, y + int(cached->image.header.h) - 1};
    lv_draw_image(layer, &style, &area);
}
void LvglCanvas::checkerboard(int x, int y, int columns, int rows, int cell,
                             uint32_t first, uint32_t second) {
    // One bounded background cache, reused for every Snake frame and game.
    if (!board.pixels && columns == 32 && rows == 22 && cell == 24) {
        int w = columns * cell, h = rows * cell;
        board.pixels.reset(new (std::nothrow) uint8_t[w * h * 2]);
        if (board.pixels) {
            board.columns = columns; board.rows = rows; board.cell = cell;
            board.first = first; board.second = second;
            uint16_t colors[] = {lv_color_to_u16(lv_color_hex(first)), lv_color_to_u16(lv_color_hex(second))};
            for (int row = 0; row < h; ++row) for (int col = 0; col < w; ++col) {
                uint16_t pixel = colors[((row / cell) + (col / cell)) % 2];
                std::memcpy(board.pixels.get() + (row * w + col) * 2, &pixel, 2);
            }
            header(board.image, w, h, LV_COLOR_FORMAT_RGB565, unsigned(w * 2), unsigned(w * h * 2), board.pixels.get());
        }
    }
    if (!board.pixels || board.columns != columns || board.rows != rows || board.cell != cell ||
        board.first != first || board.second != second) {
        Canvas::checkerboard(x, y, columns, rows, cell, first, second); return;
    }
    lv_draw_image_dsc_t style; lv_draw_image_dsc_init(&style); style.src = &board.image;
    lv_area_t area{x, y, x + columns * cell - 1, y + rows * cell - 1};
    lv_draw_image(layer, &style, &area);
}

void LvglCanvas::dot_row(int x, int y, uint32_t mask, unsigned columns, int pitch, uint32_t paint) {
    if (!mask) return;
    if (!columns || columns > 32 || pitch < 8 || pitch > 32) return;
    DotStrip *cached = nullptr;
    for (auto &entry : strips)
        if (entry.pixels && entry.x == x && entry.y == y && entry.columns == columns && entry.pitch == pitch) { cached = &entry; break; }
    if (!cached) for (auto &entry : strips) if (!entry.pixels) {
        auto width = columns * unsigned(pitch);
        entry.pixels.reset(new (std::nothrow) uint8_t[width * 4]{});
        if (!entry.pixels) break;
        entry.x = x; entry.y = y; entry.columns = columns; entry.pitch = pitch;
        header(entry.image, int(width), 4, LV_COLOR_FORMAT_A8, width, width * 4, entry.pixels.get());
        cached = &entry; break;
    }
    if (!cached) { Canvas::dot_row(x, y, mask, columns, pitch, paint); return; }
    if (cached->mask != mask) {
        const unsigned width = columns * unsigned(pitch);
        std::memset(cached->pixels.get(), 0, width * 4);
        for (unsigned col = 0; col < columns; ++col) if (mask & (uint32_t(1) << col))
            for (unsigned row = 0; row < 4; ++row)
                std::memset(cached->pixels.get() + row * width + col * unsigned(pitch) + unsigned(pitch / 2 - 2), 255, 4);
        cached->mask = mask;
    }
    lv_draw_image_dsc_t style; lv_draw_image_dsc_init(&style);
    style.src = &cached->image; style.recolor = lv_color_hex(paint); style.recolor_opa = LV_OPA_COVER;
    lv_area_t area{x, y, x + int(columns) * pitch - 1, y + 3};
    lv_draw_image(layer, &style, &area);
}
void LvglCanvas::line(int x1, int y1, int x2, int y2, uint32_t paint, int width) {
    if (width < 1) return;
    lv_draw_line_dsc_t style; lv_draw_line_dsc_init(&style);
    style.color = lv_color_hex(paint); style.width = width;
    style.p1.x = x1; style.p1.y = y1; style.p2.x = x2; style.p2.y = y2;
    lv_draw_line(layer, &style);
}
void LvglCanvas::rect(int x, int y, int width, int height, uint32_t color, int radius,
                      uint8_t opacity) {
    if (width <= 0 || height <= 0)
        return;
    lv_draw_rect_dsc_t style;
    lv_draw_rect_dsc_init(&style);
    style.bg_color = lv_color_hex(color);
    style.radius = radius;
    style.bg_opa = opacity;
    lv_area_t area = {x, y, x + width - 1, y + height - 1};
    lv_draw_rect(layer, &style, &area);
}
void LvglCanvas::text(int x, int y, const char *text, uint32_t color, Font font) {
    lv_draw_label_dsc_t style;
    lv_draw_label_dsc_init(&style);
    style.font = font == Font::Title   ? &lv_font_montserrat_28
                 : font == Font::Small ? &lv_font_montserrat_16
                 : font == Font::Pixel ? &lv_font_unscii_16
                                       : &lv_font_montserrat_22;
    style.text = text;
    style.text_local = 1;
    style.color = lv_color_hex(color);
    lv_area_t area = {x, y, 1279, 719};
    lv_draw_label(layer, &style, &area);
}
} // namespace arcade
