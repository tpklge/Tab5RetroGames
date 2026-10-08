#pragma once
#include "journal.h"
#include "nvs.h"
namespace arcade {
class NvsBackend : public Backend {
  public:
    bool init();
    bool read(const std::string &key, std::vector<uint8_t> &bytes) override;
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override;
    bool erase(const std::string &key) override;
    bool available() const { return handle != 0; }
    size_t free_entries() const;

  private:
    nvs_handle_t handle = 0;
};
} // namespace arcade
