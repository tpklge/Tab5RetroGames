#include "display_manager.h"
#include "lvgl_port.h"
#include "driver/ledc.h"
#include "esp_lcd_touch.h"
#include "esp_log.h"
#include <sys/time.h>
#include <ctime>
namespace arcade::display {
namespace {
m5::tab5::m5tab5_component hardware;
esp_lcd_touch_handle_t touch;
uint32_t rendered_frames = 0;
void read_touch(lv_indev_t *, lv_indev_data_t *sample) {
    esp_lcd_touch_point_data_t point{};
    uint8_t count = 0;
    if (esp_lcd_touch_read_data(touch) == ESP_OK &&
        esp_lcd_touch_get_data(touch, &point, &count, 1) == ESP_OK && count) {
        sample->point = {int32_t(1279 - point.y), int32_t(point.x)};
        sample->state = LV_INDEV_STATE_PRESSED;
    } else
        sample->state = LV_INDEV_STATE_RELEASED;
}
} // namespace
m5::tab5::m5tab5_component &board() { return hardware; }
uint32_t refreshes() { return rendered_frames; }
esp_err_t init() {
    esp_err_t err = hardware.begin();
    if (err != ESP_OK)
        return err;
    m5::tab5::m5tab5_rtc_datetime_t clock;
    if (hardware.rtc_init() == ESP_OK && !hardware.rtc_volt_low() &&
        hardware.rtc_get_datetime(&clock) == ESP_OK && clock.date.year >= 2026) {
        auto utc = clock.to_tm();
        // ESP-IDF starts with UTC; import valid existing RTC time without network.
        timeval value{std::mktime(&utc), 0};
        if (value.tv_sec > 0)
            settimeofday(&value, nullptr);
    }
    lvgl_port_cfg_t port = lvgl_PORT_INIT_CONFIG();
    port.task_stack = 16384;
    port.task_affinity = 1;
    port.timer_period_ms = 5;
    err = lvgl_port_init(&port);
    if (err != ESP_OK)
        return err;
    lvgl_disp_cfg_t config{};
    config.panel_handle = hardware.lcd_panel();
    config.buffer_size = 720 * 1280;
    config.double_buffer = true;
    config.hres = 720;
    config.vres = 1280;
    config.color_format = LV_COLOR_FORMAT_RGB565;
    config.flags.sw_rotate = 1;
    config.flags.direct_mode = 1;
    lvgl_disp_dsi_cfg_t dsi{};
    dsi.sw_rotation = LV_DISPLAY_ROTATION_90;
    dsi.flags.avoid_tearing = 1;
    dsi.flags.use_ppa = 1;
    lv_display_t *panel = lvgl_port_add_disp_dsi(&config, &dsi);
    if (!panel)
        return ESP_FAIL;
    if (!lvgl_port_lock(0))
        return ESP_FAIL;
    lv_display_add_event_cb(
        panel, [](lv_event_t *) { ++rendered_frames; }, LV_EVENT_RENDER_READY, nullptr);
    touch = hardware.touch_panel();
    if (touch) {
        lv_indev_t *input = lv_indev_create();
        if (!input) {
            lvgl_port_unlock();
            return ESP_ERR_NO_MEM;
        }
        lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(input, read_touch);
        lv_indev_set_display(input, panel);
    }
    lvgl_port_unlock();
    brightness(75);
    return ESP_OK;
}
void brightness(uint8_t percent) {
    if (percent > 100)
        percent = 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, (4095u * percent) / 100u);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}
} // namespace arcade::display
