cmake_minimum_required(VERSION 3.24)
project(TouchRewrite VERSION 0.2.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(TOUCH_BUILD_UI "Build the Dear ImGui editor (requires ImGui sources/network)" ON)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(SDL REQUIRED IMPORTED_TARGET sdl3>=3.2)
pkg_check_modules(JSON REQUIRED IMPORTED_TARGET json-c)
pkg_check_modules(AV REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale libswresample libavdevice)
add_library(touch_core
  src/data/samples.cpp
  src/settings/repository.cpp
  src/audio/output.cpp
  src/editor/document.cpp
  src/editor/demo.cpp
  src/engine/runtime.cpp
  src/engine/evaluate.cpp
  src/engine/plan.cpp
  src/graph/graph.cpp
  src/graph/routing.cpp
  src/graph/types.cpp
  src/media/media.cpp
  src/media/decoder.cpp
  src/media/decode.cpp
  src/media/timeline.cpp
  src/media/microphone.cpp
  src/operators/audio.cpp
  src/operators/audio/filter.cpp
  src/operators/audio/mixer.cpp
  src/operators/audio/oscillator.cpp
  src/operators/audio/playback.cpp
  src/operators/channels.cpp
  src/operators/control.cpp
  src/operators/effects.cpp
  src/operators/generators.cpp
  src/operators/masks.cpp
  src/operators/module.cpp
  src/operators/factory.cpp
  src/operators/registry.cpp
  src/operators/spatial.cpp
  src/plugins/plugins.cpp
  src/plugins/operator.cpp
  src/projects/codec.cpp
  src/projects/repository.cpp
)
target_include_directories(touch_core PUBLIC include)
target_link_libraries(touch_core PUBLIC Threads::Threads PkgConfig::SDL PkgConfig::JSON PkgConfig::AV)
add_executable(touch_headless src/headless.cpp)
target_link_libraries(touch_headless PRIVATE touch_core)
add_library(touch_plugin_sdk INTERFACE)
target_include_directories(touch_plugin_sdk INTERFACE include)
add_library(example_nodes MODULE plugins/example.cpp)
target_link_libraries(example_nodes PRIVATE touch_plugin_sdk)
set_target_properties(example_nodes PROPERTIES PREFIX "" SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
include(cmake/ui.cmake)
include(CTest)
if(BUILD_TESTING)
    include(cmake/tests.cmake)
endif()
find_program(CLANG_FORMAT_PROGRAM NAMES clang-format HINTS /opt/homebrew/opt/llvm/bin)
if(CLANG_FORMAT_PROGRAM)
 file(GLOB_RECURSE OWNED_CPP CONFIGURE_DEPENDS include/*.hpp include/*.h src/*.cpp src/*.hpp plugins/*.cpp tests/*.cpp)
 add_custom_target(format COMMAND ${CLANG_FORMAT_PROGRAM} -i ${OWNED_CPP})
 add_custom_target(format-check COMMAND ${CLANG_FORMAT_PROGRAM} --dry-run --Werror ${OWNED_CPP})
endif()
