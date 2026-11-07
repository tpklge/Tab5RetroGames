#pragma once
#include "file_backend.h"
#include "nvs_backend.h"
namespace arcade {
class StorageBackend : public Backend {
  public:
    void init();
    bool read(const std::string &key, std::vector<uint8_t> &bytes) override {
        return active && active->read(key, bytes);
    }
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override {
        return active && active->write(key, bytes);
    }
    bool erase(const std::string &key) override { return active && active->erase(key); }
    bool prepare_write() override {
        if (!active) init(); // Select before Journal reads the existing generations.
        return active != nullptr;
    }
    std::string description() const override;
    std::string error() const override;
  private:
    FileBackend sd;
    NvsBackend nvs;
    Backend *active = nullptr;
    std::string failure;
};
}
