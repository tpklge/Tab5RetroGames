#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::breakout {
class BreakoutGame : public ModelGame<Model, BreakoutGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
