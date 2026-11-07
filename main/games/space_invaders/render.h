#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::invaders {
class InvadersGame : public ModelGame<Model, InvadersGame> {
 public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
