#!/bin/zsh
set -euo pipefail
cd "${0:A:h}"
if [[ "$(uname)" == Darwin ]]; then
 command -v brew >/dev/null || { print 'Install Homebrew first: https://brew.sh'; exit 1; }
 brew install cmake pkg-config sdl3 json-c ffmpeg
elif [[ "$(uname)" == Linux ]]; then
 print 'Required: CMake >=3.24, C++20 compiler, pkg-config, SDL3 >=3.2, json-c, FFmpeg development libraries, OpenGL development headers.'
fi
mkdir -p vendor
if [[ ! -f vendor/imgui/imgui.h ]]; then
 archive=$(mktemp)
 trap 'rm -f "$archive"' EXIT
 curl --fail --location --retry 3 https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz -o "$archive"
 tar -xzf "$archive" -C vendor
 mv vendor/imgui-1.91.9b vendor/imgui
fi
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_IMGUI="$PWD/vendor/imgui" -DCMAKE_BUILD_TYPE=Release -DTOUCH_BUILD_UI=ON -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
print 'Build complete. Run build/touch.app/Contents/MacOS/touch on macOS, or build/touch on Linux.'
