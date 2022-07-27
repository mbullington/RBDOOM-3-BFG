# Build system

In order to keep things more sane, the project was switched to only officially support Clang as a toolchain. This comes with multiple benefits:

- Overall simpler!
- Cross platform: Linux, Mac, and Windows via MSYS2/MINGW64
- We can statically link https://libcxx.llvm.org/ for more consistent performance when using external deps that use the STL.

The goal is also to split `src` into distinct directories with clear dependency paths:
- `src/idlib` - id's alternative to the STL
- `src/neo` - The actual engine itself
- `src/doom3bfg` - Game-specific code for Doom 3: player, player movement, weapons, AI, etc...
- `src/test` - The Catch2 test binary

These will all be handled by CMake and Ninja.