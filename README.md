[README.md](https://github.com/user-attachments/files/28652871/README.md)
# Project-SNAKE# Snake Arena Game — Abdallah's Edition

A Snake game built with **C++17** and **SFML 3.x**.

---

## Features

| Feature | Details |
|---|---|
| Multiple screens | Start, Playing, Game Over, Leaderboard |
| Level system | Speed increases every 5 apples |
| Persistent leaderboard | Saved to `leaderboard.txt` |
| Visual flash effect | Screen flashes when an apple is eaten |
| Animated snake | Eyes follow direction of movement |
| Custom assets | Supports `menu.png` and `background.png` |

---

## Requirements

- CMake ≥ 3.20
- C++17 compiler (MinGW-W64 x86_64 recommended on Windows)
- SFML 3.1.0 (place in project folder or provide `-DSFML_DIR`)
- A TTF font file (Arial or DejaVu Sans)

---

## Build Instructions

```bash
# 1. Enter project folder
cd "the game"

# 2. Configure (adjust SFML path as needed)
cmake .. -DSFML_DIR="C:/Game.cpp/the game/SFML-3.1.0/lib/cmake/SFML" -G "MinGW Makefiles"

# 3. Build
cmake --build .

# 4. Run tests
ctest --output-on-failure

# 5. Play
SnakeArena.exe
```

> **If you get DLL errors when launching**, copy the SFML DLLs next to the exe:
> ```bash
> copy "SFML-3.1.0\bin\*.dll" .
> ```

---

## Optional Command-Line Arguments

```
SnakeArena.exe [leaderboard_file] [menu_png] [background_png] [font1] [font2]
```

Example:
```bash
SnakeArena.exe scores.txt menu.png background.png arial.ttf
```

All arguments are optional — defaults are `leaderboard.txt`, `menu.png`, `background.png`, and system fonts.

---

## Controls

| Key | Action |
|---|---|
| W A S D | Move snake |
| Enter | Start game |
| R | Retry after game over |
| L | Open leaderboard |
| ESC | Exit |

---

## Project Structure

```
the game/
├── src/
│   └── main.cpp          # Game source with Doxygen comments
├── tests/
│   ├── test_logic.cpp    # Doctest unit tests (no SFML needed)
│   └── doctest.h         # Doctest single-header library
├── SFML-3.1.0/           # SFML library folder
├── menu.png              # Start screen background
├── background.png        # Gameplay background
├── CMakeLists.txt
├── Doxyfile
├── .clang-format
└── README.md
```

---

## Running Tests

```bash
ctest --output-on-failure
```

Tests cover:
- `hasSnake` — positive and negative cases
- `delay` — base value, level scaling, minimum clamp
- `addLB` — sorting, size cap, rejection of low scores
- `calcLevel` — level advancement logic
- `calcScore` — score multiplier per level

---

## Generating Documentation

```bash
doxygen Doxyfile
# Output: docs/html/index.html
```
