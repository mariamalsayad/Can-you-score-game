#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace soccer {
constexpr float dt = 1.0f/240.0f;
constexpr float radius = .11f;
struct Vec { float x=0, y=radius, z=0; };
struct Setup { float distance=20, wind=0; bool wall=false, keeper=true; };
struct Shot { float aimX=2, aimY=1.5f, power=.65f, spin=0; };
enum class Outcome { Flying, Goal, Saved, Wall, Post, Wide, Over };
inline const char* label(Outcome o) {
    switch(o) {
        case Outcome::Goal:return "WHAT A FINISH.";
        case Outcome::Saved:return "DENIED.";
        case Outcome::Wall:return "INTO THE WALL.";
        case Outcome::Post:return "OFF THE FRAME.";
        case Outcome::Wide:return "JUST WIDE.";
        case Outcome::Over:return "OVER THE BAR.";
        default:return "IN FLIGHT";
    }
}
struct Frame { Vec ball; float keeperX=0, keeperY=.85f, time=0; };
struct Simulation {
    Setup setup;
    Shot shot;
    Vec p, v;
    float time=0, keeperX=0, keeperY=.85f, habit=0;
    Outcome result=Outcome::Flying;
    std::vector<Frame> frames;
    Simulation(Setup s, Shot a, float learned=0):setup(s),shot(a),habit(learned) {
        shot.power=std::clamp(shot.power,0.f,1.f);
        shot.spin=std::clamp(shot.spin,-1.f,1.f);
        v.z=17+shot.power*15;
        const float t=setup.distance/v.z;
        v.x=shot.aimX/t;
        v.y=(shot.aimY-radius+4.905f*t*t)/t;
        keeperX=std::clamp(habit*.55f,-2.5f,2.5f);
        frames.push_back({p,keeperX,keeperY,0});
    }
    void step() {
        if(result!=Outcome::Flying) return;
        const Vec old=p;
        time+=dt;
        v.x+=(shot.spin*3.5f+setup.wind*.55f)*dt;
        v.y-=9.81f*dt;
        p.x+=v.x*dt; p.y+=v.y*dt; p.z+=v.z*dt;
        if(p.y<radius) { p.y=radius; v.y=std::abs(v.y)*.42f; }
        if(setup.keeper && time>.22f) {
            const float remaining=std::max(0.f,(setup.distance-p.z)/v.z);
            const float predicted=std::clamp(p.x+v.x*remaining+habit*.15f,-3.15f,3.15f);
            keeperX+=std::clamp(predicted-keeperX,-3.1f*dt,3.1f*dt);
            const float targetY=std::clamp(p.y+v.y*remaining-4.905f*remaining*remaining,.65f,1.85f);
            keeperY+=std::clamp(targetY-keeperY,-2.f*dt,2.f*dt);
        }
        // Swept plane intersections avoid fast balls tunnelling through obstacles.
        auto atPlane=[&](float z) {
            float f=std::clamp((z-old.z)/(p.z-old.z),0.f,1.f);
            return Vec{old.x+(p.x-old.x)*f,old.y+(p.y-old.y)*f,z};
        };
        float wallZ=setup.distance-9.15f;
        if(setup.wall && old.z<wallZ && p.z>=wallZ) {
            auto q=atPlane(wallZ);
            if(std::abs(q.x)<1.18f+radius && q.y<1.85f+radius) {p=q; result=Outcome::Wall;}
        }
        if(result==Outcome::Flying && p.z>=setup.distance) {
            p=atPlane(setup.distance);
            const bool post=std::abs(std::abs(p.x)-3.66f)<=radius+.06f && p.y<=2.44f+radius;
            const bool bar=std::abs(p.y-2.44f)<=radius+.06f && std::abs(p.x)<=3.66f+radius;
            if(post||bar) result=Outcome::Post;
            else if(std::abs(p.x)>3.66f-radius) result=Outcome::Wide;
            else if(p.y>2.44f-radius) result=Outcome::Over;
            else if(setup.keeper && std::pow((p.x-keeperX)/.76f,2)+std::pow((p.y-keeperY)/.83f,2)<1) result=Outcome::Saved;
            else result=Outcome::Goal;
        }
        frames.push_back({p,keeperX,keeperY,time});
    }
    void finish() { while(result==Outcome::Flying && time<5) step(); }
};
}
