# 🕵️ DEF CON 34 Cryptocurrency Village Badge - Hardware & OS Analysis

This document serves as the single source of truth for the ESP32-C3 hardware pinouts and the features built into **Super Badge OS 2.0**.

---

## 🛠️ Hardware Specifications & Pinout

After extensive testing and reverse engineering, here is the definitive hardware map for the badge:

**Microcontroller**: ESP32-C3 (160MHz, 320KB RAM, 4MB Flash, No PSRAM)

### 🖥️ Display (ST7789 TFT Color LCD)
- **Resolution**: 320 x 240
- **SPI Pins**:
  - **MOSI**: GPIO 7
  - **SCLK**: GPIO 6
  - **CS**: GPIO 1
  - **DC**: GPIO 0
  - **RST**: -1 (Software reset / tied to EN)

### 🎛️ Physical Buttons (Digital Inputs)
The 4 main buttons pull LOW when pressed.
- **B1**: GPIO 0
- **B2**: GPIO 1
- **B3**: GPIO 10
- **B4**: GPIO 9

### 💡 NeoPixel LEDs
- **Data Pin**: GPIO 8
- **Count**: 16 LEDs

### 🔌 I2C Bus & Expansion
- **SDA**: GPIO 20
- **SCL**: GPIO 21
- **PCF8574 I/O Expander**: Address `0x20`
- **Dynamic NFC Tag**: Wired internally to I2C/Circuit (Present on back of badge)
- **SAO Port**: Standard I2C pinout for expansion

### 📡 Infrared (IR) Transmitter
- **IR TX Pins**: GPIO 2 and GPIO 5 (Tested with TV-B-Gone)

---

## 💾 Super Badge OS 2.0 (Custom Firmware)

We successfully overwrote the factory CLI/Zork text adventure with a fully graphical, menu-driven OS designed for daily use and security.

### Core Features

1. **🔒 Security & Auto-Lock**
   - **4-Digit PIN**: The badge boots into a "Welcome" lock screen if a PIN is set. The screen remains intact without flickering during entry.
   - **Idle Timeout**: Automatically locks the badge after 5 minutes of inactivity (no button presses or app commands).
   - **Manual Lock**: Dedicated "Lock Badge" button in the OS menu.

2. **🕹️ Arcade Games**
   Due to memory constraints (no PSRAM), games are statically allocated to run smoothly:
   - **Flappy Badge**: Gravity-based side-scroller.
   - **Snake**: Classic grid-based snake.
   - **Pong**: Single-player paddle vs AI wall.
   - **Simon Says**: Memory game utilizing the NeoPixel ring.

3. **🔐 TOTP Authenticator**
   - Stores up to 6 Time-based One-Time Password (TOTP) base32 secrets in non-volatile storage (NVS).
   - Generates live 6-digit 2FA codes for accounts like Microsoft Authenticator, Google, etc.
   - Real-time progress bar synced to the 30-second epoch.

4. **📛 Digital Nametag**
   - Displays custom name, title, and scrolling marquee messages loaded via the desktop app.

5. **📺 TV-B-Gone**
   - Ported 137 NA/EU power-off IR codes using proper 32-bit memory pointers to prevent ESP32 memory panics.
   - Transmits via GPIO 2 or 5.
   - Killswitch: B4 immediately aborts transmission.

---

## 💻 Python Desktop Companion App (`Super_Badge_OS_2.0.py`)

A fully-featured desktop control center built with `tkinter` and `pyserial`.

- **Live Serial Communication**: Communicates with the badge over USB-Serial (115200 baud) using JSON payloads.
- **Nametag Sync**: Instantly pushes names and marquee messages to the badge display.
- **Lighting Controls**: UI sliders to change the brightness and color of the NeoPixel ring.
- **Authenticator Setup**: Tab to securely inject base32 TOTP secrets into the badge's NVS memory slots.
- **Security Tab**: Allows setting, updating, and removing the badge PIN, plus an instant "Lock Badge Now" override button.

---

## 📂 Repository Organization

The project is cleanly separated into two distinct eras:

* `1.0_Original_Badge/`: The factory DEF CON 34 firmware (Zork, Text Adventure, Frotz, OpenLASIR) and original python scripts.
* `2.0_Super_Badge/`: The active, modern GUI operating system built using PlatformIO, the Arduino framework, and Adafruit GFX.
* `archive/`: Old scripts, test dumps, and standalone `.ino` experiments.
* `docs/`: Implementation plans, task lists, and this hardware manual.
