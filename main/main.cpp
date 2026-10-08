#include "display_manager.h"
#include "keyboard_manager.h"
#include "audio_manager.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lvgl_port.h"
#include "application.h"
#include "nvs_backend.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "sd_backup.h"

static arcade::NvsBackend storage;
static arcade::Application application(storage, [] {
    return arcade::backup_to_sd(storage, unsigned(arcade::registry().size()));
});
extern "C" void app_main() {
    storage.init();
    ESP_ERROR_CHECK(arcade::display::init());
    arcade::keyboard::init();
    arcade::audio::init();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(arcade::keyboard::cleanup));
    ESP_ERROR_CHECK(esp_register_shutdown_handler(arcade::audio::cleanup));
    lvgl_port_lock(0);
    application.init();
    lv_timer_create([](lv_timer_t *) { application.tick(uint64_t(esp_timer_get_time())); }, 8,
                    nullptr);
    lvgl_port_unlock();
}
