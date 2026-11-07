#include "render.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace arcade::asteroids {
namespace {
constexpr int X=265,Y=110;
int outside(float x,float y) { return (x<0?1:0)|(x>Width-1?2:0)|(y<0?4:0)|(y>Height-1?8:0); }
void line(Canvas &c,float x1,float y1,float x2,float y2,uint32_t paint,int width=2) {
    for (int attempt=0;attempt<8;++attempt) {
        int a=outside(x1,y1), b=outside(x2,y2);
        if (!(a|b)) { c.line(X+int(x1),Y+int(y1),X+int(x2),Y+int(y2),paint,width); return; }
        if (a&b) return;
        int out=a?a:b; float x,y;
        if (out&4) { y=0; x=x1+(x2-x1)*(y-y1)/(y2-y1); }
        else if (out&8) { y=Height-1.f; x=x1+(x2-x1)*(y-y1)/(y2-y1); }
        else if (out&1) { x=0; y=y1+(y2-y1)*(x-x1)/(x2-x1); }
        else { x=Width-1.f; y=y1+(y2-y1)*(x-x1)/(x2-x1); }
        if (a) { x1=x; y1=y; } else { x2=x; y2=y; }
    }
}
void box(Canvas &c,int x,int y,int w,int h,uint32_t paint,int radius=0) {
    int right=std::min(Width,x+w), bottom=std::min(Height,y+h);
    x=std::max(0,x); y=std::max(0,y);
    if (right>x && bottom>y) c.rect(X+x,Y+y,right-x,bottom-y,paint,radius);
}
template<class Draw> void wrapped(float x,float y,float radius,Draw draw) {
    for (int ox=-1;ox<=1;++ox) for (int oy=-1;oy<=1;++oy) {
        float xx=x+ox*Width, yy=y+oy*Height;
        if (xx>=-radius && xx<=Width+radius && yy>=-radius && yy<=Height+radius) draw(xx,yy);
    }
}
void rock(Canvas &c,const Rock &r,float x,float y) {
    constexpr uint32_t paints[]={0,0xffd798,0x8ee5d0,0xa3b8d3};
    float px[8],py[8];
    for (int i=0;i<8;++i) {
        float a=r.angle+i*Tau/8;
        float radius=RockRadius[r.size]*(.75f+((r.shape*37+i*19)%25)*.01f);
        px[i]=x+std::cos(a)*radius; py[i]=y+std::sin(a)*radius;
    }
    for (int i=0;i<8;++i) line(c,px[i],py[i],px[(i+1)%8],py[(i+1)%8],paints[r.size]);
}
void ship(Canvas &c,const State &s,float x,float y) {
    float angles[]={0,2.45f,Pi,-2.45f}, distances[]={16,14,7,14}, px[4],py[4];
    for (int i=0;i<4;++i) { px[i]=x+std::cos(s.angle+angles[i])*distances[i]; py[i]=y+std::sin(s.angle+angles[i])*distances[i]; }
    if (!s.invulnerable_ms || s.invulnerable_ms/100%2) {
        for (int i=0;i<4;++i) line(c,px[i],py[i],px[(i+1)%4],py[(i+1)%4],0xe6f8ff);
        if (s.thrust) {
            float tip=22.f+(s.elapsed/45%3)*4;
            float xx=x-std::cos(s.angle)*tip, yy=y-std::sin(s.angle)*tip;
            line(c,px[1],py[1],xx,yy,0xffb86b); line(c,xx,yy,px[3],py[3],0xffb86b);
        }
    }
    if (s.invulnerable_ms) for (int i=0;i<8;++i) {
        float a=i*Tau/8,b=(i+1)*Tau/8;
        line(c,x+std::cos(a)*23,y+std::sin(a)*23,x+std::cos(b)*23,y+std::sin(b)*23,0x3e829a,1);
    }
}
void ufo(Canvas &c,float x,float y) {
    float px[]={-23,-12,12,23,12,-12},py[]={0,-7,-7,0,7,7};
    for (int i=0;i<6;++i) line(c,x+px[i],y+py[i],x+px[(i+1)%6],y+py[(i+1)%6],0xff9dd8);
    line(c,x-8,y-7,x-5,y-15,0xff9dd8); line(c,x-5,y-15,x+5,y-15,0xff9dd8); line(c,x+5,y-15,x+8,y-7,0xff9dd8);
    line(c,x-23,y,x+23,y,0xffd5ea,1);
}
}
void AsteroidsGame::render(Canvas &c) const {
    const auto &s=model.state;
    c.rect(X-4,Y-4,Width+8,Height+8,0x334c68,8);
    c.rect(X,Y,Width,Height,0x060e1c);
    for (int i=0;i<45;++i) box(c,(i*197+33)%Width,(i*113+9)%Height,2,2,i%3?0x23344e:0x4c6584);
    for (const auto &r:s.rocks) if (r.active) wrapped(r.x,r.y,RockRadius[r.size],[&](float x,float y){rock(c,r,x,y);});
    if (s.ufo_active) wrapped(s.ufo_x,s.ufo_y,24,[&](float x,float y){ufo(c,x,y);});
    for (auto shot:s.shots) if (shot.active) box(c,int(shot.x)-2,int(shot.y)-2,4,4,shot.enemy?0xff78ae:0x83f4ff,2);
    wrapped(s.x,s.y,30,[&](float x,float y){ship(c,s,x,y);});
    c.text(35,115,"ASTEROIDS",0x8ee5d0,Font::Title);
    char text[170];
    std::snprintf(text,sizeof(text),"PONTOS\n%08lu\n\nONDA\n%u\n\nVIDAS\n%u\n\nASTEROIDES\n%u",(unsigned long)s.score,s.level,s.lives,model.rock_count());
    c.text(40,190,text,color::Text);
    std::snprintf(text,sizeof(text),"HIPERESPACO [Z]\n%s",s.warp_ms?"Recarregando":"Pronto");
    c.text(40,520,text,s.warp_ms?color::Muted:color::Gold,Font::Small);
    std::snprintf(text,sizeof(text),"Tempo: %lu s",(unsigned long)(s.elapsed/1000)); c.text(40,595,text,color::Muted,Font::Small);
    c.text(X+10,650,"Esq/Dir: girar   Cima: impulso   Baixo: frear   X: tiro   Z: hiperespaco",color::Muted,Font::Small);
    if (s.transition) {
        c.rect(X+265,Y+220,420,72,color::Panel,9);
        c.text(X+295,Y+242,"ONDA CONCLUIDA!",color::Gold,Font::Title);
    }
    if (s.options.mode==2) {
        std::snprintf(text,sizeof(text),"Restam %lu s",(unsigned long)((180000-std::min<uint32_t>(180000,s.elapsed))/1000));
        c.text(X+Width-155,Y+16,text,color::Gold,Font::Small);
    }
}
Game *create() { static AsteroidsGame game; return &game; }
}
