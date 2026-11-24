# Can You Score? — Free Kick Club

A complete desktop free-kick game written in C++17 with raylib. Aim at the goal, charge your shot, and bend the ball past a goalkeeper who learns your preferred corner. All stadium art is drawn in code. No accounts, external assets, music, or network connection are needed to play.

## Start on your Mac

1. Extract this ZIP. Open the `can-you-score` folder in VS Code.
2. Install the compiler once if needed: `xcode-select --install`.
3. Install CMake once: `brew install cmake` if you have Homebrew. Otherwise get CMake from https://cmake.org/download/ and enable its command-line tools.
4. Open VS Code's terminal inside the project and run:

```bash
bash Play.command
```

The script builds the game, runs the physics tests, and opens the window. Run the same command again whenever you want to play. After the first build you can launch directly with `./build/can_you_score`.

The ZIP contains source code, not a precompiled Mac app. raylib 5.5 source and its license are bundled, so CMake does not need to fetch the graphics library. Both Intel and Apple Silicon Macs compile from the same source.

## Controls

| Control | Action |
|---|---|
| Mouse / arrow keys | Aim at a point on or around the goal |
| Hold Space / left mouse | Charge power; release to shoot |
| Q / E | Adjust left / right spin |
| C | Reset spin |
| Enter / Space after a shot | Next shot |
| R after a shot | Slow-motion replay; R again returns |
| H | Help overlay |
| P | Pause / resume |
| M | Main menu (abandons current round) |
| Escape | Quit |
| W in practice | Toggle defensive wall |
| K in practice | Toggle goalkeeper |
| V in practice | Cycle wind from -2 to +2 |

The reticle is your **no-wind, no-spin target**. Wind and curve move the actual landing point away from it. The power meter oscillates while held. Higher power reduces flight time and gives the keeper less time to react. Start with no spin, then experiment with Q/E. A low, hard shot may hit the wall; a slower lofted shot can clear it. The full ball must fit inside the posts and below the bar.

## Modes and scoring

- **Practice:** Unlimited shots; optional wall, keeper, and wind.
- **Ten-shot challenge:** Distance varies from 18–24 m, wind changes every shot, and a defensive wall appears from shot four. Get the highest score in ten attempts.
- **Scoring:** 100 per goal, up to 50 bonus points for spin, and 20 per prior consecutive goal (streak bonus capped at 100).
- **Personal best:** Highest challenge score, saved locally in your home folder as `.can-you-score-best`. Practice does not update it. Delete that file to reset the record.
- **Goalkeeper:** A heuristic opponent, not a trained ML model. It uses a weighted history of your landing positions for its starting position, then reacts to flight after a delay with limited movement speed. Learning resets each new round.
- **Replay:** Recorded ball and goalkeeper states played at 0.35x speed with a trajectory overlay. Replay does not simulate or score the shot again.

## Other platforms

### Windows

Install CMake and Visual Studio Build Tools with the **Desktop development with C++** workload. Open `Play-Windows.bat`, or use the commands below and run `build\Release\can_you_score.exe`.

### Ubuntu / Debian

```bash
sudo apt install build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
bash Play.command
```

### Manual build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
./build/can_you_score
```

## Project structure

- `src/physics.hpp`: Pure C++ simulation, swept collisions, keeper behavior, and replay recording. Independent of graphics.
- `src/main.cpp`: Window, responsive letterboxed rendering, menus, inputs, scoring, and save file.
- `tests/physics_tests.cpp`: Collision/trajectory tests plus 1,000 varied shots checking finite states, termination, replay bounds, and frozen results.
- `vendor/raylib/`: Unmodified raylib 5.5 build sources and license.
- `CMakeLists.txt`: Builds the game and tests; uses installed raylib 5.5+ if available, bundled raylib otherwise.

## Physics and design

The world uses metres: x is across the goal, y is height, z is distance down the pitch. A fixed 240 Hz simulation integrates gravity, a simplified sideways spin force, and wind. Swept collision checks intersect the wall and goal planes so fast shots cannot jump through them. The view is a stylized perspective projection of this 3D state into 2D drawing primitives.

This is an arcade free-kick game, not a full football match simulator. The wall uses a continuous collision rectangle, the goalkeeper uses an elliptical reach region, and ball drag / realistic aerodynamics are simplified. Shots end at the first wall or goal-plane result; post rebounds and net physics are not simulated. The game is silent.

## Testing without a graphics display

```bash
cmake -S . -B build-tests -DBUILD_GAME=OFF
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

Or, with only a compiler:

```bash
c++ -std=c++17 -Isrc tests/physics_tests.cpp -o physics_tests
./physics_tests
```

For graphical smoke checks, `./build/can_you_score --smoke-test` automatically exercises shots, replay, practice with a wall, and a complete challenge; saves five PNG screenshots; and exits without writing a personal best.

