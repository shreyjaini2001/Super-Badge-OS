#ifndef GAMES_H
#define GAMES_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_ST7789 tft;
extern Adafruit_NeoPixel strip;

extern bool state_b1;
extern bool state_b2;
extern bool state_b3;
extern bool state_b4;

extern void set_mode_menu(); // We will define this in main.cpp

// Game States
enum GameState {
    GAME_MENU,
    GAME_PONG,
    GAME_SNAKE,
    GAME_SIMON,
    GAME_DOOM
};

extern GameState current_game_state;
void switch_game_state(GameState state);
void render_games_menu();
void loop_games();
void launch_game_by_name(String name);

#endif
