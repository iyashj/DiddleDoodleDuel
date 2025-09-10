
# Diddle Doodle Duel

Diddle Doodle Duel is a simple multiplayer action game where the user who paints the most amount of canvas by the end of the round wins the game.

![HumbleEngine](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Prerequisites
- C++23-compatible compiler (GCC, Clang, or MSVC)
- CMake >= 3.20
- Conan >= 2.0
- Ninja (recommended)

## Quick Start

1. Install dependencies with Conan:
   ```sh
   conan install . --output-folder=build --build=missing
   ```
2. Configure the project with CMake (using presets):
   ```sh
   cmake --preset=default
   ```
3. Build:
   ```sh
   cmake --build --preset=default
   ```

For debug builds, use the `debug` preset:
```sh
cmake --preset=debug
cmake --build --preset=debug
```

## Project Structure

- `src/` — Game implementation files
- `include/` — Public headers
- `humble-engine/` — Submodule: C++23 utility/game engine
- `cmake/` — Custom CMake scripts
- `tests/` — Unit tests

## Adding Dependencies
Add dependencies to `conandata.yml` and manage them via Conan.

## License
MIT (see LICENSE file)
