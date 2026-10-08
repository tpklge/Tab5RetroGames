#pragma once
#include "journal.h"
#include "types.h"
namespace arcade {
struct Record {
    char player[16]{};
    uint32_t score = 0, time_ms = 0, distance = 0, date = 0;
    uint16_t level = 0;
    uint8_t won = 0, reserved = 0;
};
struct Leaderboard {
    uint8_t count = 0;
    Record entries[10]{};
};
bool record_better(GameId game, uint8_t mode, const Record &first, const Record &second);
void insert_record(Leaderboard &board, GameId game, uint8_t mode, Record record);
class Records {
  public:
    explicit Records(Journal &journal) : journal(journal) {}
    bool load(GameId game, uint8_t mode, uint8_t difficulty, Leaderboard &board);
    bool add(GameId game, uint8_t mode, uint8_t difficulty, Record record);
    bool erase(GameId game, uint8_t mode, uint8_t difficulty);

  private:
    Journal &journal;
    std::string key(GameId game, uint8_t mode, uint8_t difficulty);
};
} // namespace arcade
