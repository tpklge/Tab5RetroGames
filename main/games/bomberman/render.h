#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::bomberman {
class BomberGame : public ModelGame<Model, BomberGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
