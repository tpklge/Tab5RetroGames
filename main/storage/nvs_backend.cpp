#include "nvs_backend.h"
#include "nvs_flash.h"
#include "esp_log.h"
namespace arcade {
bool NvsBackend::init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_OK)
        err = nvs_open("pipoca5", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        handle = 0;
        ESP_LOGW("storage", "NVS unavailable, never erased: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}
bool NvsBackend::read(const std::string &key, std::vector<uint8_t> &bytes) {
    if (!handle)
        return false;
    size_t size = 0;
    if (nvs_get_blob(handle, key.c_str(), nullptr, &size) != ESP_OK || size > 65560)
        return false;
    bytes.resize(size);
    return nvs_get_blob(handle, key.c_str(), bytes.data(), &size) == ESP_OK;
}
bool NvsBackend::write(const std::string &key, const std::vector<uint8_t> &bytes) {
    if (!handle)
        return false;
    esp_err_t err = nvs_set_blob(handle, key.c_str(), bytes.data(), bytes.size());
    if (err == ESP_OK)
        err = nvs_commit(handle);
    if (err != ESP_OK)
        ESP_LOGW("storage", "Write failed (%s): %s", key.c_str(), esp_err_to_name(err));
    return err == ESP_OK;
}
bool NvsBackend::erase(const std::string &key) {
    if (!handle)
        return false;
    esp_err_t err = nvs_erase_key(handle, key.c_str());
    return (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) && nvs_commit(handle) == ESP_OK;
}
size_t NvsBackend::free_entries() const {
    nvs_stats_t stats{};
    return nvs_get_stats(nullptr, &stats) == ESP_OK ? stats.free_entries : 0;
}
} // namespace arcade
