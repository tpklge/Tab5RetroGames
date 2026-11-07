#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::sokoban {
class SokobanGame : public ModelGame<Model, SokobanGame> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
