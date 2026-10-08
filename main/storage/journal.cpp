#include "journal.h"
#include <algorithm>
namespace arcade {
uint32_t crc32(const uint8_t *bytes, size_t length, uint32_t previous) {
    uint32_t value = ~previous;
    for (size_t i = 0; i < length; ++i) {
        value ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit)
            value = (value >> 1) ^ (0xedb88320u & uint32_t(-int32_t(value & 1)));
    }
    return ~value;
}
namespace {
constexpr uint32_t magic = 0x35504341, version = 1;
void append(std::vector<uint8_t> &bytes, uint32_t value) {
    for (int i = 0; i < 4; ++i)
        bytes.push_back(uint8_t(value >> (i * 8)));
}
uint32_t word(const std::vector<uint8_t> &bytes, size_t offset) {
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i)
        value |= uint32_t(bytes[offset + i]) << (i * 8);
    return value;
}
bool newer(uint32_t a, uint32_t b) { return int32_t(a - b) > 0; }
} // namespace
bool Journal::slot(const std::string &key, uint32_t kind, uint32_t &generation,
                   std::vector<uint8_t> &payload) {
    std::vector<uint8_t> bytes;
    if (!backend.read(key, bytes) || bytes.size() < 24 || bytes.size() > 65560 ||
        word(bytes, 0) != magic || word(bytes, 4) != version || word(bytes, 8) != kind ||
        word(bytes, 16) != bytes.size() - 24)
        return false;
    uint32_t crc = crc32(bytes.data(), 20);
    crc = crc32(bytes.data() + 24, bytes.size() - 24, crc);
    if (crc != word(bytes, 20))
        return false;
    generation = word(bytes, 12);
    payload.assign(bytes.begin() + 24, bytes.end());
    return true;
}
bool Journal::read(const std::string &key, uint32_t kind, std::vector<uint8_t> &payload) {
    uint32_t a = 0, b = 0;
    std::vector<uint8_t> first, second;
    bool have_a = slot(key + "a", kind, a, first), have_b = slot(key + "b", kind, b, second);
    if (!have_a && !have_b)
        return false;
    payload = have_a && (!have_b || newer(a, b)) ? std::move(first) : std::move(second);
    return true;
}
bool Journal::write(const std::string &key, uint32_t kind, const std::vector<uint8_t> &payload) {
    if (payload.size() > 65536 || key.size() > 14)
        return false;
    uint32_t a = 0, b = 0;
    std::vector<uint8_t> first, second;
    bool have_a = slot(key + "a", kind, a, first), have_b = slot(key + "b", kind, b, second);
    bool use_a = !have_a || (have_b && !newer(a, b));
    uint32_t generation =
        (!have_a && !have_b) ? 1 : (have_a && (!have_b || newer(a, b)) ? a : b) + 1;
    std::vector<uint8_t> bytes;
    bytes.reserve(24 + payload.size());
    append(bytes, magic);
    append(bytes, version);
    append(bytes, kind);
    append(bytes, generation);
    append(bytes, uint32_t(payload.size()));
    uint32_t crc = crc32(bytes.data(), bytes.size());
    crc = crc32(payload.data(), payload.size(), crc);
    append(bytes, crc);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    return backend.write(key + (use_a ? "a" : "b"), bytes);
}
bool Journal::erase(const std::string &key) {
    bool first = backend.erase(key + "a");
    return backend.erase(key + "b") && first;
}
} // namespace arcade
