#include "sd_card.h"
#include "esp_vfs_fat.h"
#include <cassert>
#include <cstring>
#include <cstdio>
static int power_calls = 0, mount_calls = 0;
static esp_err_t power_error = ESP_ERR_NO_MEM, mount_error = ESP_ERR_TIMEOUT;
static bool recover_single_line = false;
static int power_token;
static sdmmc_card_t card{};
const char *esp_err_to_name(esp_err_t error) {
    switch (error) {
        case ESP_OK: return "ESP_OK";
        case ESP_FAIL: return "ESP_FAIL";
        case ESP_ERR_NO_MEM: return "ESP_ERR_NO_MEM";
        case ESP_ERR_TIMEOUT: return "ESP_ERR_TIMEOUT";
        default: return "OTHER_ERROR";
    }
}
esp_err_t sd_pwr_ctrl_new_on_chip_ldo(const sd_pwr_ctrl_ldo_config_t *config, sd_pwr_ctrl_handle_t *handle) {
    ++power_calls;
    assert(config->ldo_chan_id == 4);
    if (power_error == ESP_OK) *handle = &power_token;
    return power_error;
}
esp_err_t esp_vfs_fat_sdmmc_mount(const char *path, const sdmmc_host_t *host,
        const sdmmc_slot_config_t *slot, const esp_vfs_fat_sdmmc_mount_config_t *config, sdmmc_card_t **out) {
    ++mount_calls;
    assert(std::strcmp(path, "/arcadesd") == 0);
    assert(host->slot == 0 && host->pwr_ctrl_handle == &power_token);
    assert(!config->format_if_mount_failed && config->max_files >= 3);
    assert(slot->clk == 43 && slot->cmd == 44 && slot->d0 == 39 && slot->d1 == 40 && slot->d2 == 41 && slot->d3 == 42);
    assert(slot->width == 4 || (slot->width == 1 && host->max_freq_khz == 10000));
    if (recover_single_line && slot->width == 1) { *out = &card; return ESP_OK; }
    return mount_error;
}
int main() {
    assert(!arcade::mount_sd() && mount_calls == 0 && power_calls == 1);
    assert(arcade::sd_error().find("LDO4") != std::string::npos);
    assert(arcade::sd_error().find("ESP_ERR_NO_MEM") != std::string::npos);
    power_error = ESP_OK;
    assert(!arcade::mount_sd() && power_calls == 2 && mount_calls == 2);
    assert(arcade::sd_error().find("ESP_ERR_TIMEOUT") != std::string::npos);
    mount_error = ESP_FAIL;
    assert(!arcade::mount_sd() && mount_calls == 3 && power_calls == 2);
    assert(arcade::sd_error().find("ESP_FAIL") != std::string::npos);
    mount_error = ESP_ERR_TIMEOUT; recover_single_line = true;
    assert(arcade::mount_sd() && mount_calls == 5 && arcade::sd_error().empty());
    assert(arcade::mount_sd() && mount_calls == 5 && power_calls == 2);
    puts("PASS SD mount: Tab5 slot 0, LDO4, pinout, no format, power/mount errors, retry, single-line fallback and shared mount");
}
