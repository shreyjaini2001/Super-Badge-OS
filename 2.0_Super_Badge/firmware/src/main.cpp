#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include "tvbgone.h"
#include "ble_manager.h"
#include <TOTP.h>

// --- Constants & Pins ---
#define LED_PIN     8
#define NUM_LEDS    16
#define TFT_CS      1
#define TFT_DC      0
#define TFT_RST     -1 
#define TFT_MOSI    7
#define TFT_SCLK    6
#define SCREEN_W    320
#define SCREEN_H    240
#define I2C_SDA     20
#define I2C_SCL     21
#define PCF8574_ADDR 0x20
#define BTN_B3      10
#define BTN_B4      9

// --- Globals ---
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Preferences prefs;

// LED State
int led_r = 0, led_g = 255, led_b = 0;
int led_brightness = 80;
String current_pattern = "Solid";
unsigned long last_pattern_update = 0;
uint16_t pattern_step = 0;

// Button State
bool pcf_ready = false;
bool state_b1 = false;
bool state_b2 = false;
bool state_b3 = false;
bool state_b4 = false;

// Nametag State
String current_text = "";
int text_size = 3;
int text_r = 255, text_g = 255, text_b = 255;
String text_align = "center";
bool show_image_mode = false;

// TOTP State
String totp_names[6];
String totp_secrets[6];
int current_totp_slot = -1;
unsigned long time_offset = 0;
String last_totp_code = "";
File imgFile;

// App States
enum AppState {
    MODE_MENU,
    MODE_NAMETAG,
    MODE_TVBGONE,
    MODE_TOTP,
    MODE_GAMES
};
AppState current_state = MODE_MENU;

// Menu
const char* menu_items[] = {
    "Nametag",
    "TV-B-Gone",
    "TOTP Tokens",
    "Games"
};
const int MENU_COUNT = 4;
int menu_index = 0;

const char* patterns[] = {"Solid", "Rainbow", "Breathe", "Theater Chase", "Mixed Cylon", "Mixed Twinkle", "Rainbow Sparkle"};
const int PATTERN_COUNT = 7;
int pattern_index = 0;

// Function Prototypes
void render_menu();
void render_nametag();
void render_tvbgone();
void render_totp();
void render_games();
void load_state();
void save_state();

// --- Base32 Decoder ---
int base32_decode(const char *encoded, uint8_t *result, int bufSize) {
    int buffer = 0;
    int bitsLeft = 0;
    int count = 0;
    for (const char *ptr = encoded; count < bufSize && *ptr; ++ptr) {
        uint8_t ch = *ptr;
        if (ch == ' ' || ch == '-' || ch == '\r' || ch == '\n' || ch == '=') continue;
        ch = toupper(ch);
        int val = -1;
        if (ch >= 'A' && ch <= 'Z') val = ch - 'A';
        else if (ch >= '2' && ch <= '7') val = ch - '2' + 26;
        if (val < 0) break;
        buffer = (buffer << 5) | val;
        bitsLeft += 5;
        if (bitsLeft >= 8) {
            result[count++] = (buffer >> (bitsLeft - 8)) & 0xFF;
            bitsLeft -= 8;
        }
    }
    return count;
}

