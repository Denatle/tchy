# Tchy - Touchdesigner wannabe clone

C++20, ImGui, SDL3, OpenGL and FFmpeg libraries.

## Build on macOS

```zsh
./build_macos.sh.sh
open build/touch.app
```

`cmake --build build --target format` and `format-check` use the checked-in `.clang-format` when clang-format is installed.

The script installs Homebrew dependencies, downloads ImGui **v1.91.9b**, configures a Release build and runs tests. It uses zsh. Dependencies already installed on this machine allow an offline rebuild:

```zsh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`vendor/imgui` can also be supplied with `-DFETCHCONTENT_SOURCE_DIR_IMGUI=/path/to/imgui`. SDL3 >=3.2, json-c and FFmpeg development libraries are discovered through pkg-config. System dependency versions are supplied by the OS package manager; CMake prints the resolved versions.

Without a graphics stack, build the engine and plugin SDK with `-DTOUCH_BUILD_UI=OFF`:

```zsh
./build/touch_headless '../touch/scenes/packet-slip/Packet Slip.touch.json'
```

## Windows and Linux

Linux requires a C++20 compiler, CMake >=3.24, pkg-config, SDL3 >=3.2, json-c, OpenGL and development packages for libavformat, libavcodec, libavutil, libavdevice, libswscale and libswresample. Build with the same CMake commands.

Windows uses the MSYS2 CLANG64 environment. Install `mingw-w64-clang-x86_64-{clang,cmake,ninja,pkgconf,sdl3,json-c,ffmpeg}` with pacman and configure with `-G Ninja`. Run from CLANG64 so dependent DLLs are available on PATH. The root `.github/workflows/rewrite.yml` contains exact CI commands for all three systems. Windows and Linux jobs have not been run locally.

The macOS `.app` currently uses installed SDL3/FFmpeg/json-c libraries.

## Editor

- Run / Stop, Mute and monitoring volume; project resolution and frame rate apply on the next Run.
- Shift+A, F3 or Tab opens the operator catalog. Tabs filter TOP, CHOP, DAT and conversions. Search supports arrows and Enter.
- Drag nodes; Shift+click extends selection; drag empty space for box selection. G/S/R transforms the selection, X/Y constrain an axis, Escape cancels. Middle mouse or Space+drag pans; wheel zooms; F/Home frames the graph and `.` frames selection.
- Drag between ports or onto compatible node bodies. Incompatible signals offer a visible converter. Drag to empty space to create a connected operator. Right-click a wire to cut it or insert the selected compatible node; Shift+right-drag inserts reroutes, Ctrl+right-drag cuts crossed wires.
- Cmd+C/X/V, Cmd+Z, Cmd+Shift+Z, Cmd+N/O/S and Cmd+Shift+S on macOS; Ctrl is the command modifier on other systems. Shift+D duplicates, Delete removes, N toggles the inspector.
- Files can be dropped onto the canvas. Use the inspector to edit parameters, choices, media paths, tables and channel selection. Parameters expose Control inputs for modulation.
- Load Plugin adds `.dylib`, `.so` or `.dll` operators immediately. Settings → Stop and unload plugins releases instances before unloading. Reload a replacement library afterwards.

