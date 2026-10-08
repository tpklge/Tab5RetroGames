#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::snake {
class SnakeGame : public ModelGame<Model> {
  public:
    void render(Canvas &canvas) const override;
    void reset() override {
        auto saved = model.state;
        ModelGame::reset();
        std::copy(std::begin(saved.rules), std::end(saved.rules), model.state.rules);
    }
};
Game *create();
} // namespace arcade::snake