// --- LED Logic ---
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void updateLEDs() {
  if (current_state == MODE_TOTP && time_offset > 0 && current_totp_slot != -1 && totp_secrets[current_totp_slot] != "") {
      unsigned long current_time = (millis() / 1000) + time_offset;
      int seconds_left = 30 - (current_time % 30);
      int leds_to_light = (seconds_left * NUM_LEDS) / 30;
      
      strip.clear();
      uint32_t color = strip.Color(0, 255, 0); // Green
      if (seconds_left <= 5) color = strip.Color(255, 0, 0); // Red
      else if (seconds_left <= 10) color = strip.Color(255, 255, 0); // Yellow
      
      for(int i=0; i<leds_to_light; i++) strip.setPixelColor(i, color);
      strip.show();
      return; 
  }

  if (current_pattern == "Solid") return;
  if (millis() - last_pattern_update > 20) { // Default fast loop
      
      if (current_pattern == "Rainbow") {
          last_pattern_update = millis();
          for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, Wheel(((i * 256 / NUM_LEDS) + pattern_step) & 255));
          strip.show();
          pattern_step = (pattern_step + 1) % 256;
      }
      else if (current_pattern == "Breathe") {
          last_pattern_update = millis();
          float val = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
          strip.setBrightness(val);
          for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
          strip.show();
      }
      else if (current_pattern == "Theater Chase") {
          if (millis() - last_pattern_update > 50) {
              last_pattern_update = millis();
              strip.clear();
              for(int i=0; i<NUM_LEDS; i+=3) strip.setPixelColor(i + (pattern_step % 3), strip.Color(led_r, led_g, led_b));
              strip.show();
              pattern_step++;
          }
      }
      else if (current_pattern == "Mixed Cylon") {
          if (millis() - last_pattern_update > 50) {
              last_pattern_update = millis();
              strip.clear();
              int pos = pattern_step % (NUM_LEDS * 2);
              if (pos >= NUM_LEDS) pos = (NUM_LEDS * 2) - 1 - pos;
              strip.setPixelColor(pos, Wheel((pattern_step * 5) & 255));
              strip.show();
              pattern_step++;
          }
      }
      else if (current_pattern == "Mixed Twinkle") {
          if (millis() - last_pattern_update > 100) {
              last_pattern_update = millis();
              strip.clear();
              for(int i=0; i<NUM_LEDS; i++) {
                  if(random(10) > 7) {
                      strip.setPixelColor(i, Wheel(random(255)));
                  }
              }
              strip.show();
          }
      }
      else if (current_pattern == "Rainbow Sparkle") {
          last_pattern_update = millis();
          strip.clear();
          strip.setPixelColor(random(NUM_LEDS), Wheel(random(255)));
          strip.show();
      }
  }
}

// --- Text Drawing ---
void drawWordWrappedText(String text, int size, int r, int g, int b, String align) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(size);
  tft.setTextColor(tft.color565(r, g, b), ST77XX_BLACK);
  
  int char_w = 6 * size;
  int char_h = 8 * size;
  int max_chars_per_line = SCREEN_W / char_w;
  
  String lines[30];
  int line_cnt = 0;
  
  int start_idx = 0;
  while (start_idx < text.length() && line_cnt < 30) {
    int end_idx = start_idx + max_chars_per_line;
    if (end_idx >= text.length()) {
      lines[line_cnt++] = text.substring(start_idx);
      break;
    }
    
    int space_idx = -1;
    for (int i = end_idx; i >= start_idx; i--) {
      if (text.charAt(i) == ' ') {
        space_idx = i;
        break;
      }
    }
    
    if (space_idx != -1) {
      lines[line_cnt++] = text.substring(start_idx, space_idx);
      start_idx = space_idx + 1;
    } else {
      lines[line_cnt++] = text.substring(start_idx, end_idx);
      start_idx = end_idx;
    }
  }
  
  int total_height = line_cnt * (char_h + 4);
  int start_y = (SCREEN_H - total_height) / 2;
  if (align == "up") start_y = 10;
  if (align == "down") start_y = SCREEN_H - total_height - 10;
  if (start_y < 10) start_y = 10;
  
  for (int i = 0; i < line_cnt; i++) {
    int line_w = lines[i].length() * char_w;
    int x = 10; 
    if (align == "center" || align == "up" || align == "down") {
      x = (SCREEN_W - line_w) / 2;
    } else if (align == "right") {
      x = SCREEN_W - line_w - 10;
    }
    tft.setCursor(x, start_y + i * (char_h + 4));
    tft.println(lines[i]);
  }
}

// --- Menu UI ---
void render_menu() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(20, 20);
    tft.println("MAIN MENU");
    tft.drawLine(20, 50, 300, 50, ST77XX_CYAN);
    
    tft.setTextSize(2);
    for(int i = 0; i < MENU_COUNT; i++) {
        int y = 80 + (i * 35);
        if (i == menu_index) {
            tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
            tft.fillRect(10, y-5, 300, 30, ST77XX_WHITE);
        } else {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        }
        tft.setCursor(20, y);
        tft.print(menu_items[i]);
    }
    
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(20, 220);
    tft.print("B1:Up  B2:Down  B3:Select");
}

