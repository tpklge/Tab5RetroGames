#include "games/asteroids/model.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
using namespace arcade;
using namespace arcade::asteroids;
Controls action(Action a,bool press=false) { Controls c; (press?c.press:c.held)[size_t(a)]=1;return c; }
void quiet(Model &m,Options options={}) {
    m.begin(options);for(auto &r:m.state.rocks)r=Rock{};
    m.state.rocks[0]={30,30,0,0,0,0,1,1,0};
}
int main() {
    Model m; m.begin({}); assert(m.rock_count()==4 && m.state.lives==3);
    quiet(m); m.step(1000,action(Action::Up)); assert(m.state.vy< -180 && m.state.y<Height/2);
    float velocity=m.state.vy;m.step(100,{});assert(m.state.vy==velocity);
    float angle=m.state.angle;m.step(200,action(Action::Right));assert(m.state.angle!=angle && m.state.vy==velocity);
    m.step(500,action(Action::Down));assert(std::hypot(m.state.vx,m.state.vy)<std::abs(velocity)/2);
    quiet(m);for(int i=0;i<5;++i)m.step(1000,action(Action::Up));
    assert(std::hypot(m.state.vx,m.state.vy)<=MaxShipSpeed+.01f);
    quiet(m);m.state.x=Width-1;m.state.vx=300;m.step(10,{});assert(m.state.x<5);
    m.state.y=1;m.state.vy=-300;m.step(10,{});assert(m.state.y>Height-5);
    assert(Model::distance_squared(1,100,Width-1,100)==4);
    m.state.status=Status::Paused;auto paused=m.save();m.step(500,action(Action::Primary));assert(m.save()==paused);
    quiet(m);m.step(3,action(Action::Primary,true));assert(m.state.shot_request);
    Model partial;assert(partial.load(m.save()));m.step(2,{});partial.step(2,{});
    assert(m.save()==partial.save() && m.state.shots[0].active && !m.state.shots[0].enemy);
    m.state.shots[0].life=5;m.step(5,{});assert(!m.state.shots[0].active);
    quiet(m);m.state.rocks[0]={150,150,0,0,0,0,1,3,0};m.destroy(0);
    assert(m.rock_count()==2 && m.state.score==20);
    unsigned medium=0;for(auto r:m.state.rocks)medium+=r.active&&r.size==2;assert(medium==2);
    while(m.rock_count())for(unsigned i=0;i<MaxRocks;++i)if(m.state.rocks[i].active){m.destroy(i);break;}
    assert(m.state.score==520);m.step(5,{});assert(m.state.transition);
    m.step(1000,{});m.step(200,{});assert(m.state.level==2 && m.rock_count()==5);
    quiet(m);m.state.rocks[0].x=1;m.state.rocks[0].y=200;
    m.state.shots[0]={Width-1.f,200,0,0,1000,1,0};m.step(5,{});
    assert(!m.rock_count() && !m.state.shots[0].active);
    quiet(m);m.state.invulnerable_ms=0;m.state.x=Width-1;m.state.y=100;
    m.state.rocks[0].x=1;m.state.rocks[0].y=100;m.step(5,{});
    assert(m.state.lives==2 && m.state.respawn_ms && m.state.invulnerable_ms);
    m.state.lives=1;m.state.invulnerable_ms=m.state.respawn_ms=0;
    m.state.x=m.state.rocks[0].x=100;m.state.y=m.state.rocks[0].y=100;m.step(5,{});assert(m.state.status==Status::Lost);
    quiet(m);m.spawn_ufo();m.step(1000,{});m.step(100,{});
    unsigned hostile=0;for(auto s:m.state.shots)hostile+=s.active&&s.enemy;assert(hostile && m.state.ufo_active);
    quiet(m);m.spawn_ufo();m.state.ufo_x=200;m.state.ufo_y=m.state.ufo_base_y=100;m.state.ufo_vx=0;
    m.state.shots[0]={200,100,0,0,1000,1,0};m.step(5,{});assert(!m.state.ufo_active && m.state.score==250);
    quiet(m);m.state.vx=200;m.hyperspace();assert(m.state.warp_ms==8000 && !m.state.vx && m.state.invulnerable_ms>=1000);
    auto warp=m.save();m.hyperspace();assert(m.save()==warp);
    quiet(m);m.state.level=CampaignWaves;m.state.rocks[0].active=0;m.step(5,{});assert(m.state.status==Status::Won);
    Options timed;timed.mode=2;m.begin(timed);m.state.elapsed=179995;m.step(5,{});assert(m.state.status==Status::Won);
    Options train;train.mode=3;quiet(m,train);m.state.invulnerable_ms=0;
    m.state.x=m.state.rocks[0].x;m.state.y=m.state.rocks[0].y;m.step(5,{});assert(m.state.lives==3);
    quiet(m);for(auto &r:m.state.rocks)r=Rock{};
    for(int i=0;i<9;++i)m.state.rocks[i]={float(i*70),30,0,0,0,0,1,3,0};
    for(;;){int index=-1;for(int i=0;i<MaxRocks;++i)if(m.state.rocks[i].active&&m.state.rocks[i].size>1){index=i;break;}
        if(index<0)break;m.destroy(unsigned(index));}
    assert(m.rock_count()==36);Model copy;assert(copy.load(m.save()));
    Model a,b;a.begin({});b.begin({});Controls thrust=action(Action::Up);thrust.held[size_t(Action::Primary)]=1;
    a.step(1000,thrust);for(int i=0;i<200;++i)b.step(5,thrust);assert(a.save()==b.save());
    assert(copy.load(a.save()));for(int i=0;i<100;++i){a.step(5,{});copy.step(5,{});}assert(a.save()==copy.save());
    auto before=copy.save();auto bad=before;bad.pop_back();assert(!copy.load(bad)&&copy.save()==before);
    State invalid=a.state;invalid.vx=std::numeric_limits<float>::quiet_NaN();assert(!copy.load(snapshot(invalid)));
    invalid=a.state;invalid.rocks[0].size=8;assert(!copy.load(snapshot(invalid)));
    train.difficulty=3;m.begin(train);
    for(int i=0;i<20000;++i){Controls c=action(Action::Up);c.held[size_t(Action::Right)]=1;c.held[size_t(Action::Primary)]=1;
        if(i%700==0)c.press[size_t(Action::CounterRotate)]=1;
        m.step(10,c);if(i%50==0)assert(copy.load(m.save()));}
    puts("PASS Asteroids: rotation/inertia/brake, speed cap, torus/collisions, fire, splitting/capacity, UFO, damage, warp, waves/modes and deterministic validated saves");
}
