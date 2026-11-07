#pragma once
#include "model.h"
#include "model_game.h"
namespace arcade::impact {
class ImpactGame : public ModelGame<Model, ImpactGame> {
 public:
    void render(Canvas &canvas) const override;
};
Game *create();
}
