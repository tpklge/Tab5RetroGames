#include "lvgl_canvas.h"
namespace arcade {
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