void render_nametag() {
    if (show_image_mode) {
        if(SPIFFS.exists("/image.raw")) {
            File f = SPIFFS.open("/image.raw", "r");
            if(f) {
                uint8_t buffer[640]; 
                tft.setAddrWindow(0, 0, 320, 240);
                for(int y = 0; y < 240; y++) {
                    f.read(buffer, 640);
                    tft.drawRGBBitmap(0, y, (uint16_t*)buffer, 320, 1);
                }
                f.close();
            }
        } else {
            tft.fillScreen(ST77XX_BLACK);
            tft.setTextSize(3);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(20, 100);
            tft.print("No Image Saved");
        }
    } else {
        if (current_text == "") {
            tft.fillScreen(ST77XX_BLACK);
            tft.setTextSize(3);
            tft.setTextColor(ST77XX_WHITE);
            tft.setCursor(20, 100);
            tft.print("No Text Saved");
        } else {
            drawWordWrappedText(current_text, text_size, text_r, text_g, text_b, text_align);
        }
    }
}

void render_tvbgone() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(20, 30);
    tft.println("TV-B-GONE");
    
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(20, 100);
    tft.println("B1: Fire (GPIO 2)");
    tft.setCursor(20, 140);
    tft.println("B2: Fire (GPIO 5)");
    
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(20, 200);
    tft.println("B4: Exit");
}

void render_totp() {
    tft.fillScreen(ST77XX_BLACK);
    
    if (time_offset == 0) {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(20, 100);
        tft.println("Time Not Synced!");
        tft.setCursor(20, 140);
        tft.setTextColor(ST77XX_WHITE);
        tft.println("Connect Python/Android");
        tft.setCursor(20, 170);
        tft.println("App to Sync Time.");
    } 
    else if (current_totp_slot == -1) {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(20, 40);
        tft.println("Select Account:");
        tft.setTextColor(ST77XX_CYAN);
        tft.setCursor(20, 90);
        tft.println("B1: Slots 1 & 2");
        tft.setCursor(20, 130);
        tft.println("B2: Slots 3 & 4");
        tft.setCursor(20, 170);
        tft.println("B3: Slots 5 & 6");
    }
    else if (totp_secrets[current_totp_slot] == "") {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(20, 100);
        tft.println("Slot " + String(current_totp_slot + 1) + " Empty");
        tft.setCursor(20, 140);
        tft.setTextColor(ST77XX_WHITE);
        tft.println("Add via the App.");
    } 
    else {
        uint8_t hmacKey[40];
        int keyLen = base32_decode(totp_secrets[current_totp_slot].c_str(), hmacKey, sizeof(hmacKey));
        
        TOTP totp(hmacKey, keyLen);
        unsigned long current_time = (millis() / 1000) + time_offset;
        char* new_code = totp.getCode(current_time);
        last_totp_code = String(new_code);
        
        tft.setTextSize(3);
        tft.setTextColor(ST77XX_GREEN);
        String t_name = totp_names[current_totp_slot];
        int name_x = (SCREEN_W - (t_name.length() * 18)) / 2;
        if(name_x < 0) name_x = 0;
        tft.setCursor(name_x, 50);
        tft.println(t_name);
        
        tft.setTextSize(5);
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor((SCREEN_W - (6 * 30)) / 2, 120);
        tft.println(last_totp_code);
    }
    
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(20, 210);
    tft.println("B4: Exit");
}

void render_games() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(20, 30);
    tft.println("GAMES");
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(20, 100);
    tft.println("Games coming soon!");
    tft.setCursor(20, 200);
    tft.setTextColor(ST77XX_YELLOW);
    tft.println("B4: Exit");
}

