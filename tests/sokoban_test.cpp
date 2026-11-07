#include "games/sokoban/model.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdio>
using namespace arcade;
using namespace arcade::sokoban;
Controls press(Action a) { Controls c; c.press[size_t(a)] = 1; return c; }
unsigned direction(char c) { return c == 'U' ? 0 : c == 'R' ? 1 : c == 'D' ? 2 : 3; }
void fixture(Model &m) {
    m.begin({});
    for (int i = 0; i < Cells; ++i) {
        m.state.walls[i] = ! (i % Width) || i % Width == Width-1 || !(i/Width) || i/Width == Height-1;
        m.state.boxes[i] = m.state.goals[i] = 0;
    }
    m.state.player = m.state.previous = Width+1;
    m.state.boxes[Width+2] = 1; m.state.goals[Width+4] = 1;
}
int main() {
    // Golden map+witness hashes from the published four-stage packs.
    constexpr uint32_t legacy[] = {0x2ffe22b6u,0xad50de7du,0x54407030u,0x6d2f85cbu,0xec7cb6f5u,0x3a51508du,0x970aea5u,0x100ec9ebu,0x93310f87u,0x7506be61u,0xc1a4bcdeu,0xf75dc930u,0xe074b60u,0xf8f81d89u,0x1e8e822fu,0xd8259015u};
    for (int d=0;d<4;++d) for (int i=0;i<4;++i) {
        const auto &p = Puzzles[d*LevelsPerPack+i]; uint32_t hash=2166136261u;
        for (const char *text : {p.map,p.solution}) for (const char *c=text;*c;++c) hash=(hash^uint8_t(*c))*16777619u;
        assert(hash==legacy[d*4+i]);
    }
    for (int i=0;i<PackCount*LevelsPerPack;++i) for (int j=0;j<i;++j)
        assert(std::strcmp(Puzzles[i].map,Puzzles[j].map)!=0);
    // A completed legacy campaign resumes at stage 5, with its banked score.
    for (unsigned d=0;d<4;++d) for (unsigned mode=0;mode<3;++mode) {
        Options options; options.difficulty=uint8_t(d); options.mode=uint8_t(mode);
        Model old; old.begin(options);
        for (int stage=1;stage<=4;++stage) {
            for (const char *c=old.puzzle().solution;*c;++c) assert(old.move(direction(*c)));
            if (stage<4) { old.step(1000,{});old.step(200,{}); }
        }
        old.state.status=Status::Won; old.state.transition=0;
        old.state.score+=uint32_t(std::max<int64_t>(100,10000+d*250-int64_t(old.state.moves)*15-int64_t(old.state.pushes)*35));
        Model resumed; assert(resumed.load(old.save()));
        assert(resumed.state.level==5 && resumed.state.status==Status::Running && !resumed.state.moves);
        assert(resumed.state.score==old.state.score && resumed.state.elapsed==old.state.elapsed);
        assert(resumed.state.total_moves==old.state.total_moves+old.state.moves);
        assert(resumed.state.total_pushes==old.state.total_pushes+old.state.pushes);
    }
    Model m; fixture(m);
    assert(!m.move(0) && !m.state.moves);
    assert(m.move(1) && m.state.player == Width+2 && m.state.boxes[Width+3] && m.state.pushes == 1);
    assert(m.undo() && m.state.player == Width+1 && m.state.boxes[Width+2] && !m.state.moves && !m.state.pushes);
    m.state.boxes[Width+3] = 1; assert(!m.move(1));
    fixture(m); m.state.boxes[Width+2] = 0; m.state.boxes[Width+3] = 1;
    m.state.player = Width+2; assert(m.move(3) && m.state.boxes[Width+3] && !m.state.pushes);
    fixture(m); m.state.player = Width+3; assert(m.move(3)); assert(m.deadlocked());
    assert(m.undo() && !m.deadlocked());
    fixture(m); assert(m.move(1) && m.move(1)); assert(m.solved() && m.state.transition);
    assert(m.undo() && !m.solved() && !m.state.transition);
    // Every authored phase has a playable witness, in every mode and difficulty.
    for (unsigned mode = 0; mode < 4; ++mode) for (unsigned difficulty = 0; difficulty < 4; ++difficulty) {
        Options options; options.mode = uint8_t(mode); options.difficulty = uint8_t(difficulty);
        m.begin(options); unsigned total_moves = 0, total_pushes = 0;
        for (int level = 0; level < LevelsPerPack; ++level) {
            assert(m.load(m.save()) && !m.solved() && !m.deadlocked());
            const Puzzle &p = m.puzzle();
            for (const char *c = p.solution; *c; ++c) assert(m.move(direction(*c)));
            assert(m.solved() && m.state.moves == p.moves && m.state.pushes == p.pushes);
            total_moves += p.moves; total_pushes += p.pushes;
            Model copy; assert(copy.load(m.save()));
            assert(copy.undo() && !copy.solved());
            m.step(1000, {}); m.step(200, {});
        }
        if (mode != 3) {
            assert(m.state.status == Status::Won && m.state.score);
            assert(m.result().distance == (mode == 2 ? total_pushes : total_moves));
        } else assert(m.state.status == Status::Running && m.state.level == 1);
    }
    m.begin({}); auto initial_player = m.state.player;
    auto initial_boxes = snapshot(m.state.boxes);
    const char *solution = m.puzzle().solution;
    for (const char *c = solution; *c; ++c) assert(m.move(direction(*c)));
    while (m.undo()) {}
    assert(m.state.player == initial_player && snapshot(m.state.boxes) == initial_boxes && !m.state.moves);
    for (int i = 0; i < 600; ++i) assert(m.move(i % 2 ? 1 : 3));
    assert(m.state.undo_count == MaxUndo && m.state.moves == 600);
    Model copy; assert(copy.load(m.save()));
    for (int i = 0; i < MaxUndo; ++i) assert(copy.undo());
    assert(!copy.undo() && copy.state.moves == 600-MaxUndo);
    m.step(10, press(Action::Primary)); assert(!m.state.moves && !m.state.undo_count && m.state.player == initial_player);
    m.step(10, press(Action::Left)); assert(m.state.moves == 1);
    m.step(10, press(Action::CounterRotate)); assert(!m.state.moves);
    m.state.status = Status::Paused; auto paused = m.save(); m.step(1000, press(Action::Right)); assert(m.save() == paused);
    Options limited; limited.mode = 1; m.begin(limited);
    while (m.state.moves < m.move_limit()) assert(m.move(m.state.moves % 2 ? 1 : 3));
    assert(m.limited() && !m.move(3) && m.undo() && !m.limited());
    Model push_budget; fixture(push_budget); push_budget.state.options.mode = 2;
    std::fill_n(push_budget.state.boxes, Cells, uint8_t(0));
    push_budget.state.boxes[4*Width+5] = 1; push_budget.state.player = 4*Width+4;
    const unsigned cycle[] = {1,0,1,1,2,3,2,3,3,0};
    for (int round = 0; round < 7; ++round) for (unsigned d : cycle) assert(push_budget.move(d));
    assert(push_budget.limited() && !push_budget.move(1));
    for (int i = 0; i < 5; ++i) assert(push_budget.undo());
    assert(!push_budget.limited());
    auto saved = m.save(); assert(copy.load(saved));
    m.step(50, {}); copy.step(50, {}); assert(m.save() == copy.save());
    auto before = copy.save(); auto broken = saved; broken.pop_back(); assert(!copy.load(broken) && copy.save() == before);
    State bad = m.state; bad.boxes[0] = 1; assert(!copy.load(snapshot(bad)));
    bad = m.state; bad.history[(bad.undo_head+bad.undo_count-1)%MaxUndo].player = 255; assert(!copy.load(snapshot(bad)));
    bad = m.state; bad.player = Cells; assert(!copy.load(snapshot(bad)));
    puts("PASS Sokoban: 80 solved original puzzles, legacy saves/maps in four modes, walls/pushing/no pulling, corner warning, 512-step undo/replay, restart, budgets, scoring and validated saves");
}
