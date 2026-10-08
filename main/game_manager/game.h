#pragma once
#include "types.h"
#include "canvas.h"
namespace arcade {
class Game {
  public:
    virtual ~Game() = default;
    virtual void init(const Options &options) = 0;
    virtual void start() = 0;
    virtual void update(uint32_t delta_ms, const Controls &controls) = 0;
    virtual void render(Canvas &canvas) const = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual std::vector<uint8_t> save() const = 0;
    virtual bool load(const std::vector<uint8_t> &bytes) = 0;
    virtual void reset() = 0;
    virtual void cleanup() = 0;
    virtual Status status() const = 0;
    virtual Options options() const = 0;
    virtual Result result() const = 0;
    virtual uint32_t sounds() = 0;
};
struct Descriptor {
    GameId id;
    const char *name;
    const char *description;
    const char *const *modes;
    const char *controls;
    Game *(*create)();
};
const std::vector<Descriptor> &registry();
} // namespace arcade