// --- Persistence ---
void load_state() {
    prefs.begin("badge", false);
    current_state = (AppState)prefs.getInt("mode", MODE_MENU);
    current_pattern = prefs.getString("pattern", "Rainbow");
    led_brightness = prefs.getInt("bright", 80);
    led_r = prefs.getInt("led_r", 0);
    led_g = prefs.getInt("led_g", 255);
    led_b = prefs.getInt("led_b", 0);
    
    current_text = prefs.getString("text", "");
    text_size = prefs.getInt("t_sz", 3);
    text_r = prefs.getInt("t_r", 255);
    text_g = prefs.getInt("t_g", 255);
    text_b = prefs.getInt("t_b", 255);
    text_align = prefs.getString("t_al", "center");
    
    show_image_mode = prefs.getBool("img_mode", false);
    
    for(int i=0; i<6; i++) {
        totp_names[i] = prefs.getString(("t_n_" + String(i)).c_str(), "");
        totp_secrets[i] = prefs.getString(("t_s_" + String(i)).c_str(), "");
    }
    
    // Find pattern index
    for(int i=0; i<PATTERN_COUNT; i++) {
        if(current_pattern == String(patterns[i])) {
            pattern_index = i;
            break;
        }
    }
    
    prefs.end();
}

void save_state() {
    prefs.begin("badge", false);
    prefs.putInt("mode", current_state);
    prefs.putString("pattern", current_pattern);
    prefs.putInt("bright", led_brightness);
    prefs.putInt("led_r", led_r);
    prefs.putInt("led_g", led_g);
    prefs.putInt("led_b", led_b);
    
    prefs.putString("text", current_text);
    prefs.putInt("t_sz", text_size);
    prefs.putInt("t_r", text_r);
    prefs.putInt("t_g", text_g);
    prefs.putInt("t_b", text_b);
    prefs.putString("t_al", text_align);
    
    prefs.putBool("img_mode", show_image_mode);
    
    for(int i=0; i<6; i++) {
        prefs.putString(("t_n_" + String(i)).c_str(), totp_names[i]);
        prefs.putString(("t_s_" + String(i)).c_str(), totp_secrets[i]);
    }
    
    prefs.end();
}

void switch_state(AppState new_state) {
    current_state = new_state;
    save_state();
    if(current_state == MODE_MENU) {
        if(current_pattern == "Solid") {
            strip.setBrightness(led_brightness);
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
            strip.show();
        }
        render_menu();
    }
    else if(current_state == MODE_NAMETAG) render_nametag();
    else if(current_state == MODE_TVBGONE) render_tvbgone();
    else if(current_state == MODE_TOTP) render_totp();
    else if(current_state == MODE_GAMES) render_games();
}

