#include "file_backend.h"
#include "game.h"
#include "records.h"
#include "settings.h"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <unistd.h>
using namespace arcade;
int main() {
    char temporary[] = "/tmp/pipoca5-storage-XXXXXX";
    const char *root = mkdtemp(temporary); assert(root);
    const auto folder = std::string(root) + "/pipoca5";
    FileBackend files; assert(files.init(folder));
    Journal journal(files);
    std::vector<uint8_t> bytes;
    assert(!journal.read("save0", 3, bytes));
    assert(!files.write("../outside", {1}) && !files.write("", {1}));
    assert(!files.write("toolongfilename16", {1}));
    assert(!files.write("123456789", {1}));
    assert(!files.write("large", std::vector<uint8_t>(65561)));
    assert(journal.write("save0", 3, {1,2,3}));
    assert(journal.write("save0", 3, {4,5,6}));
    FileBackend reboot; assert(reboot.init(folder)); Journal restored(reboot);
    assert(restored.read("save0", 3, bytes) && bytes == std::vector<uint8_t>({4,5,6}));
    // Torn newest slot and leftover temporary file cannot replace the good slot.
    std::ofstream(folder + "/save0b.bin", std::ios::binary | std::ios::trunc) << "broken";
    std::ofstream(folder + "/save0a.tmp", std::ios::binary) << "unfinished";
    assert(restored.read("save0", 3, bytes) && bytes == std::vector<uint8_t>({1,2,3}));
    assert(restored.write("save0", 3, {7,8,9}));
    assert(restored.read("save0", 3, bytes) && bytes[0] == 7);
    // Failure replacing the older slot must preserve the newest valid save.
    std::filesystem::remove(folder + "/save0a.bin");
    std::filesystem::create_directory(folder + "/save0a.bin");
    std::ofstream(folder + "/save0a.bin/keep") << "block removal";
    assert(!restored.write("save0", 3, {99}));
    assert(reboot.error().find("substituir save antigo") != std::string::npos);
    assert(restored.read("save0", 3, bytes) && bytes[0] == 7);
    std::filesystem::remove_all(folder + "/save0a.bin");
    assert(restored.erase("save0") && !restored.read("save0", 3, bytes));
    assert(restored.erase("save0"));
    auto options = default_settings(); options.brightness = 55;
    assert(save_settings(restored, options));
    FileBackend final_boot; assert(final_boot.init(folder)); Journal final_journal(final_boot);
    Settings settings; assert(load_settings(final_journal, settings) && settings.brightness == 55);
    for (const auto &entry : registry()) {
        Game *game = entry.create(); game->init(Options{}); game->start();
        Controls controls; controls.held[size_t(Action::Right)] = 1;
        Game *frame = game->render_copy();
        const auto frame_before = frame->save();
        game->update(20, controls);
        assert(frame->save() == frame_before); // Rasterizer sees an immutable frame.
        game->pause();
        auto state = game->save();
        auto key = "save" + std::to_string(unsigned(entry.id));
        assert(final_journal.write(key, 3, state));
        game->reset();
        FileBackend card; assert(card.init(folder)); Journal loaded(card);
        assert(loaded.read(key, 3, bytes) && game->load(bytes));
        assert(game->save() == state);
        game->resume(); assert(game->status() == Status::Running);
        Records records(loaded); Record record{}; record.score = 123; std::strcpy(record.player,"SD");
        assert(records.add(entry.id, 0, 1, record));
        Records after_reboot(final_journal); Leaderboard board;
        assert(after_reboot.load(entry.id, 0, 1, board) && board.entries[0].score == 123);
        assert(loaded.erase(key) && !final_journal.read(key, 3, bytes));
        game->cleanup();
    }
    // Card missing/removed: never report a save as successful.
    std::filesystem::remove_all(folder);
    assert(!final_journal.write("save0", 3, {1}));
    assert(final_boot.error().find("abrir arquivo de save") != std::string::npos);
    std::ofstream(folder) << "not a directory";
    FileBackend invalid; assert(!invalid.init(folder));
    std::filesystem::remove_all(root);
    puts("PASS file storage: all eleven games resume from disk without NVS, settings, records, reboot, torn write recovery, failed replacement, deletion and missing card");
}
