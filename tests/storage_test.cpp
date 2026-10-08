#include "journal.h"
#include "records.h"
#include "settings.h"
#include <map>
#include <cassert>
#include <cstdio>
using namespace arcade;
class Memory : public Backend {
  public:
    std::map<std::string, std::vector<uint8_t>> values;
    bool fail = false;
    bool read(const std::string &key, std::vector<uint8_t> &bytes) override {
        if (!values.count(key))
            return false;
        bytes = values[key];
        return true;
    }
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override {
        if (fail)
            return false;
        values[key] = bytes;
        return true;
    }
    bool erase(const std::string &key) override {
        values.erase(key);
        return true;
    }
};
int main() {
    Memory memory;
    Journal journal(memory);
    std::vector<uint8_t> bytes;
    assert(!journal.read("save", 3, bytes));
    assert(journal.write("save", 3, {1, 2, 3}) && journal.write("save", 3, {4, 5, 6}));
    assert(journal.read("save", 3, bytes) && bytes == std::vector<uint8_t>({4, 5, 6}));
    memory.values["saveb"].back() ^= 0x80;
    assert(journal.read("save", 3, bytes) && bytes == std::vector<uint8_t>({1, 2, 3}));
    memory.fail = true;
    assert(!journal.write("save", 3, {7}));
    assert(journal.read("save", 3, bytes) && bytes[0] == 1);
    assert(!journal.read("save", 4, bytes));
    memory.fail = false;
    auto preferences = default_settings();
    assert(save_settings(journal, preferences));
    Settings restored;
    assert(load_settings(journal, restored));
    assert(restored.brightness == 75 && restored.bindings[0] == key::Left);
    preferences.brightness = 0;
    assert(!save_settings(journal, preferences));
    preferences = default_settings();
    preferences.bindings[1] = preferences.bindings[0];
    assert(!save_settings(journal, preferences));
    Records records(journal);
    Record record{};
    std::strcpy(record.player, "Ana");
    for (unsigned i = 0; i < 20; ++i) {
        record.score = i;
        assert(records.add(GameId::Snake, 0, 1, record));
    }
    Leaderboard board;
    assert(records.load(GameId::Snake, 0, 1, board) && board.count == 10);
    for (int i = 0; i < 10; ++i)
        assert(board.entries[i].score == unsigned(19 - i));
    assert(!records.load(GameId::Snake, 1, 1, board));
    assert(!records.load(GameId::Snake, 0, 0, board));
    Record fast = record, slow = record;
    fast.won = slow.won = 1;
    fast.time_ms = 100;
    slow.time_ms = 200;
    slow.score = 99999;
    assert(record_better(GameId::BlockDrop, 1, fast, slow));
    assert(records.erase(GameId::Snake, 0, 1));
    assert(!records.load(GameId::Snake, 0, 1, board));
    assert(journal.erase("save"));
    assert(!journal.read("save", 3, bytes));
    puts("PASS storage: CRC, schema/type, dual-generation recovery, failed writes, validated "
         "preferences, TOP10, category separation, sprint ordering and confirmed deletion backend");
}
