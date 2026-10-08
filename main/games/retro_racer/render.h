#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::racer {
class RacerGame : public ModelGame<Model> {
  public:
    void render(Canvas &canvas) const override;
};
Game *create();
} // namespace arcade::racer
