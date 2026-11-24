#include "raylib.h"
#include "physics.hpp"
#include <array>
#include <memory>
#include <string>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <iostream>
using namespace soccer;
namespace {
constexpr int W=1280,H=800;
const Color ink{12,22,30,255}, panel{20,33,42,255}, muted{139,160,170,255}, paper{235,244,239,255}, lime{205,250,91,255};
Vector2 mouse{};
void text(const char* s,int x,int y,int size,Color c=paper){DrawText(s,x,y,size,c);}
void box(float x,float y,float w,float h,Color c){DrawRectangleRounded({x,y,w,h},.13f,10,c);}
bool button(Rectangle r,const char* s,bool accent=false){
 bool hover=CheckCollisionPointRec(mouse,r);
 box(r.x,r.y,r.width,r.height,hover?Color{65,84,82,255}:accent?lime:Color{34,51,61,255});
 int size=18; text(s,int(r.x+(r.width-MeasureText(s,size))/2),int(r.y+(r.height-size)/2),size,accent&&!hover?ink:paper);
 return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}
Vector2 project(Vec p){float d=1+.04f*p.z;return {458+p.x*100/d,665-p.z*28/d-p.y*100/d};}
void line(Vec a,Vec b,Color c,float thick=2){DrawLineEx(project(a),project(b),thick,c);}
void person(float x,float z,float h,Color jersey,float lean=0){
 Vector2 feet=project({x,0,z});float s=100/(1+.04f*z);
 Vector2 hip{feet.x+lean*s*.25f,feet.y-h*s*.43f};
 Vector2 chest{feet.x+lean*s*.5f,feet.y-h*s*.72f};
 DrawLineEx({feet.x-s*.14f,feet.y},hip,s*.10f,ink);DrawLineEx({feet.x+s*.14f,feet.y},hip,s*.10f,ink);
 DrawLineEx(hip,chest,s*.30f,jersey);
 DrawLineEx({chest.x-s*.43f,chest.y+s*.12f},{chest.x+s*.43f,chest.y+s*.12f},s*.08f,jersey);
 DrawCircleV({chest.x,chest.y-s*.22f},s*.13f,Color{212,167,128,255});
}
std::filesystem::path savePath(){
 const char* base=std::getenv("HOME");
 if(!base)base=std::getenv("USERPROFILE");
 return (base?std::filesystem::path(base):std::filesystem::current_path())/".can-you-score-best";
}
}
int main(int argc,char** argv){
 bool smoke=argc>1 && std::string(argv[1])=="--smoke-test";
 SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
 InitWindow(W,H,"Can You Score? | Free Kick Club");
 if(!IsWindowReady()){std::cerr<<"Unable to open a graphics window. A desktop display and OpenGL 3.3 are required.\n";return 1;}
 SetWindowMinSize(960,600);SetTargetFPS(60);
 RenderTexture2D canvas=LoadRenderTexture(W,H);
 enum class State{Menu,Aim,Flight,Result,Replay,Summary}; State state=State::Menu;
 bool challenge=false,paused=false,help=false,charging=false;
 Setup setup;Shot shot; std::unique_ptr<Simulation> sim;
 float chargeTime=0,accumulator=0,replayTime=0,habit=0,clock=0;
 int attempts=0,goals=0,points=0,best=0,streak=0,smokeFrame=0;
 std::array<Outcome,10> results{}; std::ifstream(savePath())>>best; best=std::clamp(best,0,1000000);
 auto prepare=[&](){
   setup.distance=challenge?18.f+float(attempts%4)*2:20;
   setup.wind=challenge?float((attempts*7)%9-4)*.55f:setup.wind;
   setup.wall=challenge?attempts>=3:setup.wall;
   shot={2,1.5f,.65f,0};sim.reset();charging=false;accumulator=0;state=State::Aim;
 };
 auto start=[&](bool c){challenge=c;attempts=goals=points=streak=0;habit=0;setup={};prepare();};
 auto fire=[&](){sim=std::make_unique<Simulation>(setup,shot,habit);state=State::Flight;charging=false;accumulator=0;};
 auto advance=[&](){if(challenge && attempts>=10)state=State::Summary;else prepare();};
 while(!WindowShouldClose()){
  float delta=std::min(GetFrameTime(),.05f);clock+=delta;
  float scale=std::min(float(GetScreenWidth())/W,float(GetScreenHeight())/H);
  Vector2 offset{(GetScreenWidth()-W*scale)/2,(GetScreenHeight()-H*scale)/2};
  Vector2 raw=GetMousePosition();mouse={(raw.x-offset.x)/scale,(raw.y-offset.y)/scale};
  if(IsKeyPressed(KEY_H)){help=!help;charging=false;}
  if(IsKeyPressed(KEY_P) && state!=State::Menu){paused=!paused;charging=false;}
  if(IsKeyPressed(KEY_M)){state=State::Menu;paused=false;charging=false;}
  if(smoke){
   if(smokeFrame==1)start(true);
   if(smokeFrame==10)fire();
   if(smokeFrame==161){state=State::Replay;replayTime=0;}
   if(smokeFrame==220){start(false);setup.wall=true;setup.keeper=false;shot={0,.15f,1,0};fire();}
   if(smokeFrame==300)start(true);
   if(smokeFrame>300 && state==State::Aim)fire();
   if(smokeFrame>300 && state==State::Result)advance();
  }
  bool blocked=paused||help;
  if(!blocked){
   if(state==State::Menu){if(IsKeyPressed(KEY_ENTER))start(true);if(IsKeyPressed(KEY_F))start(false);}
   else if(state==State::Aim){
    float d=1+.04f*setup.distance;
    Vector2 ground=project({0,0,setup.distance});
    if(mouse.x<895 && mouse.y>150 && mouse.y<690 && (GetMouseDelta().x!=0||GetMouseDelta().y!=0)){
     shot.aimX=std::clamp((mouse.x-458)*d/100,-5.f,5.f);
     shot.aimY=std::clamp((ground.y-mouse.y)*d/100,.15f,4.f);
    }
    shot.aimX+=float(IsKeyDown(KEY_RIGHT)-IsKeyDown(KEY_LEFT))*delta*2;
    shot.aimY+=float(IsKeyDown(KEY_UP)-IsKeyDown(KEY_DOWN))*delta*2;
    shot.aimX=std::clamp(shot.aimX,-5.f,5.f);shot.aimY=std::clamp(shot.aimY,.15f,4.f);
    shot.spin=std::clamp(shot.spin+float(IsKeyDown(KEY_E)-IsKeyDown(KEY_Q))*delta,-1.f,1.f);
    if(IsKeyPressed(KEY_C))shot.spin=0;
    if(!challenge && IsKeyPressed(KEY_W))setup.wall=!setup.wall;
    if(!challenge && IsKeyPressed(KEY_K))setup.keeper=!setup.keeper;
    if(!challenge && IsKeyPressed(KEY_V))setup.wind=setup.wind>=2?-2:setup.wind+1;
    bool mousePitch=mouse.x>15&&mouse.x<890&&mouse.y>145&&mouse.y<700;
    if(IsKeyPressed(KEY_SPACE)||(mousePitch&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))){charging=true;chargeTime=0;}
    if(charging){chargeTime+=delta;shot.power=.15f+.85f*(.5f-.5f*std::cos(chargeTime*3.0f));
     if(IsKeyReleased(KEY_SPACE)||IsMouseButtonReleased(MOUSE_BUTTON_LEFT))fire();}
   }else if(state==State::Flight){
    accumulator+=delta;
    while(accumulator>=dt && sim->result==Outcome::Flying){sim->step();accumulator-=dt;}
    if(sim->result!=Outcome::Flying){
     if(challenge)results[attempts]=sim->result;
     ++attempts;
     if(sim->result==Outcome::Goal){++goals;++streak;points+=100+int(std::abs(shot.spin)*50)+std::min(streak-1,5)*20;}else streak=0;
     habit=.6f*habit+.4f*std::clamp(sim->p.x,-3.5f,3.5f);
     if(challenge&&points>best){best=points;if(!smoke)std::ofstream(savePath())<<best;}
     state=State::Result;
    }
   }else if(state==State::Result){
    if(IsKeyPressed(KEY_R)){state=State::Replay;replayTime=0;}
    else if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE))advance();
   }else if(state==State::Replay){
    replayTime+=delta*.35f;
    if(replayTime>sim->time+.6f||IsKeyPressed(KEY_R))state=State::Result;
   }else if(state==State::Summary){if(IsKeyPressed(KEY_ENTER))start(true);else if(IsKeyPressed(KEY_F))start(false);}
  }
  BeginTextureMode(canvas);ClearBackground(ink);
  // Editorial scoreboard header.
  box(24,24,46,46,lime);text("FC",31,36,25,ink);
  text("FREE KICK CLUB",84,26,15,muted);text("CAN YOU SCORE?",82,48,30);
  text("A LITTLE CURVE. A LOT OF NERVE.",435,42,17,muted);
  box(935,24,320,56,panel);text(TextFormat("PERSONAL BEST   %04i",best),960,43,20,lime);
  // Stadium terraces and floodlights, all procedural: no asset downloads.
  BeginScissorMode(24,106,872,594);
  DrawRectangleGradientV(24,106,872,320,Color{26,48,62,255},Color{69,97,101,255});
  for(int row=0;row<5;++row){
   DrawRectangle(24,170+row*20,872,15,Color{23,40,48,255});
   for(int col=0;col<60;++col)DrawRectangle(30+col*15,173+row*20,4,4,Color{(unsigned char)(60+(col*7+row*13)%50),100,104,255});
  }
  for(int x: {95,815}){DrawRectangle(x,120,3,125,muted);box(float(x-32),124,67,14,paper);}
  DrawRectangle(24,272,872,428,Color{38,92,71,255});
  for(int z=0;z<40;z+=4){Vector2 a=project({0,0,float(z)}),b=project({0,0,float(z+2)});DrawRectangle(24,int(b.y),872,int(a.y-b.y),Color{42,100,76,255});}
  Color chalk{171,210,183,180};
  line({-8,0,0},{-8,0,35},chalk);line({8,0,0},{8,0,35},chalk);
  float gz=setup.distance;
  line({-8,0,gz},{8,0,gz},chalk);
  line({-7,0,gz},{-7,0,gz-12},chalk);line({7,0,gz},{7,0,gz-12},chalk);line({-7,0,gz-12},{7,0,gz-12},chalk);
  // Goal depth, mesh, and frame.
  for(float x=-3.66f;x<=3.67f;x+=.3f)line({x,0,gz+1.6f},{x,2.44f,gz+1.6f},Fade(paper,.23f),1);
  for(float y=0;y<=2.45f;y+=.25f)line({-3.66f,y,gz+1.6f},{3.66f,y,gz+1.6f},Fade(paper,.23f),1);
  for(float x: {-3.66f,3.66f}){line({x,0,gz},{x,2.44f,gz},paper,5);line({x,2.44f,gz},{x,2.44f,gz+1.6f},paper,2);line({x,0,gz+1.6f},{x,2.44f,gz+1.6f},paper,2);}
  line({-3.66f,2.44f,gz},{3.66f,2.44f,gz},paper,5);
  Frame frame{{0,radius,0},habit*.55f,.85f,0}; size_t frameIndex=0;
  if(sim){frameIndex=sim->frames.size()-1;if(state==State::Replay)frameIndex=std::min(size_t(replayTime/dt),frameIndex);frame=sim->frames[frameIndex];}
  if(setup.keeper)person(frame.keeperX,gz-.15f,frame.keeperY+.85f,Color{247,174,74,255},frame.keeperX*.18f);
  if(setup.wall)for(float x:{-.8f,0.f,.8f})person(x,gz-9.15f,1.85f,Color{215,104,100,255});
  if(sim){
   size_t from=(state==State::Replay||state==State::Result)?0:(frameIndex>40?frameIndex-40:0);
   for(size_t i=from+3;i<=frameIndex;i+=3)DrawLineEx(project(sim->frames[i-3].ball),project(sim->frames[i].ball),2,Fade(lime,.5f));
  }
  Vector2 ball=project(frame.ball),shadow=project({frame.ball.x,0,frame.ball.z});float br=std::max(5.f,12/(1+.04f*frame.ball.z));
  DrawEllipse(int(shadow.x),int(shadow.y+3),br*1.4f,br*.4f,Fade(ink,.4f));
  DrawCircleV(ball,br,paper);DrawPoly(ball,5,br*.46f,clock*90,ink);
  DrawCircleLines(int(ball.x),int(ball.y),br,Fade(ink,.8f));
  if(state==State::Aim){
   Vector2 target=project({shot.aimX,shot.aimY,gz});
   DrawCircleLines(int(target.x),int(target.y),16,lime);
   DrawLineEx({target.x-24,target.y},{target.x-8,target.y},2,lime);DrawLineEx({target.x+8,target.y},{target.x+24,target.y},2,lime);
   DrawLineEx({target.x,target.y-24},{target.x,target.y-8},2,lime);DrawLineEx({target.x,target.y+8},{target.x,target.y+24},2,lime);
   text("AIM",int(target.x+25),int(target.y-7),14,lime);
  }
  EndScissorMode();
  box(42,121,challenge?190:163,32,ink);text(challenge?"TEN-SHOT CHALLENGE":"PRACTICE / NO LIMIT",54,130,14,lime);
  box(649,121,229,32,ink);text(TextFormat("%02im  /  WIND %+.1f",int(setup.distance),setup.wind),665,133,16,paper);
  // Shot controls and scorecard.
  box(918,106,338,594,panel);
  text("YOUR SCORE",942,132,15,muted);text(TextFormat("%04i",points),940,155,56,lime);
  text(TextFormat("%i GOALS  /  %i SHOTS",goals,attempts),942,220,18);
  if(challenge){for(int i=0;i<10;++i){Color c=i<attempts?(results[i]==Outcome::Goal?lime:Color{222,113,102,255}):Color{52,72,79,255};DrawCircle(953+i*29,263,8,c);}}
  text("01   PICK YOUR CORNER",942,302,18);text("Mouse or arrow keys to aim",942,330,16,muted);
  text("02   ADD SOME CURVE",942,375,18);text("Hold Q / E    -    C to reset",942,404,16,muted);
  DrawRectangle(944,442,284,4,Color{60,79,85,255});DrawCircle(int(1086+shot.spin*139),444,7,lime);
  text("LEFT",944,459,12,muted);text("RIGHT",1190,459,12,muted);
  text("03   FIND YOUR POWER",942,504,18);text("Hold SPACE or mouse. Release.",942,533,16,muted);
  box(942,568,286,21,ink);box(942,568,std::max(8.f,286*shot.power),21,charging?lime:Color{102,154,127,255});
  text(TextFormat("%i%%",int(shot.power*100)),1180,599,18,lime);
  text(charging?"RELEASE TO STRIKE":"READY WHEN YOU ARE",942,644,18,charging?lime:muted);
  text("Q / E  CURVE      SPACE  SHOOT      H  HELP      P  PAUSE      M  MENU",28,729,16,muted);
  text(challenge?"The keeper remembers. Change your corner.":"PRACTICE:  W  WALL     K  KEEPER     V  WIND",28,760,16,lime);
  if(state==State::Result||state==State::Replay){
   box(165,425,590,157,ink);
   text(state==State::Replay?"SLOW MOTION / 0.35x":label(sim->result),190,448,30,sim->result==Outcome::Goal?lime:paper);
   if(state==State::Result){
    if(!blocked && button({190,507,244,48},challenge&&attempts==10?"SEE YOUR RESULTS":"NEXT SHOT  [ENTER]",true))advance();
    if(!blocked && button({448,507,278,48},"WATCH REPLAY  [R]")){state=State::Replay;replayTime=0;}
   }else text("R to return  /  recorded flight + trajectory",190,510,18,muted);
  }
  if(state==State::Menu||state==State::Summary){
   DrawRectangle(24,106,872,594,Fade(ink,.84f));
   text(state==State::Menu?"THE 90TH MINUTE.":"FULL TIME.",76,205,52,lime);
   text(state==State::Menu?"One ball. Make it count.":TextFormat("%i goals. %i points. Your move.",goals,points),78,278,30);
   text(state==State::Menu?"Bend it past the wall. Outsmart the keeper.":"Every shot is a new chance to find the corner.",78,339,21,muted);
   text("Aim. Hold to charge. Release to strike.",78,374,21,muted);
   if(!blocked && button({78,451,330,60},state==State::Menu?"PLAY CHALLENGE  [ENTER]":"PLAY AGAIN  [ENTER]",true))start(true);
   if(!blocked && button({425,451,295,60},"FREE PRACTICE  [F]"))start(false);
   text("10 SHOTS  /  CHANGING WIND  /  ADAPTIVE KEEPER",78,551,17,muted);
   text("Built with C++  /  No accounts. Just football.",78,625,16,muted);
  }
  if(paused||help){
   DrawRectangle(0,0,W,H,Fade(ink,.87f));box(290,175,700,445,panel);
   text(help?"THE PLAYBOOK":"PAUSED",330,215,40,lime);
   const char* lines[]={"Mouse / arrows     Aim at the goal","Space / left click  Hold, then release to shoot","Q / E               Bend left / right; C resets","R                   Replay your last shot","Enter               Next shot / start challenge","W / K / V           Practice: wall / keeper / wind","P: pause   H: close help   M: main menu"};
   for(int i=0;i<7;++i)text(lines[i],330,283+i*39,19,i==6?lime:paper);
  }
  EndTextureMode();
  BeginDrawing();ClearBackground(BLACK);
  DrawTexturePro(canvas.texture,{0,0,float(W),-float(H)},{offset.x,offset.y,W*scale,H*scale},{0,0},0,WHITE);
  EndDrawing();
  if(smoke){
   ++smokeFrame;
   if(smokeFrame==5)TakeScreenshot("smoke-aim.png");
   if(smokeFrame==160)TakeScreenshot("smoke-result.png");
   if(smokeFrame==190)TakeScreenshot("smoke-replay.png");
   if(smokeFrame==290)TakeScreenshot("smoke-wall.png");
   if(state==State::Summary){TakeScreenshot("smoke-summary.png");std::cout<<"Graphical smoke test: replay, practice wall, ten-shot challenge complete.\n";break;}
   if(smokeFrame>1800){std::cerr<<"Smoke test timeout\n";UnloadRenderTexture(canvas);CloseWindow();return 2;}
  }
 }
 UnloadRenderTexture(canvas);CloseWindow();return 0;
}
