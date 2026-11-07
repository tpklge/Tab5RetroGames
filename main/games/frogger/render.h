#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::frogger {
class FroggerGame : public ModelGame<Model, FroggerGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
