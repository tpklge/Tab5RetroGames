#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::block {
class BlockGame : public ModelGame<Model> {
  public:
    void render(Canvas &canvas) const override;
    void reset() override {
        auto grid = model.state.grid;
        ModelGame::reset();
        model.state.grid = grid;
    }
};
Game *create();
} // namespace arcade::block
