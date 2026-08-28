# Arcade Games Expansion (Snake, Doom 3D, Simon)

This plan expands the badge's gaming capabilities by introducing a dedicated Games Sub-Menu and three new memory-efficient games.

## User Review Required

> [!IMPORTANT]
> The ESP32-C3 has very limited RAM. To prevent crashes, all games will strictly avoid dynamic memory allocation (`malloc`/`new` in loops) and rely on pre-allocated static arrays for game state (e.g., Snake body segments, Raycaster depth buffers).

## Proposed Changes

### 1. Games Sub-Menu Architecture
We will update `main.cpp` to introduce a sub-menu state for Games.
- When selecting "Games" from the main menu, you will enter `MODE_GAMES_MENU`.
- **B1/B2** will scroll through the games: `Pong`, `Snake`, `Doom 3D`, `Simon`.
- **B3** will launch the selected game.
- **B4** will exit back to the OS Main Menu.

### 2. Snake Game (`MODE_GAME_SNAKE`)
- A classic Snake game on the 320x240 display.
- **Controls**: Since we only have 4 buttons (and B4 must be Exit), we will use a relative control scheme: 
  - **B1**: Turn Left
  - **B2**: Turn Right
- **Memory**: A static array of `int16_t snake_x[100]` and `snake_y[100]` to track the body segments, capping the max length at 100 to save RAM.

### 3. Simon Says - Unique Hardware Game (`MODE_GAME_SIMON`)
- A game that uniquely utilizes the 16 NeoPixel ring.
- The badge will flash a sequence on the NeoPixels: Left side (B1), Bottom side (B2), Right side (B3).
- The player must repeat the sequence using the tactile buttons.
- The sequence length increases each round until the player makes a mistake.

### 4. Doom 3D Raycaster (`MODE_GAME_DOOM`)
- A lightweight, custom-built 3D Raycaster engine written from scratch in C++.
- Will render a pseudo-3D maze using vertical strips.
- **Controls**: B1 (Turn Left), B2 (Turn Right), B3 (Move Forward).
- **Memory**: Uses a highly compressed static `8x8` 2D integer array for the map, taking less than 100 bytes of RAM.

## Verification Plan

### Manual Verification
- Compile and flash the new firmware via PlatformIO.
- Navigate the new Games Menu using the physical buttons.
- Test each game individually to ensure memory doesn't leak (no crashes when switching between games repeatedly).
- Verify the Python App's "Game Launch" buttons still correctly route to the new state enums.
