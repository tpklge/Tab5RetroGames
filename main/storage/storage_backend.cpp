#include "storage_backend.h"
#include "sd_card.h"
#include "esp_log.h"
namespace arcade {
void StorageBackend::init() {
    if (active) return; // Never switch an established store or mix save generations.
    const bool mounted = mount_sd();
    const bool files = mounted && sd.init(SdDirectory);
    failure = files ? std::string{} : mounted ? sd.error() : sd_error();
    if (files) active = &sd;
    else if (nvs.init()) active = &nvs;
    ESP_LOGI("storage", "%s", description().c_str());
}
std::string StorageBackend::description() const {
    return active ? active->description() : "Sem armazenamento disponivel";
}
std::string StorageBackend::error() const {
    return active ? active->error() : failure;
}
}
