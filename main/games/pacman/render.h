#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::pacman {
class PacmanGame : public ModelGame<Model, PacmanGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
