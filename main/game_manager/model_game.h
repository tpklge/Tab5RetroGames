#pragma once
#include "game.h"
namespace arcade {
template <class Model> class ModelGame : public Game {
  public:
    Model model;
    void init(const Options &options) override { model.begin(options); }
    void start() override { model.state.status = Status::Running; }
    void update(uint32_t delta, const Controls &controls) override { model.step(delta, controls); }
    void pause() override {
        if (model.state.status == Status::Running)
            model.state.status = Status::Paused;
    }
    void resume() override {
        if (model.state.status == Status::Paused)
            model.state.status = Status::Running;
    }
    std::vector<uint8_t> save() const override { return model.save(); }
    bool load(const std::vector<uint8_t> &bytes) override { return model.load(bytes); }
    void reset() override { model.begin(model.state.options); }
    void cleanup() override { model = Model{}; }
    Status status() const override { return model.state.status; }
    Options options() const override { return model.state.options; }
    Result result() const override { return model.result(); }
    uint32_t sounds() override {
        uint32_t events = model.sound_events;
        model.sound_events = 0;
        return events;
    }
};
} // namespace arcade
