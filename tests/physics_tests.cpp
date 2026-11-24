#include "physics.hpp"
#include <iostream>
#include <cstdlib>
using namespace soccer;
int checks=0;
void check(bool b,const char* message) {++checks; if(!b){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 Setup empty; empty.keeper=false;
 Simulation goal(empty,{2,1.5f,.7f,0}); goal.finish();
 check(goal.result==Outcome::Goal,"A clear shot must score");
 check(std::abs(goal.p.x-2)<.01f,"Plane interpolation must preserve target x");
 Simulation wide(empty,{5,1,.5f,0});wide.finish();check(wide.result==Outcome::Wide,"Wide detection");
 Simulation high(empty,{1,4,.5f,0});high.finish();check(high.result==Outcome::Over,"Over detection");
 Simulation post(empty,{3.66f,1,.5f,0});post.finish();check(post.result==Outcome::Post,"Post detection");
 Simulation bar(empty,{0,2.44f,.5f,0});bar.finish();check(bar.result==Outcome::Post,"Crossbar detection");
 Simulation save(Setup{},{0,1,.5f,0});save.finish();check(save.result==Outcome::Saved,"Central shot should be saved");
 Setup wall=empty;wall.wall=true;
 Simulation blocked(wall,{0,.15f,1,0});blocked.finish();check(blocked.result==Outcome::Wall,"Wall collision");
 Simulation left(empty,{0,1,.5f,-1}),right(empty,{0,1,.5f,1});left.finish();right.finish();
 check(left.p.x<-.5f && right.p.x>.5f,"Spin bends in both directions");
 check(std::abs(left.p.x+right.p.x)<.001f,"Spin should be symmetric");
 Setup windy=empty;windy.wind=2;Simulation wind(windy,{0,1,.5f,0});wind.finish();check(wind.p.x>0,"Wind affects trajectory");
 Simulation adapted(Setup{},{0,1,.5f,0},3);check(adapted.keeperX>1,"Keeper anticipates learned preference");
 for(int i=0;i<1000;++i){
   Setup s{18.f+(i%10),float(i%7)-3,i%2==0,true};
   Simulation a(s,{float(i%13)-6,float(i%5),float(i%11)/10,float(i%3)-1});a.finish();
   check(a.result!=Outcome::Flying,"All shots terminate");
   check(std::isfinite(a.p.x)&&std::isfinite(a.p.y),"Finite state");
   check(a.frames.size()>1 && a.frames.back().time<5,"Bounded replay");
   const auto end=a.p; a.step();check(a.p.x==end.x && a.p.z==end.z,"Resolved shots do not change");
 }
 std::cout<<checks<<" checks passed\n";
}
