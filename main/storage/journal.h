#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace arcade {
class Backend {
  public:
    virtual ~Backend() = default;
    virtual bool read(const std::string &key, std::vector<uint8_t> &bytes) = 0;
    virtual bool write(const std::string &key, const std::vector<uint8_t> &bytes) = 0;
    virtual bool erase(const std::string &key) = 0;
};
uint32_t crc32(const uint8_t *bytes, size_t length, uint32_t previous = 0);
class Journal {
  public:
    explicit Journal(Backend &backend) : backend(backend) {}
    bool read(const std::string &key, uint32_t kind, std::vector<uint8_t> &payload);
    bool write(const std::string &key, uint32_t kind, const std::vector<uint8_t> &payload);
    bool erase(const std::string &key);

  private:
    Backend &backend;
    bool slot(const std::string &key, uint32_t kind, uint32_t &generation,
              std::vector<uint8_t> &payload);
};
} // namespace arcade
