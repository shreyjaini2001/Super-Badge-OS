#include "games.h"

GameState current_game_state = GAME_MENU;

// --- GAMES MENU ---
const char* game_menu_items[] = {
    "Pong",
    "Snake",
    "Simon Says",
    "Flappy Badge"
};
const int GAME_MENU_COUNT = 4;
int game_menu_index = 0;

void render_games_menu(bool full_redraw) {
    if (full_redraw) {
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextSize(3);
        tft.setTextColor(ST77XX_MAGENTA);
        tft.setCursor((320 - (11 * 18)) / 2, 20);
        tft.println("ARCADE MENU");
    } else {
        tft.fillRect(0, 60, 320, 180, ST77XX_BLACK);
    }

    tft.setTextSize(2);
    for (int i = 0; i < GAME_MENU_COUNT; i++) {
        if (i == game_menu_index) {
            tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        } else {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        }
        tft.setCursor(60, 80 + (i * 30));
        tft.print(" "); tft.print(game_menu_items[i]); tft.print(" ");
    }
    tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 220);
    tft.print("B1:Up  B2:Down  B3:Select  B4:Exit");
}

// --- PONG ---
int paddle_y = 100;
int ai_y = 100;
float ball_x = 160;
float ball_y = 120;
float ball_dx = 2.5;
float ball_dy = 1.8;
int player_score = 0;
int ai_score = 0;
unsigned long pong_last_frame = 0;

void setup_pong() {
    tft.fillScreen(ST77XX_BLACK);
    tft.drawRect(0, 0, 320, 240, ST77XX_CYAN);
    tft.drawRect(1, 1, 318, 238, ST77XX_CYAN);
    player_score = 0;
    ai_score = 0;
    ball_x = 160; ball_y = 120;
    ball_dx = 2.5; ball_dy = 1.8;
    paddle_y = 100; ai_y = 100;
}

void loop_pong() {
    if (millis() - pong_last_frame < 16) return; // ~60 FPS
    pong_last_frame = millis();

    tft.fillRect((int)ball_x, (int)ball_y, 6, 6, ST77XX_BLACK);
    tft.fillRect(10, paddle_y, 6, 40, ST77XX_BLACK);
    tft.fillRect(304, ai_y, 6, 40, ST77XX_BLACK);

    if (state_b1) paddle_y -= 5;
    if (state_b2) paddle_y += 5;
    if (paddle_y < 2) paddle_y = 2;
    if (paddle_y > 196) paddle_y = 196;

    if (ai_y + 20 < ball_y - 10) ai_y += 3;
    if (ai_y + 20 > ball_y + 10) ai_y -= 3;
    if (ai_y < 2) ai_y = 2;
    if (ai_y > 196) ai_y = 196;

    ball_x += ball_dx;
    ball_y += ball_dy;

    if (ball_y <= 2 || ball_y >= 232) ball_dy = -ball_dy;

    if (ball_x <= 16 && ball_x >= 10 && ball_y + 6 >= paddle_y && ball_y <= paddle_y + 40) {
        ball_dx = -ball_dx; ball_x = 17;
        if (ball_dx < 7.0) ball_dx *= 1.15;
    }

    if (ball_x >= 298 && ball_x <= 304 && ball_y + 6 >= ai_y && ball_y <= ai_y + 40) {
        ball_dx = -ball_dx; ball_x = 297;
        if (ball_dx > -7.0) ball_dx *= 1.15;
    }

    if (ball_x < 0 || ball_x > 320) {
        if(ball_x < 0) ai_score++; else player_score++;
        ball_x = 160; ball_y = 120; ball_dx = (ball_x < 0) ? 2.5 : -2.5; ball_dy = 1.8;
        tft.fillScreen(ST77XX_BLACK);
        tft.drawRect(0, 0, 320, 240, ST77XX_CYAN);
        tft.drawRect(1, 1, 318, 238, ST77XX_CYAN);
    }

    for(int y=4; y<236; y+=16) tft.fillRect(158, y, 4, 8, ST77XX_CYAN);
    
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.setCursor((player_score < 10) ? 110 : 90, 20); tft.print(player_score);
    tft.setCursor(190, 20); tft.print(ai_score);

    tft.fillRect((int)ball_x, (int)ball_y, 6, 6, ST77XX_WHITE);
    tft.fillRect(10, paddle_y, 6, 40, ST77XX_MAGENTA);
    tft.fillRect(304, ai_y, 6, 40, ST77XX_RED);
}

