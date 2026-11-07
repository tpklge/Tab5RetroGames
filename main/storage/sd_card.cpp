#include "sd_card.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "esp_log.h"
namespace arcade {
namespace {
esp_err_t last_error = ESP_OK;
const char *last_stage = "";
void failed(const char *stage, esp_err_t error) {
    last_stage = stage;
    last_error = error;
    ESP_LOGW("storage", "microSD %s: %s", stage, esp_err_to_name(error));
}
}
std::string sd_error() {
    return last_error == ESP_OK ? std::string{} :
           std::string("microSD: ") + last_stage + " / " + esp_err_to_name(last_error);
}
bool mount_sd() {
    static sdmmc_card_t *card = nullptr;
    static sd_pwr_ctrl_handle_t power = nullptr;
    if (card) return true;
    // M5Stack's Tab5 reference uses slot 0, not SDMMC_HOST_DEFAULT's slot 1,
    // and explicitly powers the SDMMC I/O domain through LDO channel 4.
    if (!power) {
        sd_pwr_ctrl_ldo_config_t config{};
        config.ldo_chan_id = 4;
        auto error = sd_pwr_ctrl_new_on_chip_ldo(&config, &power);
        if (error != ESP_OK) { power = nullptr; failed("alimentacao LDO4", error); return false; }
    }
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
    host.pwr_ctrl_handle = power;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    slot.width = 4;
    slot.clk = GPIO_NUM_43;
    slot.cmd = GPIO_NUM_44;
    slot.d0 = GPIO_NUM_39;
    slot.d1 = GPIO_NUM_40;
    slot.d2 = GPIO_NUM_41;
    slot.d3 = GPIO_NUM_42;
    esp_vfs_fat_sdmmc_mount_config_t config{};
    config.format_if_mount_failed = false;
    config.max_files = 4;
    config.allocation_unit_size = 16 * 1024;
    auto error = esp_vfs_fat_sdmmc_mount("/arcadesd", &host, &slot, &config, &card);
    if (error == ESP_ERR_TIMEOUT || error == ESP_ERR_INVALID_RESPONSE || error == ESP_ERR_INVALID_CRC) {
        // The failed mount releases its host/slot resources in ESP-IDF.
        // A slower single data line can recover communication failures.
        card = nullptr;
        host.max_freq_khz = 10000;
        slot.width = 1;
        error = esp_vfs_fat_sdmmc_mount("/arcadesd", &host, &slot, &config, &card);
    }
    if (error != ESP_OK) { card = nullptr; failed("montagem FAT/SDMMC0", error); return false; }
    last_error = ESP_OK;
    last_stage = "";
    return true;
}
}