// --- API ---
void processCommand(String data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
        Serial.println("JSON Parse Error: " + data);
        return;
    }
    
    String cmd = doc["cmd"];
    if (cmd == "display_text") {
        current_text = doc["args"]["msg"].as<String>();
        text_size = doc["args"]["size"].as<int>();
        text_r = doc["args"]["r"].as<int>();
        text_g = doc["args"]["g"].as<int>();
        text_b = doc["args"]["b"].as<int>();
        text_align = doc["args"]["align"].as<String>();
        
        save_state();
        if(current_state == MODE_NAMETAG && !show_image_mode) {
            render_nametag();
        }
        Serial.println("{\"status\": \"ok\", \"cmd\": \"display_text\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"display_text\"}");
    }
    else if (cmd == "led_pattern") {
        current_pattern = doc["args"]["type"].as<String>();
        for(int i=0; i<PATTERN_COUNT; i++) {
            if(current_pattern == String(patterns[i])) pattern_index = i;
        }
        
        save_state();
        if(current_pattern == "Solid") {
            strip.setBrightness(led_brightness);
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
            strip.show();
        }
        Serial.println("{\"status\": \"ok\", \"cmd\": \"led_pattern\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"led_pattern\"}");
    }
    else if (cmd == "led_color") {
        led_r = doc["args"]["r"].as<int>();
        led_g = doc["args"]["g"].as<int>();
        led_b = doc["args"]["b"].as<int>();
        save_state();
        if(current_pattern == "Solid") {
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
            strip.show();
        }
        Serial.println("{\"status\": \"ok\", \"cmd\": \"led_color\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"led_color\"}");
    }
    else if (cmd == "image_start") {
        imgFile = SPIFFS.open("/image.raw", "w");
        Serial.println("{\"status\": \"ok\", \"cmd\": \"image_start\"}");
    }
    else if (cmd == "image_raw") {
        int y = doc["args"]["y"].as<int>();
        Serial.printf("{\"event\": \"send_raw\", \"y\": %d}\n", y);
        
        uint8_t buffer[640];
        unsigned long t0 = millis();
        int bytesRead = 0;
        while(bytesRead < 640 && millis() - t0 < 1000) {
            if(Serial.available()) {
                buffer[bytesRead++] = Serial.read();
            }
        }
        
        if (bytesRead == 640) {
            if(imgFile) imgFile.write(buffer, 640);
            
            if(current_state == MODE_NAMETAG && show_image_mode) {
                tft.drawRGBBitmap(0, y, (uint16_t*)buffer, 320, 1);
            }
            Serial.printf("{\"event\": \"row_ok\", \"y\": %d}\n", y);
            
            if(y == 239) {
                if(imgFile) imgFile.close();
            }
        } else {
            Serial.printf("{\"event\": \"row_err\", \"y\": %d}\n", y);
        }
    }
    else if (cmd == "sync_time") {
        unsigned long t = doc["args"]["t"].as<unsigned long>();
        time_offset = t - (millis() / 1000);
        Serial.println("{\"status\": \"ok\", \"cmd\": \"sync_time\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"sync_time\"}");
        if (current_state == MODE_TOTP) render_totp();
    }
    else if (cmd == "add_totp") {
        int slot = doc["args"]["slot"].as<int>();
        if(slot >= 0 && slot < 6) {
            totp_names[slot] = doc["args"]["name"].as<String>();
            totp_secrets[slot] = doc["args"]["secret"].as<String>();
            save_state();
        }
        Serial.println("{\"status\": \"ok\", \"cmd\": \"add_totp\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"add_totp\"}");
        if (current_state == MODE_TOTP) render_totp();
    }
    else if (cmd == "delete_totp") {
        int slot = doc["args"]["slot"].as<int>();
        if(slot >= 0 && slot < 6) {
            totp_names[slot] = "";
            totp_secrets[slot] = "";
            save_state();
        }
        Serial.println("{\"status\": \"ok\", \"cmd\": \"delete_totp\"}");
        sendBLE("{\"status\": \"ok\", \"cmd\": \"delete_totp\"}");
        if (current_state == MODE_TOTP) render_totp();
    }
    else if (cmd == "get_totps") {
        String json = "{\"event\": \"totp_list\", \"data\": [";
        for(int i=0; i<6; i++) {
            json += "\"" + totp_names[i] + "\"";
            if(i < 5) json += ",";
        }
        json += "]}";
        Serial.println(json);
        sendBLE(json);
    }
}

