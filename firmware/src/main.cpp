#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>

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

int extractInt(String json, String key) {
  int start = json.indexOf("\"" + key + "\":");
  if (start < 0) return 0;
  start += key.length() + 3;
  int end = start;
  while (end < json.length() && isDigit(json.charAt(end))) end++;
  return json.substring(start, end).toInt();
}

String extractString(String json, String key) {
  int start = json.indexOf("\"" + key + "\":\"");
  if (start < 0) return "";
  start += key.length() + 4;
  int end = json.indexOf("\"", start);
  if (end < 0) return "";
  return json.substring(start, end);
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

void processCommand(String cmdLine) {
  if (cmdLine.indexOf("\"cmd\": \"image_raw\"") > 0) {
    int y = extractInt(cmdLine, "y");
    Serial.printf("{\"status\": \"send_raw\", \"y\": %d}\n", y);
    uint8_t rowData[640];
    int read = Serial.readBytes(rowData, 640);
    if (read == 640) {
      tft.drawRGBBitmap(0, y, (uint16_t*)rowData, 320, 1);
      Serial.printf("{\"status\": \"row_ok\", \"y\": %d}\n", y);
    }
  } 
  else if (cmdLine.indexOf("\"cmd\": \"led_color\"") > 0) {
    led_r = extractInt(cmdLine, "r");
    led_g = extractInt(cmdLine, "g");
    led_b = extractInt(cmdLine, "b");
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
    }
    strip.show();
  }
  else if (cmdLine.indexOf("\"cmd\": \"display_text\"") > 0) {
    String msg = extractString(cmdLine, "msg");
    int size = extractInt(cmdLine, "size");
    if (size <= 0) size = 2;
    int r = extractInt(cmdLine, "r");
    int g = extractInt(cmdLine, "g");
    int b = extractInt(cmdLine, "b");
    
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 50);
    tft.setTextSize(size);
    tft.setTextColor(tft.color565(r, g, b));
    tft.println(msg);
  }
  else if (cmdLine.indexOf("\"cmd\": \"home_screen\"") > 0) {
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

  if (digitalRead(9) == LOW && millis() - last_b4 > 500) {
    last_b4 = millis();
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(200);
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(led_r, led_g, led_b));
    strip.show();
  }
  delay(10);
}
