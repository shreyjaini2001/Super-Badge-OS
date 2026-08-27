# Super Badge OS 2.0 - Hardware & Architecture Manual

This document details every minutest hardware connection, pinout, state machine logic, and protocol mapping discovered and implemented for the "DEFCON Super Badge" platform. 

If we ever need to fall back or build a 3.0, this document serves as the absolute source of truth.

## 1. Microcontroller
- **Chip**: ESP32-C3-DevKitM-1 (RISC-V, 160MHz)
- **RAM**: 320KB
- **Flash**: 4MB (Configured with SPIFFS for image storage, NVS for Preferences)
- **Wireless**: Wi-Fi (Not actively used to save battery) & NimBLE-Arduino (Bluetooth Low Energy) for app communication.

## 2. Pinouts & Hardware Map

### TFT Display (ST7789)
The display is a 240x320 IPS LCD controlled via SPI.
- **MOSI**: GPIO 7
- **SCLK**: GPIO 6
- **CS**: GPIO 1
- **DC**: GPIO 0
- **RST**: -1 (Software Reset is used)
- **Backlight**: Hardwired to VCC (Always on)

### NeoPixel Ring (WS2812B)
16 Addressable RGB LEDs arranged in a ring on the front of the badge.
- **Data Pin**: GPIO 8
- **Count**: 16 LEDs

### I2C Bus & PCF8574 (Port Expander)
The badge relies on a PCF8574 I2C port expander to read some of the tactile buttons, freeing up GPIO on the ESP32-C3.
- **SDA**: GPIO 20
- **SCL**: GPIO 21
- **PCF8574 Address**: `0x20`

### Physical Buttons (B1, B2, B3, B4)
The front of the badge has 4 tactile buttons. Their routing is split between direct GPIO and the I2C expander.
- **B1 (Top Left)**: PCF8574 Bit 1 (Mask `(1 << 1)`) - Active Low.
- **B2 (Bottom Left)**: PCF8574 Bit 2 (Mask `(1 << 2)`) - Active Low.
- **B3 (Top Right)**: Tied to BOTH PCF8574 Bit 3 AND GPIO 10 (Direct ESP32 pin). Active Low.
- **B4 (Bottom Right)**: GPIO 9 (Direct ESP32 pin). Active Low.

### Infrared (IR) Blaster (TV-B-Gone)
The top edge of the badge has IR LEDs and an IR Receiver.
- **IR Transmitter (TX)**: Tied to GPIO 2 (and possibly GPIO 5).
- **IR Receiver (RX)**: Tied to GPIO 3.
*(Note: To fire the TV-B-Gone codes, we output 38kHz modulated PWM signals on GPIO 2 or 5).*

## 3. Software Architecture (Super Badge OS 2.0)

The entire OS operates as a **non-blocking State Machine** running in `loop()`. The badge never uses `delay()` blocking code, allowing it to seamlessly handle Bluetooth commands, USB serial commands, Button interrupts, and LED animations concurrently.

### The State Machine (`current_state`)
1. **MODE_MENU (Main Menu)**
   - B1: Scroll Up
   - B2: Scroll Down
   - B3: Select App
2. **MODE_NAMETAG (Display & Customization)**
   - Displays either a 240x320 Image from SPIFFS or Word-Wrapped Text.
   - Background updating: App commands update the hidden state silently.
   - B1: Toggle View to Image Mode
   - B2: Toggle View to Text Mode
   - B3: Cycle LED Patterns (Solid, Rainbow, Breathe, Theater Chase, Mixed Cylon, Mixed Twinkle, Rainbow Sparkle)
   - B4: Exit to Menu
3. **MODE_TVBGONE (IR Blaster)**
   - Shoots North American TV power-off codes.
   - B1: Fire codes on GPIO 2 (Red LEDs flash)
   - B2: Fire codes on GPIO 5 (Blue LEDs flash)
   - B4: Exit to Menu
4. **MODE_TOTP (Authenticator)**
   - Decodes a Base32 Secret and generates rolling 6-digit 2FA codes.
   - NeoPixels act as a 30-second countdown timer (Green -> Yellow -> Red).
   - Time is synced via Bluetooth/USB using the `sync_time` JSON command.
5. **MODE_GAMES (Placeholder)**
   - Stubbed out for future Arcade games (Snake, Pong).

## 4. Communication Protocol (JSON)
The badge listens for newline-terminated JSON strings via **USB Serial (115200 baud)** and **Bluetooth LE UART**. Both interfaces are fed into the `processCommand()` parser.

- **Set Text**: `{"cmd": "display_text", "args": {"msg": "Hello", "size": 3, "r": 255, "g": 0, "b": 0, "align": "center"}}`
- **Set Pattern**: `{"cmd": "led_pattern", "args": {"type": "Rainbow"}}`
- **Sync Time**: `{"cmd": "sync_time", "args": {"t": 172983723}}`
- **Add TOTP**: `{"cmd": "add_totp", "args": {"name": "GitHub", "secret": "JBSWY3DPEHPK3PXP"}}`

## 5. Persistence (Non-Volatile Storage)
The `Preferences.h` library is used to save the badge's state to NVS flash memory. Whenever a setting is changed (Pattern, Text, Image View Mode, Color, TOTP Secret), it is written to memory. On boot, `load_state()` restores the badge exactly to how it was when it lost power.

## 6. Image Streaming (SPIFFS)
To achieve fast image uploading without exhausting the ESP32 RAM:
1. App sends `{"cmd": "image_start"}`. Badge opens `/image.raw` in `w` mode.
2. App sends `{"cmd": "image_raw", "args": {"y": 0}}`. Badge replies `{"event": "send_raw", "y": 0}`.
3. App writes 640 raw bytes (320 pixels * 2 bytes for RGB565).
4. Badge reads bytes, writes to file, and draws to screen.
5. On row 239, Badge safely closes the SPIFFS file handle.
