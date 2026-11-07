#pragma once
#include "journal.h"
namespace arcade {
// Portable file I/O; the platform mounts the card before init().
class FileBackend : public Backend {
  public:
    bool init(const std::string &directory);
    bool read(const std::string &key, std::vector<uint8_t> &bytes) override;
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override;
    bool erase(const std::string &key) override;
    std::string error() const override { return failure; }
    bool available() const { return !directory.empty(); }
    std::string description() const override { return "microSD /pipoca5"; }
  private:
    std::string directory, failure;
    void failed(const char *stage);
    std::string path(const std::string &key, bool temporary = false) const;
};
}
