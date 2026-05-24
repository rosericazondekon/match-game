# Match-3

An implementation of the Match-3 game written in C with [raylib](https://www.raylib.com).
Swap adjacent tiles, trigger cascades, stack points, and enjoy animated score popups with music and SFX.

## Highlights

- 12x12 board with randomized tile generation
- Adjacent-tile swap logic with validation
- Horizontal and vertical match detection
- Cascade resolution with falling-tile animation
- Match preview state before tiles clear
- Floating score popups and score scale animation
- Background image, custom font, background music, and match sound

## Tech Stack
- C programming language
- raylib

## Controls
- Left click a tile to select it
- Left click an adjacent tile to attempt a swap
- Valid swaps that create matches score points
- Close the window to quit

## Scoring
- Each 3-tile match awards `+10`
- Cascades can trigger additional matches automatically

## Prerequisites
Before building, install:

- GCC
- [raylib](https://www.raylib.com)
- pkg-config (recommended, for portable build flags)

Make sure raylib development headers and libraries are available to your compiler.

## Build

From the project root:

```bash
gcc -std=c11 -O2 -Wall -Wextra match.c -o match $(pkg-config --cflags --libs raylib)
```

If pkg-config is not available, compile by manually providing your raylib include and library paths:

```bash
gcc -std=c11 -O2 -Wall -Wextra match.c -o match -I<raylib_include_path> -L<raylib_lib_path> -lraylib
```

## Run

On Linux/macOS:

```bash
./match
```

On Windows:

```bash
match.exe
```