// --- SNAKE ---
#define SNAKE_MAX 100
int16_t snake_x[SNAKE_MAX];
int16_t snake_y[SNAKE_MAX];
int snake_len = 3;
int snake_dx = 10;
int snake_dy = 0;
int16_t apple_x = 0;
int16_t apple_y = 0;
unsigned long snake_last_frame = 0;
bool snake_dead = false;
bool prev_b1 = false;
bool prev_b2 = false;

void spawn_apple() {
    apple_x = (random(1, 31) * 10);
    apple_y = (random(3, 23) * 10);
    tft.fillRect(apple_x, apple_y, 10, 10, ST77XX_RED);
}

void setup_snake() {
    tft.fillScreen(ST77XX_BLACK);
    tft.fillRect(0, 20, 320, 2, ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(5, 5); tft.print("Score: 0");
    tft.setCursor(220, 5); tft.print("B1: L | B2: R");
    
    snake_len = 3;
    for(int i=0; i<snake_len; i++) {
        snake_x[i] = 160 - (i*10);
        snake_y[i] = 120;
    }
    snake_dx = 10; snake_dy = 0;
    snake_dead = false;
    spawn_apple();
}

void loop_snake() {
    if (snake_dead) return;
    if (millis() - snake_last_frame < 100) return; // 10 FPS
    snake_last_frame = millis();

    // Controls (Relative turning to save buttons)
    if (state_b1 && !prev_b1) { // Turn Left (counter-clockwise)
        if (snake_dx != 0) { snake_dy = -snake_dx; snake_dx = 0; }
        else { snake_dx = snake_dy; snake_dy = 0; }
    }
    if (state_b2 && !prev_b2) { // Turn Right (clockwise)
        if (snake_dx != 0) { snake_dy = snake_dx; snake_dx = 0; }
        else { snake_dx = -snake_dy; snake_dy = 0; }
    }
    prev_b1 = state_b1; prev_b2 = state_b2;

    // Erase tail
    tft.fillRect(snake_x[snake_len-1], snake_y[snake_len-1], 10, 10, ST77XX_BLACK);

    // Shift body
    for(int i = snake_len - 1; i > 0; i--) {
        snake_x[i] = snake_x[i-1];
        snake_y[i] = snake_y[i-1];
    }
    
    snake_x[0] += snake_dx;
    snake_y[0] += snake_dy;

    // Wall collision
    if(snake_x[0] < 0 || snake_x[0] >= 320 || snake_y[0] < 22 || snake_y[0] >= 240) {
        snake_dead = true;
        tft.setTextSize(3); tft.setTextColor(ST77XX_RED);
        tft.setCursor(80, 110); tft.print("GAME OVER");
        return;
    }

    // Self collision
    for(int i=1; i<snake_len; i++) {
        if(snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
            snake_dead = true;
            tft.setTextSize(3); tft.setTextColor(ST77XX_RED);
            tft.setCursor(80, 110); tft.print("GAME OVER");
            return;
        }
    }

    // Apple collision
    if (snake_x[0] == apple_x && snake_y[0] == apple_y) {
        if(snake_len < SNAKE_MAX) snake_len++;
        tft.fillRect(0, 0, 100, 20, ST77XX_BLACK);
        tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(5, 5); tft.print("Score: "); tft.print(snake_len - 3);
        spawn_apple();
    }

    // Draw head
    tft.fillRect(snake_x[0], snake_y[0], 10, 10, ST77XX_GREEN);
}

// --- SIMON SAYS ---
uint8_t simon_seq[50];
int simon_len = 0;
int simon_step = 0;
bool simon_playback = false;
unsigned long simon_timer = 0;
bool simon_dead = false;
bool simon_b1=false, simon_b2=false, simon_b3=false;

void draw_simon_ui() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(80, 20); tft.print("SIMON SAYS");
    
    tft.setTextColor(ST77XX_GREEN); tft.setCursor(20, 100); tft.print("B1: Green");
    tft.setTextColor(ST77XX_YELLOW); tft.setCursor(20, 140); tft.print("B2: Yellow");
    tft.setTextColor(ST77XX_BLUE); tft.setCursor(20, 180); tft.print("B3: Blue");
    
    tft.setTextSize(2); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(200, 140); tft.print("Score: "); tft.print(simon_len);
}

