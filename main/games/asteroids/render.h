#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::asteroids {
class AsteroidsGame : public ModelGame<Model, AsteroidsGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
