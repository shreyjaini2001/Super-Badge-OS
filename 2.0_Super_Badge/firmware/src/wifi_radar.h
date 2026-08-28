#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_ST7789 tft;
extern Adafruit_NeoPixel strip;

volatile int deauth_count = 0;
volatile unsigned long last_deauth_time = 0;

void wifi_promiscuous_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t *payload = pkt->payload;
    
    // Check if it is a management frame
    uint8_t frame_ctrl_0 = payload[0];
    uint8_t frame_type = (frame_ctrl_0 >> 2) & 0x03;
    uint8_t frame_subtype = (frame_ctrl_0 >> 4) & 0x0F;
    
    if (frame_type == 0) { // Management
        if (frame_subtype == 12 || frame_subtype == 10) { // Deauth (0x0C) or Disassoc (0x0A)
            deauth_count++;
            last_deauth_time = millis();
        }
    }
}

void init_wifi_radar() {
    deauth_count = 0;
    WiFi.mode(WIFI_MODE_NULL);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous_cb);
}

void stop_wifi_radar() {
    esp_wifi_set_promiscuous(false);
}

void render_wifi_radar(bool full_redraw = true) {
    if (full_redraw) {
        tft.fillScreen(ST77XX_BLACK);
        tft.drawRect(5, 5, 310, 230, ST77XX_RED);
        tft.setCursor(20, 20);
        tft.setTextSize(3);
        tft.setTextColor(ST77XX_WHITE);
        tft.println("Wi-Fi Attack");
        tft.setCursor(20, 50);
        tft.println("Detector");
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_GREEN);
        tft.setCursor(20, 90);
        tft.println("Listening for");
        tft.setCursor(20, 110);
        tft.println("Deauth Packets...");
        tft.setCursor(20, 210);
        tft.setTextColor(ST77XX_YELLOW);
        tft.println("[B4] Exit");
    }
    
    // Draw count
    tft.fillRect(20, 150, 280, 40, ST77XX_BLACK);
    tft.setCursor(20, 160);
    tft.setTextSize(3);
    if (deauth_count > 0) {
        tft.setTextColor(ST77XX_RED);
        tft.printf("ATTACKS: %d", deauth_count);
    } else {
        tft.setTextColor(ST77XX_WHITE);
        tft.printf("ATTACKS: 0");
    }
    
    // Blink LEDs red if under attack recently
    if (deauth_count > 0 && (millis() - last_deauth_time) < 2000) {
        if ((millis() / 200) % 2 == 0) {
            strip.fill(strip.Color(255, 0, 0));
        } else {
            strip.fill(strip.Color(0, 0, 0));
        }
    } else {
        strip.fill(strip.Color(0, 0, 10)); // faint blue listening
    }
    strip.show();
}
