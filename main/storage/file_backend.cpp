#include "file_backend.h"
#include "file_names.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
namespace arcade {
namespace {
constexpr size_t MaxRecordBytes = 65560;
bool remove_if_present(const std::string &path) {
    return std::remove(path.c_str()) == 0 || errno == ENOENT;
}
}
void FileBackend::failed(const char *stage) {
    const int code = errno ? errno : EIO;
    failure = std::string("microSD: ") + stage + " / " + std::strerror(code) + " (" + std::to_string(code) + ")";
}
bool FileBackend::init(const std::string &root) {
    directory.clear(); failure.clear();
    if (root.empty()) { failure = "Pasta microSD invalida"; return false; }
    if (mkdir(root.c_str(), 0755) != 0 && errno != EEXIST) { failed("criar /pipoca5"); return false; }
    struct stat info{};
    if (stat(root.c_str(), &info) != 0) { failed("acessar /pipoca5"); return false; }
    if (!S_ISDIR(info.st_mode)) { failure = "/pipoca5 existe, mas nao e uma pasta"; return false; }
    directory = root;
    return true;
}
std::string FileBackend::path(const std::string &key, bool temporary) const {
    const auto filename = record_filename(key, temporary);
    return directory.empty() || filename.empty() ? std::string{} : directory + "/" + filename;
}
bool FileBackend::read(const std::string &key, std::vector<uint8_t> &bytes) {
    const auto filename = path(key);
    if (filename.empty()) return false;
    struct stat info{};
    if (stat(filename.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0 ||
        uint64_t(info.st_size) > MaxRecordBytes) return false;
    FILE *file = std::fopen(filename.c_str(), "rb");
    if (!file) return false;
    std::vector<uint8_t> loaded(size_t(info.st_size));
    bool ok = std::fread(loaded.data(), 1, loaded.size(), file) == loaded.size();
    if (ok) ok = std::fgetc(file) == EOF && !std::ferror(file);
    if (std::fclose(file) != 0) ok = false;
    if (ok) bytes = std::move(loaded);
    return ok;
}
bool FileBackend::write(const std::string &key, const std::vector<uint8_t> &bytes) {
    failure.clear();
    const auto filename = path(key);
    if (filename.empty() || bytes.size() > MaxRecordBytes) {
        failure = "microSD: registro ou pasta invalida"; return false;
    }
    const auto temporary = path(key, true);
    FILE *file = std::fopen(temporary.c_str(), "wb");
    if (!file) { failed("abrir arquivo de save"); return false; }
    bool ok = std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
    if (!ok) failed("escrever save");
    if (std::fflush(file) != 0) { if (ok) failed("flush do save"); ok = false; }
    if (fsync(fileno(file)) != 0) { if (ok) failed("sincronizar save"); ok = false; }
    if (std::fclose(file) != 0) { if (ok) failed("fechar save"); ok = false; }
    // Journal writes the older slot; the other CRC-valid generation survives.
    if (ok && !remove_if_present(filename)) { failed("substituir save antigo"); ok = false; }
    if (ok && std::rename(temporary.c_str(), filename.c_str()) != 0) { failed("renomear save"); ok = false; }
    if (!ok) std::remove(temporary.c_str());
    return ok;
}
bool FileBackend::erase(const std::string &key) {
    const auto filename = path(key);
    if (filename.empty()) return false;
    const bool temporary = remove_if_present(path(key, true));
    return remove_if_present(filename) && temporary;
}
}
