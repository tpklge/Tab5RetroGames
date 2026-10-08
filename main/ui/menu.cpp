#include "menu.h"
#include "canvas.h"
#include "audio_manager.h"
#include <algorithm>
namespace arcade {
namespace {
void icon(lv_obj_t *parent, int game) {
    const uint32_t colors[3] = {0xb469f5, 0x48e895, 0xff5367};
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
        lv_obj_set_style_bg_color(pixel, lv_color_hex(colors[game % 3]), 0);
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
    lv_obj_set_style_bg_color(screen, lv_color_hex(backgrounds[theme % 3]), 0);
    container = lv_obj_create(screen);
    lv_obj_set_size(container, 1160, 650);
    lv_obj_set_pos(container, 60, 30);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 12, 0);
    lv_obj_set_style_pad_row(container, 10, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    auto *heading = lv_label_create(container);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(heading, lv_color_hex(color::Gold), 0);
    auto *details = lv_label_create(container);
    lv_label_set_text(details, description.c_str());
    lv_obj_set_width(details, 1100);
    lv_obj_set_style_text_color(details, lv_color_hex(color::Muted), 0);
    auto *list = lv_obj_create(container);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 8, 0);
    lv_obj_set_style_pad_all(list, 4, 0);
    for (unsigned index = 0; index < items.size(); ++index) {
        auto *button = lv_button_create(list);
        buttons.push_back(button);
        lv_obj_set_size(button, lv_pct(100), items[index].icon >= 0 ? 100 : 58);
        lv_obj_set_style_radius(button, 8, 0);
        lv_obj_set_style_shadow_width(button, 0, 0);
        auto *label = lv_label_create(button);
        lv_label_set_text(label, items[index].label.c_str());
        lv_obj_set_width(label, items[index].icon >= 0 ? 975 : 1040);
        lv_obj_set_pos(label, items[index].icon >= 0 ? 94 : 12, 10);
        lv_obj_set_style_text_color(label, lv_color_hex(color::Text), 0);
        if (items[index].icon >= 0)
            icon(button, items[index].icon);
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
        lv_obj_set_style_border_width(buttons[i], i == selected ? 2 : 0, 0);
        lv_obj_set_style_border_color(buttons[i], lv_color_hex(color::Cyan), 0);
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
