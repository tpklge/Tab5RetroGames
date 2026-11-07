#include "menu.h"
#include "canvas.h"
#include "audio_manager.h"
#include <algorithm>
namespace arcade {
namespace {
void icon(lv_obj_t *parent, int game) {
    if (game == 10) {
        constexpr uint16_t rows[] = {0x010,0x038,0x028,0x06c,0x044,0x0c6,0x1ff,0x010};
        for (int r = 0; r < 8; ++r) for (int col = 0; col < 9; ++col) if (rows[r] & (1 << col)) {
            auto *pixel = lv_obj_create(parent); lv_obj_set_size(pixel, 6, 6);
            lv_obj_set_pos(pixel, 10 + (8-col)*6, 14+r*6);
            lv_obj_set_style_bg_color(pixel, lv_color_hex(0x8ee5d0), 0);
            lv_obj_set_style_border_width(pixel, 0, 0); lv_obj_set_style_radius(pixel, 0, 0);
            lv_obj_remove_flag(pixel, LV_OBJ_FLAG_CLICKABLE);
        }
        return;
    }
    if (game == 8 || game == 9) {
        constexpr uint16_t bomb[] = {0x060, 0x0a0, 0x078, 0x0fc, 0x1fe, 0x1fe, 0x0fc, 0x078};
        constexpr uint16_t box[] = {0x1fe, 0x102, 0x16a, 0x13a, 0x13a, 0x16a, 0x102, 0x1fe};
        const auto *rows = game == 8 ? bomb : box;
        for (int r = 0; r < 8; ++r) for (int col = 0; col < 9; ++col) if (rows[r] & (1 << col)) {
            auto *pixel = lv_obj_create(parent);
            lv_obj_set_size(pixel, 6, 6); lv_obj_set_pos(pixel, 10 + (8 - col) * 6, 14 + r * 6);
            lv_obj_set_style_bg_color(pixel, lv_color_hex(game == 8 ? 0xffb46d : 0xd5a066), 0);
            lv_obj_set_style_border_width(pixel, 0, 0); lv_obj_set_style_radius(pixel, 0, 0);
            lv_obj_remove_flag(pixel, LV_OBJ_FLAG_CLICKABLE);
        }
        return;
    }
    if (game == 6 || game == 7) {
        constexpr uint16_t frog[] = {0x066, 0x0ff, 0x199, 0x1ff, 0x0ff, 0x0ff, 0x1b6, 0x132};
        constexpr uint16_t maze[] = {0x03c, 0x07e, 0x0fc, 0x0f8, 0x0f0, 0x0f8, 0x07e, 0x03c};
        const auto *rows = game == 6 ? frog : maze;
        for (int r = 0; r < 8; ++r) for (int col = 0; col < 9; ++col) if (rows[r] & (1 << col)) {
            auto *pixel = lv_obj_create(parent);
            lv_obj_set_size(pixel, 6, 6); lv_obj_set_pos(pixel, 10 + (8 - col) * 6, 14 + r * 6);
            lv_obj_set_style_bg_color(pixel, lv_color_hex(game == 6 ? 0x8af58b : 0xffd166), 0);
            lv_obj_set_style_border_width(pixel, 0, 0); lv_obj_set_style_radius(pixel, 0, 0);
            lv_obj_remove_flag(pixel, LV_OBJ_FLAG_CLICKABLE);
        }
        return;
    }
    if (game == 5) {
        for (int i = 0; i < 8; ++i) {
            auto *shape = lv_obj_create(parent);
            lv_obj_set_style_border_width(shape, 0, 0);
            lv_obj_set_style_radius(shape, 2, 0);
            lv_obj_remove_flag(shape, LV_OBJ_FLAG_CLICKABLE);
            if (i < 6) {
                lv_obj_set_size(shape, 17, 9); lv_obj_set_pos(shape, 9 + (i % 3) * 19, 12 + (i / 3) * 12);
            } else if (i == 6) {
                lv_obj_set_size(shape, 38, 7); lv_obj_set_pos(shape, 18, 56);
            } else {
                lv_obj_set_size(shape, 9, 9); lv_obj_set_pos(shape, 46, 39);
            }
            lv_obj_set_style_bg_color(shape, lv_color_hex(i < 3 ? 0xff6b8a : i < 6 ? 0xffd166 : i == 6 ? 0x27d9ef : 0xfff2cd), 0);
        }
        return;
    }
    if (game == 3 || game == 4) {
        // Original bitmap icons: an invader and a right-facing starfighter.
        constexpr uint16_t alien[] = {0x084, 0x048, 0x0fc, 0x1b6, 0x1fe, 0x12a, 0x084, 0x132};
        constexpr uint16_t ship[] = {0x00c, 0x018, 0x13c, 0x1fe, 0x3ff, 0x1fe, 0x13c, 0x018};
        const auto *rows = game == 3 ? alien : ship;
        for (int r = 0; r < 8; ++r) for (int col = 0; col < 10; ++col)
            if (rows[r] & (1 << col)) {
                auto *pixel = lv_obj_create(parent);
                lv_obj_set_size(pixel, 6, 6);
                lv_obj_set_pos(pixel, 10 + (9 - col) * 6, 15 + r * 6);
                lv_obj_set_style_bg_color(pixel, lv_color_hex(game == 3 ? 0x27d9ef : 0xffb454), 0);
                lv_obj_set_style_border_width(pixel, 0, 0);
                lv_obj_set_style_radius(pixel, 0, 0);
                lv_obj_remove_flag(pixel, LV_OBJ_FLAG_CLICKABLE);
            }
        return;
    }
    const uint32_t colors[4] = {0xb469f5, 0x48e895, 0xff5367, 0x27d9ef};
    for (int i = 0; i < 4; ++i) {
        auto *pixel = lv_obj_create(parent);
        lv_obj_set_size(pixel, 18, 18);
        lv_obj_set_pos(pixel,
                       12 + (game == 0   ? (i == 0 ? 20 : i % 3 * 20)
                             : game == 1 ? i * 15
                                         : (i % 2) * 20),
                       12 + (game == 0   ? (i == 0 ? 0 : 20)
                             : game == 1 ? i % 2 * 16
                                         : i / 2 * 20));
        lv_obj_set_style_bg_color(pixel, lv_color_hex(colors[game % 4]), 0);
        lv_obj_set_style_border_width(pixel, 0, 0);
        lv_obj_set_style_radius(pixel, game == 1 ? 8 : 3, 0);
        lv_obj_remove_flag(pixel, LV_OBJ_FLAG_CLICKABLE);
    }
}
} // namespace
void Menu::close() {
    if (container)
        lv_obj_delete(container);
    container = nullptr;
    buttons.clear();
    items.clear();
}
void Menu::show(const char *title, const std::string &description, std::vector<MenuItem> next,
                unsigned choice, uint8_t theme) {
    close();
    items = std::move(next);
    const uint32_t backgrounds[3] = {color::Bg, 0x210e35, 0x10181b};
    auto *screen = lv_screen_active();
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_scroll_to(screen, 0, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(screen, lv_color_hex(backgrounds[theme % 3]), 0);
    container = lv_obj_create(screen);
    lv_obj_set_size(container, 1080, 668);
    lv_obj_set_pos(container, 100, 26);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x111c30), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(container, lv_color_hex(0x2b405f), 0);
    lv_obj_set_style_radius(container, 16, 0);
    lv_obj_set_style_border_width(container, 1, 0);
    lv_obj_set_style_pad_all(container, 22, 0);
    lv_obj_set_style_pad_row(container, 10, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    auto *heading = lv_label_create(container);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(heading, lv_color_hex(color::Gold), 0);
    auto *details = lv_label_create(container);
    lv_label_set_text(details, description.c_str());
    if (description.empty()) lv_obj_add_flag(details, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(details, lv_pct(100));
    lv_obj_set_style_text_font(details, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_line_space(details, 4, 0);
    lv_obj_set_style_text_color(details, lv_color_hex(color::Muted), 0);
    auto *list = lv_obj_create(container);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_set_style_pad_all(list, 4, 0);
    for (unsigned index = 0; index < items.size(); ++index) {
        auto *button = lv_button_create(list);
        buttons.push_back(button);
        const bool card = items[index].icon >= 0;
        lv_obj_set_size(button, lv_pct(100), card ? 74 : LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(button, card ? 74 : 50, 0);
        lv_obj_set_style_pad_all(button, 0, 0);
        if (!card) {
            lv_obj_set_style_pad_top(button, 11, 0);
            lv_obj_set_style_pad_bottom(button, 11, 0);
        }
        lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(button, 8, 0);
        lv_obj_set_style_shadow_width(button, 0, 0);
        const auto split = items[index].label.find('\n');
        auto *label = lv_label_create(button);
        const std::string name = card ? items[index].label.substr(0, split) : items[index].label;
        lv_label_set_text(label, name.c_str());
        lv_obj_set_width(label, card ? 880 : 980);
        lv_obj_set_pos(label, card ? 88 : 16, card ? 11 : 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_22, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(color::Text), 0);
        if (card) {
            icon(button, items[index].icon);
            if (split != std::string::npos) {
                auto *subtitle = lv_label_create(button);
                lv_label_set_text(subtitle, items[index].label.c_str() + split + 1);
                lv_obj_set_width(subtitle, 880);
                lv_label_set_long_mode(subtitle, LV_LABEL_LONG_DOT);
                lv_obj_set_height(subtitle, 20);
                lv_obj_set_pos(subtitle, 88, 42);
                lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_16, 0);
                lv_obj_set_style_text_color(subtitle, lv_color_hex(0xb7c6dd), 0);
            }
        }
        lv_obj_set_user_data(button, this);
        lv_obj_add_event_cb(
            button,
            [](lv_event_t *event) {
                auto *button = static_cast<lv_obj_t *>(lv_event_get_target(event));
                auto *menu = static_cast<Menu *>(lv_obj_get_user_data(button));
                unsigned index =
                    unsigned(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
                menu->select(index);
                menu->activate();
            },
            LV_EVENT_CLICKED, reinterpret_cast<void *>(uintptr_t(index)));
    }
    auto *hint = lv_label_create(container);
    lv_label_set_text(hint, "Setas: navegar / ajustar    Enter: confirmar    Esc: voltar");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(color::Muted), 0);
    select(std::min(choice, count() ? count() - 1 : 0));
}
void Menu::select(unsigned index) {
    if (index >= count())
        return;
    selected = index;
    for (unsigned i = 0; i < buttons.size(); ++i) {
        lv_obj_set_style_bg_color(buttons[i], lv_color_hex(i == selected ? 0x273d62 : color::Panel),
                                  0);
        lv_obj_set_style_border_width(buttons[i], 1, 0);
        lv_obj_set_style_border_color(buttons[i], lv_color_hex(i == selected ? color::Cyan : 0x2b3c56), 0);
    }
    lv_obj_scroll_to_view(buttons[selected], LV_ANIM_ON);
}
void Menu::activate() {
    if (selected < count() && items[selected].activate) {
        auto action = items[selected].activate;
        audio::play(audio::Effect::Confirm);
        action();
    }
}
void Menu::adjust(int direction) {
    if (selected < count() && items[selected].adjust) {
        auto action = items[selected].adjust;
        action(direction);
    }
}
} // namespace arcade
