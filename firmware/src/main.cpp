#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ArduinoJson.h>

#define LED_PIN     8
#define NUM_LEDS    16
#define TFT_CS      1
#define TFT_DC      0
#define TFT_RST     21
#define TFT_MOSI    7
#define TFT_SCLK    6
#define SCREEN_W    320
#define SCREEN_H    240

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int led_r = 0, led_g = 255, led_b = 0; // Default Green
int led_brightness = 80;
String current_pattern = "Solid";
unsigned long last_pattern_update = 0;
uint16_t pattern_step = 0;

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
  if (current_pattern == "Solid") return;
  
  if (current_pattern == "Rainbow") {
    if (millis() - last_pattern_update > 20) {
      last_pattern_update = millis();
      for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, Wheel(((i * 256 / NUM_LEDS) + pattern_step) & 255));
      strip.show();
      pattern_step++;
      if (pattern_step >= 256) pattern_step = 0;
    }
  }
  else if (current_pattern == "Breathe") {
    if (millis() - last_pattern_update > 15) {
      last_pattern_update = millis();
      float val = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
      strip.setBrightness(val);
      for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
      strip.show();
    }
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
  else if (current_pattern == "Cylon") {
    if (millis() - last_pattern_update > 50) {
      last_pattern_update = millis();
      strip.clear();
      int pos = pattern_step % (NUM_LEDS * 2);
      if (pos >= NUM_LEDS) pos = (NUM_LEDS * 2) - 1 - pos;
      strip.setPixelColor(pos, strip.Color(led_r, led_g, led_b));
      strip.show();
      pattern_step++;
    }
  }
  else if (current_pattern == "Twinkle") {
    if (millis() - last_pattern_update > 100) {
      last_pattern_update = millis();
      strip.clear();
      for(int i=0; i<NUM_LEDS; i++) if(random(10) > 8) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
      strip.show();
    }
  }
  else if (current_pattern == "Sparkle") {
    if (millis() - last_pattern_update > 30) {
      last_pattern_update = millis();
      strip.clear();
      int pixel = random(NUM_LEDS);
      strip.setPixelColor(pixel, strip.Color(255, 255, 255));
      strip.show();
    }
  }
}

void drawBootScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, SCREEN_W, SCREEN_H, ST77XX_GREEN);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 15);
  tft.println("SUPER BADGE OS");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 70);
  tft.println("Status : ONLINE");
  tft.setCursor(20, 95);
  tft.println("B4 = Laser Tag");
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(20, 200);
  tft.println("DEF CON 34 - Crypto Village");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(20, 220);
  tft.println("(c) ShreyJain");
}

void typeText(String text, int size, int r, int g, int b, String align) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(size);
  tft.setTextColor(tft.color565(r, g, b));
  
  int charW = 6 * size;
  int charH = 8 * size;
  int maxCharsPerLine = 320 / charW;
  
  String words[50];
  int wordCount = 0;
  int startIdx = 0;
  for (int i = 0; i <= text.length(); i++) {
    if (i == text.length() || text.charAt(i) == ' ' || text.charAt(i) == '\n') {
      words[wordCount++] = text.substring(startIdx, i);
      startIdx = i + 1;
      if (wordCount >= 50) break;
    }
  }

  String lines[15];
  int lineCount = 0;
  lines[0] = "";
  for (int i = 0; i < wordCount; i++) {
    String testLine = lines[lineCount];
    if (testLine.length() > 0) testLine += " ";
    testLine += words[i];
    
    if (testLine.length() > maxCharsPerLine && lines[lineCount].length() > 0) {
      lineCount++;
      if (lineCount >= 15) break;
      lines[lineCount] = words[i];
    } else {
      lines[lineCount] = testLine;
    }
  }
  lineCount++;

  int startY = (240 - (lineCount * charH)) / 2;
  if (startY < 0) startY = 10;
  
  for (int i = 0; i < lineCount; i++) {
    String line = lines[i];
    int startX = 10; 
    if (align == "center") startX = (320 - (line.length() * charW)) / 2;
    else if (align == "right") startX = 320 - (line.length() * charW) - 10;
    
    if (startX < 0) startX = 0;
    
    tft.setCursor(startX, startY + (i * charH) + (i * 4)); 
    
    for (int c = 0; c < line.length(); c++) {
      tft.print(line.charAt(c));
      delay(35);
    }
  }
}

void processCommand(String cmdLine) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, cmdLine);
  if (error) return;

  String cmd = doc["cmd"].as<String>();

  if (cmd == "image_raw") {
    int y = doc["args"]["y"].as<int>();
    Serial.printf("{\"status\": \"send_raw\", \"y\": %d}\n", y);
    uint8_t rowData[640];
    int read = Serial.readBytes(rowData, 640);
    if (read == 640) {
      tft.drawRGBBitmap(0, y, (uint16_t*)rowData, 320, 1);
      Serial.printf("{\"status\": \"row_ok\", \"y\": %d}\n", y);
    }
  } 
  else if (cmd == "led_color") {
    led_r = doc["args"]["r"].as<int>();
    led_g = doc["args"]["g"].as<int>();
    led_b = doc["args"]["b"].as<int>();
    if (current_pattern == "Solid") {
      strip.setBrightness(led_brightness);
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
      strip.show();
    }
  }
  else if (cmd == "led_pattern") {
    current_pattern = doc["args"]["type"].as<String>();
    strip.setBrightness(led_brightness);
    if (current_pattern == "Solid") {
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
      strip.show();
    }
  }
  else if (cmd == "display_text") {
    String msg = doc["args"]["msg"].as<String>();
    int size = doc["args"]["size"].as<int>();
    if (size <= 0) size = 2;
    int r = doc["args"]["r"].as<int>();
    int g = doc["args"]["g"].as<int>();
    int b = doc["args"]["b"].as<int>();
    String align = doc["args"]["align"].as<String>();
    
    typeText(msg, size, r, g, b, align);
  }
  else if (cmd == "home_screen") {
    drawBootScreen();
  }
}

void setup() {
  Serial.setRxBufferSize(2048);
  Serial.begin(115200);
  delay(500);

  pinMode(9, INPUT_PULLUP);

  strip.begin();
  strip.setBrightness(led_brightness);
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
  strip.show();

  tft.init(240, 320);
  tft.setRotation(1);
  tft.invertDisplay(true);
  drawBootScreen();
}

unsigned long last_b4 = 0;

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    if (data.length() > 0) processCommand(data);
  }

  updateLEDs();

  if (digitalRead(9) == LOW && millis() - last_b4 > 500) {
    last_b4 = millis();
    strip.setBrightness(255);
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(200);
    strip.setBrightness(led_brightness);
    if (current_pattern == "Solid") {
      for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
      strip.show();
    }
  }
  delay(10);
}
