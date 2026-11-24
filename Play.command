#!/bin/bash
set -e
cd "$(dirname "$0")"
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
if ! command -v cmake >/dev/null; then
  echo "CMake is needed once. If you use Homebrew, run: brew install cmake"
  echo "Or install CMake from https://cmake.org/download/ and add it to PATH."
  read -r -p "Press Enter to close."
  exit 1
fi
if [[ "$(uname)" == Darwin ]] && ! xcrun --find clang++ >/dev/null 2>&1; then
  echo "Install Apple's compiler with: xcode-select --install"
  read -r -p "Press Enter to close."
  exit 1
fi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
exec ./build/can_you_score