void setup() {
    Serial.setRxBufferSize(2048);
    Serial.begin(115200);
    delay(500);

    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
    }

    load_state();

    pinMode(BTN_B3, INPUT_PULLUP);
    pinMode(BTN_B4, INPUT_PULLUP);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(2, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(5, HIGH);

    Wire.begin(I2C_SDA, I2C_SCL, 100000);
    Wire.beginTransmission(PCF8574_ADDR);
    if (Wire.endTransmission() == 0) {
        pcf_ready = true;
        Wire.beginTransmission(PCF8574_ADDR);
        Wire.write(0xFF);
        Wire.endTransmission();
    }

    strip.begin();
    strip.setBrightness(led_brightness);
    if(current_pattern == "Solid") {
        for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
    }
    strip.show();

    tft.init(240, 320);
    tft.setRotation(1);
    tft.invertDisplay(true);

    setupBLE();

    switch_state(current_state);
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        if (data.length() > 0) processCommand(data);
    }

    updateLEDs();

    // Check if TOTP code changed to re-render screen
    if (current_state == MODE_TOTP && time_offset > 0 && current_totp_slot != -1 && totp_secrets[current_totp_slot] != "") {
        uint8_t hmacKey[40];
        int keyLen = base32_decode(totp_secrets[current_totp_slot].c_str(), hmacKey, sizeof(hmacKey));
        TOTP totp(hmacKey, keyLen);
        unsigned long current_time = (millis() / 1000) + time_offset;
        char* new_code = totp.getCode(current_time);
        if (String(new_code) != last_totp_code) {
            render_totp();
        }
    }

    bool curr_b1 = false;
    bool curr_b2 = false;
    bool curr_b3 = digitalRead(BTN_B3) == LOW;
    bool curr_b4 = digitalRead(BTN_B4) == LOW;

    if (pcf_ready) {
        Wire.requestFrom((uint16_t)PCF8574_ADDR, (uint8_t)1);
        if (Wire.available()) {
            uint8_t pcf_port = Wire.read();
            curr_b1 = (pcf_port & (1 << 1)) == 0;
            curr_b2 = (pcf_port & (1 << 2)) == 0;
            curr_b3 = curr_b3 || ((pcf_port & (1 << 3)) == 0);
        }
    }

    if (curr_b1 && !state_b1) {
        if (current_state == MODE_MENU) {
            menu_index--;
            if(menu_index < 0) menu_index = MENU_COUNT - 1;
            render_menu();
        } else if (current_state == MODE_TOTP) {
            if (current_totp_slot == 0) current_totp_slot = 1;
            else current_totp_slot = 0;
            render_totp();
        } else if (current_state == MODE_NAMETAG) {
            show_image_mode = true;
            save_state();
            render_nametag();
        } else if (current_state == MODE_TVBGONE) {
            strip.setBrightness(255);
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255,0,0));
            strip.show();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(20, 100);
            tft.setTextSize(3);
            tft.println("FIRING GPIO 2!");
            fire_tvbgone(2);
            render_tvbgone();
        }
    }
    if (curr_b2 && !state_b2) {
        if (current_state == MODE_MENU) {
            menu_index++;
            if(menu_index >= MENU_COUNT) menu_index = 0;
            render_menu();
        } else if (current_state == MODE_TOTP) {
            if (current_totp_slot == 2) current_totp_slot = 3;
            else current_totp_slot = 2;
            render_totp();
        } else if (current_state == MODE_NAMETAG) {
            show_image_mode = false;
            save_state();
            render_nametag();
        } else if (current_state == MODE_TVBGONE) {
            strip.setBrightness(255);
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(0,0,255));
            strip.show();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(20, 100);
            tft.setTextSize(3);
            tft.println("FIRING GPIO 5!");
            fire_tvbgone(5);
            render_tvbgone();
        }
    }
    if (curr_b3 && !state_b3) {
        if (current_state == MODE_MENU) {
            if (menu_index == 0) switch_state(MODE_NAMETAG);
            else if (menu_index == 1) switch_state(MODE_TVBGONE);
            else if (menu_index == 2) {
                current_totp_slot = -1; // reset slot selection
                switch_state(MODE_TOTP);
            }
            else if (menu_index == 3) switch_state(MODE_GAMES);
        } else if (current_state == MODE_TOTP) {
            if (current_totp_slot == 4) current_totp_slot = 5;
            else current_totp_slot = 4;
            render_totp();
        } else if (current_state == MODE_NAMETAG) {
            pattern_index = (pattern_index + 1) % PATTERN_COUNT;
            current_pattern = patterns[pattern_index];
            if (current_pattern == "Solid") {
                strip.setBrightness(led_brightness);
                for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
                strip.show();
            }
            save_state();
        }
    }
    if (curr_b4 && !state_b4) {
        if (current_state != MODE_MENU) {
            switch_state(MODE_MENU);
        }
    }

    state_b1 = curr_b1;
    state_b2 = curr_b2;
    state_b3 = curr_b3;
    state_b4 = curr_b4;

    delay(10);
}
