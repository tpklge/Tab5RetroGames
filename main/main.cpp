#include "display_manager.h"
#include "keyboard_manager.h"
#include "audio_manager.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lvgl_port.h"
#include "application.h"
#include "storage_backend.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "sd_backup.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static arcade::StorageBackend storage;
static arcade::Application application(storage, [] {
    return arcade::backup_to_sd(storage, unsigned(arcade::registry().size()));
});
extern "C" void app_main() {
    ESP_ERROR_CHECK(arcade::display::init());
    storage.init();
    arcade::keyboard::init();
    arcade::audio::init();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(arcade::keyboard::cleanup));
    ESP_ERROR_CHECK(esp_register_shutdown_handler(arcade::audio::cleanup));
    lvgl_port_lock(0);
    application.init();
    application.independent_updates(true);
    const auto created = xTaskCreatePinnedToCore([](void *) {
        TickType_t wake = xTaskGetTickCount();
        for (;;) {
            application.poll_game(uint64_t(esp_timer_get_time()));
            vTaskDelayUntil(&wake, pdMS_TO_TICKS(4));
        }
    }, "arcade_rules", 8192, nullptr, 5, nullptr, 0);
    ESP_ERROR_CHECK(created == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
    lv_timer_create([](lv_timer_t *) { application.tick(uint64_t(esp_timer_get_time())); }, 8,
                    nullptr);
    lvgl_port_unlock();
}