void setup_simon() {
    simon_len = 1;
    simon_seq[0] = random(1, 4);
    simon_playback = true;
    simon_step = 0;
    simon_timer = millis() + 1000;
    simon_dead = false;
    strip.clear(); strip.show();
    draw_simon_ui();
}

void show_simon_led(uint8_t color, bool on) {
    strip.clear();
    if(on) {
        uint32_t c = 0;
        if(color == 1) c = strip.Color(0, 255, 0); // B1
        if(color == 2) c = strip.Color(255, 255, 0); // B2
        if(color == 3) c = strip.Color(0, 0, 255); // B3
        for(int i=0; i<16; i++) strip.setPixelColor(i, c);
    }
    strip.show();
}

void loop_simon() {
    if(simon_dead) return;

    if(simon_playback) {
        if(millis() > simon_timer) {
            if(simon_step >= simon_len) {
                simon_playback = false; // Player turn
                simon_step = 0;
                show_simon_led(0, false);
            } else {
                show_simon_led(simon_seq[simon_step], true);
                delay(400);
                show_simon_led(0, false);
                simon_step++;
                simon_timer = millis() + 100;
            }
        }
    } else {
        uint8_t pressed = 0;
        if(state_b1 && !simon_b1) pressed = 1;
        if(state_b2 && !simon_b2) pressed = 2;
        if(state_b3 && !simon_b3) pressed = 3;
        simon_b1 = state_b1; simon_b2 = state_b2; simon_b3 = state_b3;
        
        if(pressed > 0) {
            show_simon_led(pressed, true);
            delay(300);
            show_simon_led(0, false);
            
            if(pressed == simon_seq[simon_step]) {
                simon_step++;
                if(simon_step >= simon_len) {
                    if(simon_len < 50) simon_len++;
                    simon_seq[simon_len-1] = random(1, 4);
                    draw_simon_ui();
                    simon_playback = true;
                    simon_step = 0;
                    simon_timer = millis() + 1000;
                }
            } else {
                simon_dead = true;
                strip.clear();
                for(int i=0; i<16; i++) strip.setPixelColor(i, strip.Color(255,0,0));
                strip.show();
                tft.setTextSize(3); tft.setTextColor(ST77XX_RED);
                tft.setCursor(80, 200); tft.print("GAME OVER");
                delay(1000);
                strip.clear(); strip.show();
            }
        }
    }
}

// --- FLAPPY BADGE ---
float flappy_y = 120;
float flappy_dy = 0;
int pipe_x = 320;
int pipe_y = 100; 
int flappy_score = 0;
unsigned long flappy_last = 0;
bool flappy_dead = false;
bool flappy_b3_prev = false;

void setup_flappy() {
    tft.fillScreen(ST77XX_BLACK);
    flappy_y = 120; flappy_dy = 0;
    pipe_x = 320; pipe_y = random(60, 180);
    flappy_score = 0;
    flappy_dead = false;
}

