#pragma once
#include <Arduino.h>

#undef F_CPU
#define F_CPU 8000000
#define freq_to_timerval(x) (F_CPU / 8 / x - 1)

#include "WORLD_IR_CODES.h"

extern Adafruit_ST7789 tft;

void delay_ten_us(uint16_t us) {
  delayMicroseconds(us * 10);
}

uint8_t bitsleft_r = 0;
uint8_t bits_r = 0;
const uint8_t* code_ptr;

uint8_t read_bits(uint8_t count) {
  uint8_t i;
  uint8_t tmp = 0;
  for (i = 0; i < count; i++) {
    if (bitsleft_r == 0) {
      bits_r = pgm_read_byte(code_ptr++);
      bitsleft_r = 8;
    }
    bitsleft_r--;
    tmp |= (((bits_r >> bitsleft_r) & 1) << (count - 1 - i));
  }
  return tmp;
}

int current_tx_pin = 2;

void xmitCodeElement(uint16_t ontime, uint16_t offtime, uint8_t PWM_code) {
  if (PWM_code) {
    ledcWrite(0, 127);
  } else {
    // Just turn IR on completely
    digitalWrite(current_tx_pin, HIGH);
  }
  
  delay_ten_us(ontime);
  
  ledcWrite(0, 0);
  digitalWrite(current_tx_pin, LOW);
  
  delay_ten_us(offtime);
}

void fire_tvbgone(int tx_pin) {
  current_tx_pin = tx_pin;
  uint8_t num_codes = sizeof(NApowerCodes) / sizeof(NApowerCodes[0]);

  ledcSetup(0, 38000, 8);
  ledcAttachPin(tx_pin, 0);

  for (uint8_t i = 0; i < num_codes; i++) {
    // Check if B3 or B4 is pressed to stop
    if (digitalRead(10) == LOW || digitalRead(9) == LOW) {
       delay(500); // debounce
       break;
    }

    tft.fillRect(10, 120, 300, 40, ST77XX_BLACK);
    tft.setCursor(20, 125);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.printf("Code: %d / %d", i+1, num_codes);

    const struct IrCode* data_ptr = (const struct IrCode*)pgm_read_word(&NApowerCodes[i]);

    uint8_t freq_val = pgm_read_byte(&data_ptr->timer_val);
    uint8_t numpairs = pgm_read_byte(&data_ptr->numpairs);
    uint8_t bitcompression = pgm_read_byte(&data_ptr->bitcompression);
    const uint16_t* time_ptr = (const uint16_t*)pgm_read_word(&data_ptr->times);
    code_ptr = (const uint8_t*)pgm_read_word(&data_ptr->codes);

    uint32_t freq_hz = 1000000 / (freq_val + 1);
    ledcSetup(0, freq_hz, 8);
    ledcWrite(0, 0);

    for (uint8_t k = 0; k < numpairs; k++) {
      uint16_t ti = read_bits(bitcompression) * 2;
      uint16_t ontime = pgm_read_word(time_ptr + ti);
      uint16_t offtime = pgm_read_word(time_ptr + ti + 1);
      xmitCodeElement(ontime, offtime, (freq_val != 0));
    }

    bitsleft_r = 0;
    delay(205); // 205 ms delay between codes
  }
  
  ledcDetachPin(tx_pin);
}
