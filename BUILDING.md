# Building Zoosters

Requires CMake 3.24+.

Variants:

| preset | compiler | renderer | status |
|---|---|---|---|
| `macos-arm64` | Apple clang | OpenGL 3.3 (SDL3) | tested |
| `linux-x64` | clang | OpenGL 3.3 (SDL3) | untested |
| `win-x86`, `win-x64` | MSVC 2022 | Direct3D 9 | untested |
| `win-x86-gl3`, `win-x64-gl3` | MSVC 2022 | OpenGL 3.3 (SDL3) | untested |

## Dependencies
- **librw**: submodule `vendor/librw` ([zoosters/librw](https://github.com/zoosters/librw), branch `zoosters`).
  Clone with `--recursive` or run `git submodule update --init`.
- **SDL3**: uses the installed one, otherwise fetched.
- **OpenAL Soft**: uses the installed one, otherwise fetched. Optional at run time; without it there is no sound.

macOS: `brew install cmake ninja sdl3 openal-soft`

## Build
macOS:
```
cmake --preset macos-arm64
cmake --build --preset macos-arm64
```
→ `build/macos-arm64/Zoosters.app`

Windows:
```
cmake --preset win-x64
```
→ Visual Studio solution in `build/win-x64`

Linux:
```
cmake --preset linux-x64
cmake --build --preset linux-x64
```
→ `build/linux-x64/zoosters`

## Options
| option | default | |
|---|---|---|
| `ZOOSTERS_TIER` | `qol` | `vanilla`: the original game. `fixes`: + bug fixes. `qol`: + widescreen, desktop resolution, in-game options, mouse look; settings in `settings.ini` instead of the registry |
| `ZOOSTERS_OS` | `win32` on Windows, else `sdl` | OS layer |
| `ZOOSTERS_PLATFORM` | `d3d9` on Windows, else `gl3` | renderer: `d3d9`, `gl3` or `null` (headless) |
| `ZOOSTERS_WINDOW` | `sdl` with `gl3`, else `win32` | window and input |
| `ZOOSTERS_SETTINGS` | `file` with `qol` or `sdl`, else `registry` | settings storage |
| `ZOOSTERS_BINK` | `ON` | movies (Bink 1 decoder, `src/port/bink`); `OFF`: movies are skipped |
| `ZOOSTERS_BINK_LOGOS` | `OFF` | also the four startup logo movies, as the original |
| `ZOOSTERS_DEBUGUI` | `OFF` | debug overlay (F1) |
| `ZOOSTERS_DEV_HARNESS` | `OFF` | trace and test hooks (`src/dev`) |
| `ZOOSTERS_LIBRW_DIR` | `vendor/librw` | librw source tree |

All build types use `-O2` / `/O2`.

## Running
Start `Zoosters.app` (or `zoosters`). On first start it asks for your Madagascar install folder.
Saves, `settings.ini` and the log: [README](README.md#your-files).
