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
class LateBackend : public Backend {
  public:
    explicit LateBackend(Memory &card) : card(card) {}
    bool ready = false, reject = false;
    bool prepare_write() override { ready = !reject; return ready; }
    bool read(const std::string &key, std::vector<uint8_t> &bytes) override {
        return ready && card.read(key, bytes);
    }
    bool write(const std::string &key, const std::vector<uint8_t> &bytes) override {
        return ready && card.write(key, bytes);
    }
    bool erase(const std::string &key) override { return ready && card.erase(key); }
  private:
    Memory &card;
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
    Record efficient{}, inefficient{};
    efficient.won = inefficient.won = 1;
    efficient.distance = 12; inefficient.distance = 25;
    efficient.score = 100; inefficient.score = 99999;
    assert(record_better(GameId::Sokoban, 0, efficient, inefficient));
    efficient.won = 0;
    assert(!record_better(GameId::Sokoban, 0, efficient, inefficient));
    efficient.won = 1;
    assert(record_better(GameId::Sokoban, 2, efficient, inefficient));
    Record long_campaign=efficient; long_campaign.level=20; long_campaign.distance=200;
    inefficient.level=4;
    assert(record_better(GameId::Sokoban,0,long_campaign,inefficient));
    Record asteroid{}; std::strcpy(asteroid.player,"Piloto"); asteroid.score=500;
    assert(records.add(GameId::Asteroids,0,0,asteroid));
    assert(memory.values.count("rankA00a"));
    assert(records.load(GameId::Asteroids,0,0,board) && board.entries[0].score==500);
    assert(records.erase(GameId::Asteroids,0,0));
    Memory inserted;
    Journal previous(inserted);
    assert(previous.write("save0", 3, {10}) && previous.write("save0", 3, {20}));
    LateBackend late(inserted); Journal retry(late);
    auto untouched = inserted.values;
    late.reject = true;
    assert(!retry.write("save0", 3, {30}) && inserted.values == untouched);
    late.reject = false;
    assert(retry.write("save0", 3, {30}));
    assert(retry.read("save0", 3, bytes) && bytes == std::vector<uint8_t>({30}));
    inserted.values["save0a"].back() ^= 0xff;
    assert(retry.read("save0", 3, bytes) && bytes == std::vector<uint8_t>({20}));
    puts("PASS storage: CRC, schema/type, dual-generation recovery, failed writes, validated "
         "preferences, TOP10, category separation, sprint ordering and confirmed deletion backend");
}