void loop_flappy() {
    if (flappy_dead) return;
    if (millis() - flappy_last < 16) return;
    flappy_last = millis();

    // Erase old
    tft.fillRect(60, (int)flappy_y, 10, 10, ST77XX_BLACK);
    tft.fillRect(pipe_x, 0, 30, pipe_y - 30, ST77XX_BLACK);
    tft.fillRect(pipe_x, pipe_y + 30, 30, 240 - (pipe_y + 30), ST77XX_BLACK);

    if (state_b3 && !flappy_b3_prev) flappy_dy = -4.0; // Jump
    flappy_dy += 0.25; // Gravity
    flappy_y += flappy_dy;

    pipe_x -= 3;
    if (pipe_x < -30) {
        pipe_x = 320;
        pipe_y = random(60, 180);
        flappy_score++;
    }

    // Collisions
    if (flappy_y < 0 || flappy_y > 230 || 
        (60 + 10 > pipe_x && 60 < pipe_x + 30 && (flappy_y < pipe_y - 30 || flappy_y + 10 > pipe_y + 30))) {
        flappy_dead = true;
        tft.setTextSize(3); tft.setTextColor(ST77XX_RED); 
        tft.setCursor(80, 100); tft.print("GAME OVER");
        return;
    }

    // Draw new
    tft.fillRect(60, (int)flappy_y, 10, 10, ST77XX_YELLOW);
    tft.fillRect(pipe_x, 0, 30, pipe_y - 30, ST77XX_GREEN);
    tft.fillRect(pipe_x, pipe_y + 30, 30, 240 - (pipe_y + 30), ST77XX_GREEN);
    
    // Erase artifact trail from previous frame
    tft.fillRect(pipe_x + 30, 0, 4, 240, ST77XX_BLACK);

    tft.setTextSize(2); tft.setCursor(10, 10);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.print(flappy_score);
    
    flappy_b3_prev = state_b3;
}

// --- STATE MANAGEMENT ---
bool btn1_prev = false;
bool btn2_prev = false;
bool btn3_prev = false;
bool btn4_prev = false;


extern bool btn1_prev, btn2_prev, btn3_prev, btn4_prev;

void switch_game_state(GameState state) {
    current_game_state = state;
    if (state == GAME_MENU) {
        btn1_prev = true; btn2_prev = true; flappy_b3_prev = true; btn4_prev = true;
        render_games_menu(true);
    }
    else if (state == GAME_PONG) setup_pong();
    else if (state == GAME_SNAKE) setup_snake();
    else if (state == GAME_SIMON) setup_simon();
    else if (state == GAME_DOOM) setup_flappy();
}



void loop_games() {
    if (state_b4 && !btn4_prev) {
        if (current_game_state == GAME_MENU) {
            set_mode_menu(); // Exit games entirely
        } else {
            switch_game_state(GAME_MENU); // Exit current game
        }
    }
    btn4_prev = state_b4;


    if (current_game_state == GAME_MENU) {
        if (state_b1 && !btn1_prev) {
            game_menu_index--;
            if(game_menu_index < 0) game_menu_index = GAME_MENU_COUNT - 1;
            render_games_menu(false);
        }
        if (state_b2 && !btn2_prev) {
            game_menu_index++;
            if(game_menu_index >= GAME_MENU_COUNT) game_menu_index = 0;
            render_games_menu(false);
        }
        if (state_b3 && !flappy_b3_prev) {
            if (game_menu_index == 0) switch_game_state(GAME_PONG);
            if (game_menu_index == 1) switch_game_state(GAME_SNAKE);
            if (game_menu_index == 2) switch_game_state(GAME_SIMON);
            if (game_menu_index == 3) switch_game_state(GAME_DOOM);
        }
        btn1_prev = state_b1; btn2_prev = state_b2; flappy_b3_prev = state_b3;
    } 
    else if (current_game_state == GAME_PONG) loop_pong();
    else if (current_game_state == GAME_SNAKE) loop_snake();
    else if (current_game_state == GAME_SIMON) loop_simon();
    else if (current_game_state == GAME_DOOM) loop_flappy();
}

void launch_game_by_name(String name) {
    if(name == "pong") switch_game_state(GAME_PONG);
    else if(name == "snake") switch_game_state(GAME_SNAKE);
    else if(name == "simon") switch_game_state(GAME_SIMON);
    else if(name == "flappy") switch_game_state(GAME_DOOM);
}
