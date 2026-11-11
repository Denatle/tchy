# Validation status

Locally built with AppleClang 17, SDL3 3.4.20, json-c 0.19 and FFmpeg 9 development libraries on macOS arm64.

Automated checks cover:

- Graph type compatibility, rollback after a rejected cycle and dependency ordering.
- All 77 operator output contracts using deterministic signal fixtures.
- Transparent alpha blending and Scan Once edge-trigger behavior.
- Packed f16 precision, u8 range, storage size and copy independence.
- Native plugin loading while Run is active, execution, synchronous Stop and restart, output resource release.
- Packet Slip v1 scene execution and JSON v2 round-trip with preserved edges.
- Generated FFmpeg video/audio fixtures with delayed audio and a video-only file.
- ImGui frame generation and draw data without a native window.

The normal window process aborted when launched from the restricted command environment. The ImGui test verifies layout execution, not OpenGL rendering or input behavior. A desktop smoke run remains necessary. Windows and Linux CI definitions are supplied; no remote CI run has been triggered or claimed.

Manual acceptance: open Packet Slip, Run, inspect the visual result, exercise graph gestures, save/reopen, drop media, unmute, verify camera/microphone access and repeated Stop/Run. Test a plugin load during playback and replacement after Stop on each OS. The .app links system package-manager libraries and requires them at runtime.

Release CPU benchmark of Packet Slip (1280×720) on this machine: cold first frame 0.146 s; subsequent 100 audio ticks / 30 video frames completed in 0.817 s, approximately 36.7 video FPS. This excludes the window, texture uploads and real audio device. It is not a measurement of end-to-end preview latency or a comparison against the Rust renderer. Reproduce with:

```zsh
./build/touch_headless '../touch/scenes/packet-slip/Packet Slip.touch.json' --benchmark
```

Final local checks: Release CTest 10/10 passed; headless AddressSanitizer + UndefinedBehaviorSanitizer CTest 9/9 passed; clang-format check passed. The sanitizer suite omits the ImGui/window target.
