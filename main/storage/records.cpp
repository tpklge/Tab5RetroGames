#include "records.h"
#include <algorithm>
#include <cstdio>
namespace arcade {
bool record_better(GameId game, uint8_t mode, const Record &first, const Record &second) {
    if ((game == GameId::BlockDrop || game == GameId::Racer) && mode == 1) {
        if (first.won != second.won)
            return first.won > second.won;
        if (first.won && first.time_ms != second.time_ms)
            return first.time_ms < second.time_ms;
    }
    if (first.score != second.score)
        return first.score > second.score;
    if (first.distance != second.distance)
        return first.distance > second.distance;
    if (game == GameId::Snake && first.time_ms != second.time_ms)
        return first.time_ms > second.time_ms;
    return first.time_ms < second.time_ms;
}
void insert_record(Leaderboard &board, GameId game, uint8_t mode, Record record) {
    unsigned position = 0;
    while (position < board.count && !record_better(game, mode, record, board.entries[position]))
        ++position;
    if (position >= 10)
        return;
    unsigned count = std::min(10u, unsigned(board.count) + 1);
    for (unsigned index = count - 1; index > position; --index)
        board.entries[index] = board.entries[index - 1];
    board.entries[position] = record;
    board.count = uint8_t(count);
}
std::string Records::key(GameId game, uint8_t mode, uint8_t difficulty) {
    char key[15];
    std::snprintf(key, sizeof(key), "rank%u%u%u", unsigned(game), mode, difficulty);
    return key;
}
bool Records::load(GameId game, uint8_t mode, uint8_t difficulty, Leaderboard &board) {
    std::vector<uint8_t> bytes;
    Leaderboard restored;
    if (!journal.read(key(game, mode, difficulty), 2, bytes) || !decode(bytes, restored) ||
        restored.count > 10) {
        board = Leaderboard{};
        return false;
    }
    for (unsigned index = 0; index < restored.count; ++index)
        if (!std::memchr(restored.entries[index].player, 0, 16) ||
            restored.entries[index].won > 1) {
            board = Leaderboard{};
            return false;
        }
    std::memcpy(&board, &restored, sizeof(board));
    return true;
}
bool Records::add(GameId game, uint8_t mode, uint8_t difficulty, Record record) {
    Leaderboard board;
    std::fill_n(reinterpret_cast<uint8_t *>(&board), sizeof(board), 0);
    load(game, mode, difficulty, board);
    insert_record(board, game, mode, record);
    return journal.write(key(game, mode, difficulty), 2, snapshot(board));
}
bool Records::erase(GameId game, uint8_t mode, uint8_t difficulty) {
    return journal.erase(key(game, mode, difficulty));
}
} // namespace arcade
