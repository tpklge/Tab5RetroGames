#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::asteroids {
namespace {
float wrap(float value, float period) {
    value = std::fmod(value, period);
    if (value < 0) value += period;
    return value >= period ? 0.f : value;
}
float delta(float from, float to, float period) {
    float value = to - from;
    if (value > period / 2) value -= period;
    if (value < -period / 2) value += period;
    return value;
}
void reduce(uint32_t &timer) { timer = timer > 5 ? timer - 5 : 0; }
void limit(float &vx, float &vy, float maximum) {
    float speed = std::hypot(vx, vy);
    if (speed > maximum) { vx *= maximum / speed; vy *= maximum / speed; }
}
}
float Model::distance_squared(float x, float y, float ox, float oy) {
    float dx = delta(x,ox,float(Width)), dy = delta(y,oy,float(Height)); return dx*dx+dy*dy;
}
unsigned Model::rock_count() const { unsigned count = 0; for (auto r : state.rocks) count += r.active; return count; }
void Model::reset_ship() {
    state.x = Width / 2; state.y = Height / 2; state.vx = state.vy = 0; state.angle = Pi * 1.5f;
    state.thrust = state.shot_request = state.warp_request = 0;
    state.invulnerable_ms = 1800; state.cooldown = 0;
}
void Model::wave() {
    for (auto &r : state.rocks) r = Rock{};
    for (auto &s : state.shots) s = Shot{};
    reset_ship(); state.transition = state.respawn_ms = state.warp_ms = 0;
    state.ufo_active = 0; state.ufo_clock = state.ufo_age = state.ufo_shot_clock = 0;
    unsigned count = std::min(9, 3 + int(state.level));
    for (unsigned i = 0; i < count; ++i) {
        auto &r = state.rocks[i]; r.active = 1; r.size = 3;
        unsigned side = state.random.range(4);
        r.x = side < 2 ? (side ? Width - 1.f : 0.f) : float(state.random.range(Width));
        r.y = side >= 2 ? (side == 2 ? 0.f : Height - 1.f) : float(state.random.range(Height));
        float direction = float(state.random.range(6283)) * .001f;
        float speed = std::min(130.f, 42.f + state.level * 7.f + state.options.difficulty * 12.f);
        r.vx = std::cos(direction) * speed; r.vy = std::sin(direction) * speed;
        r.angle = float(state.random.range(6283)) * .001f;
        r.spin = (int(state.random.range(201)) - 100) * .008f;
        r.shape = uint8_t(state.random.range(8));
    }
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{}; state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed; sound_events = 0; wave();
}
bool Model::fire(float x, float y, float vx, float vy, bool enemy) {
    unsigned count = 0;
    for (const auto &s : state.shots) count += s.active && s.enemy == enemy;
    if (count >= (enemy ? 12u : 20u)) return false;
    for (auto &s : state.shots) if (!s.active) {
        s = {wrap(x,float(Width)), wrap(y,float(Height)), vx, vy, enemy ? 2800u : 1000u, 1, uint8_t(enemy)};
        if (!enemy) sound_events |= Drop;
        return true;
    }
    return false;
}
void Model::destroy(unsigned index) {
    if (index >= MaxRocks || !state.rocks[index].active) return;
    Rock parent = state.rocks[index]; state.rocks[index].active = 0;
    state.score += parent.size == 3 ? 20 : parent.size == 2 ? 50 : 100;
    sound_events |= Eat;
    if (parent.size == 1) return;
    for (int child = 0; child < 2; ++child) for (auto &r : state.rocks) if (!r.active) {
        r = parent; r.size = uint8_t(parent.size - 1); r.active = 1;
        float angle = parent.angle + (child ? 1.f : -1.f) * 1.1f;
        float impulse = 45.f + state.options.difficulty * 8.f;
        r.vx = parent.vx * .9f + std::cos(angle) * impulse;
        r.vy = parent.vy * .9f + std::sin(angle) * impulse; limit(r.vx,r.vy,300);
        r.x = wrap(parent.x + std::cos(angle) * 5, float(Width));
        r.y = wrap(parent.y + std::sin(angle) * 5, float(Height));
        r.spin = child ? .8f : -.8f; r.shape = uint8_t(state.random.range(8));
        break;
    }
}
void Model::spawn_ufo() {
    state.ufo_active = 1; state.ufo_clock = state.ufo_age = state.ufo_shot_clock = 0;
    bool right = state.random.range(2) != 0;
    state.ufo_x = right ? 0.f : Width - 1.f;
    state.ufo_y = state.ufo_base_y = 50.f + state.random.range(Height - 100);
    state.ufo_vx = (right ? 1.f : -1.f) * (95 + state.options.difficulty * 15.f);
    sound_events |= Special;
}
void Model::hyperspace() {
    if (state.warp_ms || state.respawn_ms) return;
    float best = -1, bx = state.x, by = state.y;
    for (int i = 0; i < 32; ++i) {
        float x = float(state.random.range(Width)), y = float(state.random.range(Height));
        float clearance = float(Width * Width);
        for (const auto &r : state.rocks) if (r.active) clearance = std::min(clearance, distance_squared(x,y,r.x,r.y));
        if (state.ufo_active) clearance = std::min(clearance, distance_squared(x,y,state.ufo_x,state.ufo_y));
        if (clearance > best) { best = clearance; bx = x; by = y; }
    }
    state.x = bx; state.y = by; state.vx = state.vy = 0;
    state.invulnerable_ms = std::max<uint32_t>(state.invulnerable_ms, 1000);
    state.warp_ms = 8000; sound_events |= Bonus;
}
void Model::damage() {
    if (state.invulnerable_ms || state.respawn_ms) return;
    sound_events |= Crash;
    if (state.options.mode != 3) --state.lives;
    if (!state.lives) { finish(Status::Lost); return; }
    reset_ship(); state.invulnerable_ms = 2200; state.respawn_ms = 500;
    for (auto &s : state.shots) s.active = 0;
}
void Model::finish(Status status) { state.status = status; state.remainder %= 5; sound_events |= GameOver; }
void Model::step(uint32_t delta_ms, const Controls &controls) {
    if (state.status != Status::Running) return;
    if (controls.hit(Action::Primary)) state.shot_request = 1;
    if (controls.hit(Action::CounterRotate)) state.warp_request = 1;
    state.remainder += std::min<uint32_t>(delta_ms, 1000);
    while (state.remainder >= 5 && state.status == Status::Running) { state.remainder -= 5; tick(controls); }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 5;
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) { state.level = uint16_t(std::min(1000, int(state.level) + 1)); wave(); sound_events |= Level; }
        return;
    }
    reduce(state.cooldown); reduce(state.invulnerable_ms); reduce(state.respawn_ms); reduce(state.warp_ms);
    if (!state.respawn_ms) {
        if (state.warp_request) { state.warp_request = 0; hyperspace(); }
        state.angle = wrap(state.angle + .019f * (int(controls.down(Action::Right)) - int(controls.down(Action::Left))), Tau);
        bool thrust = controls.down(Action::Up);
        if (thrust && !state.thrust) sound_events |= Turbo;
        state.thrust = thrust;
        if (thrust) { state.vx += std::cos(state.angle) * .925f; state.vy += std::sin(state.angle) * .925f; }
        if (controls.down(Action::Down)) { state.vx *= .985f; state.vy *= .985f; }
        limit(state.vx,state.vy,MaxShipSpeed);
        if (!state.cooldown && (state.shot_request || controls.down(Action::Primary))) {
            fire(state.x + std::cos(state.angle) * 17, state.y + std::sin(state.angle) * 17,
                 std::cos(state.angle) * 440 + state.vx * .4f, std::sin(state.angle) * 440 + state.vy * .4f, false);
            state.cooldown = 180; state.shot_request = 0;
        }
        state.x = wrap(state.x + state.vx * .005f, float(Width));
        state.y = wrap(state.y + state.vy * .005f, float(Height));
    } else state.thrust = 0;
    for (auto &r : state.rocks) if (r.active) {
        r.x = wrap(r.x + r.vx * .005f, float(Width)); r.y = wrap(r.y + r.vy * .005f, float(Height));
        r.angle = wrap(r.angle + r.spin * .005f,Tau);
    }
    state.ufo_clock += 5;
    if (!state.ufo_active && state.ufo_clock >= uint32_t(18000 - state.options.difficulty * 2000)) spawn_ufo();
    if (state.ufo_active) {
        state.ufo_age += 5; state.ufo_shot_clock += 5;
        state.ufo_x = wrap(state.ufo_x + state.ufo_vx * .005f,float(Width));
        state.ufo_y = std::clamp(state.ufo_base_y + std::sin(state.ufo_age * .002f) * 30, 22.f, Height - 22.f);
        if (state.ufo_age >= 9000) state.ufo_active = 0;
        else if (state.ufo_shot_clock >= uint32_t(1200 - state.options.difficulty * 170)) {
            state.ufo_shot_clock = 0;
            float dx = delta(state.ufo_x,state.x,float(Width)), dy = delta(state.ufo_y,state.y,float(Height));
            float length = std::max(1.f,std::hypot(dx,dy)), speed = 185 + state.options.difficulty * 25.f;
            fire(state.ufo_x,state.ufo_y,dx/length*speed,dy/length*speed,true);
        }
    }
    for (auto &s : state.shots) if (s.active) {
        if (s.life <= 5) { s.active = 0; s.life = 0; continue; }
        s.life -= 5; s.x = wrap(s.x + s.vx * .005f,float(Width)); s.y = wrap(s.y + s.vy * .005f,float(Height));
        if (s.enemy) {
            if (distance_squared(s.x,s.y,state.x,state.y) < (ShipRadius+3)*(ShipRadius+3)) { s.active = 0; damage(); }
        } else {
            for (unsigned i = 0; i < MaxRocks && s.active; ++i) if (state.rocks[i].active) {
                auto &r = state.rocks[i]; float radius = RockRadius[r.size] * .85f + 2;
                if (distance_squared(s.x,s.y,r.x,r.y) < radius*radius) { s.active = 0; destroy(i); }
            }
            if (s.active && state.ufo_active && distance_squared(s.x,s.y,state.ufo_x,state.ufo_y) < 24*24) {
                s.active = 0; state.ufo_active = 0; state.score += 250; sound_events |= Bonus;
            }
        }
        if (state.status != Status::Running) return;
    }
    if (!state.invulnerable_ms && !state.respawn_ms) {
        for (const auto &r : state.rocks) if (r.active) {
            float radius = RockRadius[r.size] * .8f + ShipRadius;
            if (distance_squared(state.x,state.y,r.x,r.y) < radius*radius) { damage(); break; }
        }
        if (state.ufo_active && distance_squared(state.x,state.y,state.ufo_x,state.ufo_y) < 30*30) damage();
    }
    if (state.status != Status::Running) return;
    if (!rock_count() && !state.ufo_active) {
        state.score += state.level * 500;
        if (state.options.mode == 0 && state.level >= CampaignWaves) finish(Status::Won);
        else { state.transition = 1200; sound_events |= Level; }
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes,s) || !options_valid(s.options) || !s.random.value || unsigned(s.status)>3 ||
        !s.level || s.level>1000 || s.lives>3 || s.thrust>1 || s.shot_request>1 || s.warp_request>1 || s.ufo_active>1 ||
        s.cooldown>180 || s.invulnerable_ms>2200 || s.respawn_ms>500 || s.warp_ms>8000 ||
        s.ufo_clock>60000 || s.ufo_age>9000 || s.ufo_shot_clock>1200 || s.transition>1200 || s.remainder>=5) return false;
    auto valid=[](float x,float low,float high) { return std::isfinite(x) && x>=low && x<=high; };
    auto position=[&](float x,float y) { return valid(x,0,Width) && x<Width && valid(y,0,Height) && y<Height; };
    if (!position(s.x,s.y) || !valid(s.vx,-MaxShipSpeed,MaxShipSpeed) || !valid(s.vy,-MaxShipSpeed,MaxShipSpeed) ||
        std::hypot(s.vx,s.vy)>MaxShipSpeed+.1f || !valid(s.angle,0,Tau) || s.angle>=Tau ||
        !position(s.ufo_x,s.ufo_y) || !valid(s.ufo_base_y,0,Height) || !valid(s.ufo_vx,-160,160)) return false;
    unsigned potential=0, friendly=0, hostile=0;
    for (const auto &r:s.rocks) {
        if (r.active>1 || r.size<1 || r.size>3 || r.shape>7 || !position(r.x,r.y) || !valid(r.angle,0,Tau) || r.angle>=Tau ||
            !valid(r.spin,-2,2) || !valid(r.vx,-300,300) || !valid(r.vy,-300,300) || std::hypot(r.vx,r.vy)>300.1f) return false;
        if (r.active) potential += 1u << (r.size-1);
    }
    if (potential>36) return false;
    for (const auto &shot:s.shots) {
        if (shot.active>1 || shot.enemy>1 || shot.life>(shot.enemy?2800u:1000u) || (shot.active&&!shot.life) ||
            !position(shot.x,shot.y) || !valid(shot.vx,-600,600) || !valid(shot.vy,-600,600)) return false;
        if (shot.active) (shot.enemy?hostile:friendly)++;
    }
    if (friendly>20 || hostile>12 || ((s.status==Status::Running || s.status==Status::Paused)&&!s.lives)) return false;
    std::memcpy(&state,&s,sizeof(state)); sound_events=0; return true;
}
}
