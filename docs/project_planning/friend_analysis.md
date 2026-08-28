# Analysis of Friend's Custom OS

## Overview
Your friend entirely stripped the badge of its original factory OS (which had the Zork game, Laser Tag, etc.). Instead, he used the ESP-IDF framework to build a **custom multi-stage Capture The Flag (CTF) puzzle vault**. 

The hardware features he implemented are strictly limited to the things needed to run his puzzle. **He completely ignored the IR transmitter, TV-B-Gone, Laser Tag, and NeoPixels**, leaving them deactivated in his code.

Here is a detailed breakdown of exactly what he implemented:

### 1. The "Trapdoor Vault" Challenge (`trapdoor.c`)
This is the core of his custom OS. He programmed a 4-stage puzzle vault that requires the player to bypass multiple security layers in order:
*   **Layer 1 (WEB):** The player must connect to a rogue WiFi network broadcasted by the badge and solve a prompt on the captive portal web page.
*   **Layer 2 (BUTTONS):** A defusal sequence using the physical buttons (B1-B4). It uses timers and state tracking, meaning you have to press the buttons in a specific combination within a time limit or the "fuse" expires.
*   **Layer 3 (BLE):** A "Ghost" Bluetooth Low Energy beacon that requires the player to scan for the badge and connect via Bluetooth.
*   **Layer 4 (DUAL AUTH):** A "two-operator sync" requiring two distinct phone/device identities to connect to the badge simultaneously.

### 2. Rogue WiFi Access Point & Web Server (`portal.html`)
He configured the ESP32-C3's WiFi chip to act in AP (Access Point) mode, meaning the badge broadcasts its own WiFi network.
*   When connected, a local DNS server forces all traffic to a custom `portal.html` file.
*   The webpage is styled as a hacker terminal (`radial-gradient`, `ui-monospace` font, dark green aesthetic) and feeds into the "Layer 1 (WEB)" puzzle.

### 3. Ghost BLE Beacon (`ghost_ble.c`)
He implemented a low-level Bluetooth Low Energy (BLE) driver. 
*   It broadcasts custom GATT characteristics.
*   This is designed to force players to use a BLE scanner app (like nRF Connect) to find the badge and interact with its exposed services to pass Layer 3.

### 4. Custom Display Engine (`avatar_display.c`)
Rather than using a high-level graphics library like Adafruit_GFX, he wrote a low-level SPI driver for the ST7789 screen.
*   He manages the memory directly, sending raw `RGB565` pixel data to the screen.
*   He used this to draw a hardcoded static image file (`meme_rgb565.bin`) onto the screen when the badge powers on.

---

## How Their Buttons Work (Hardware Breakthrough)
By analyzing their `main.c` file, I discovered a massive piece of the hardware puzzle that helped us fix our buttons earlier!

In our initial hardware scans, we assumed the `PCF8574` I2C chip was missing because we probed the I2C pins marked on the back of the badge (`SDA=3, SCL=4`). However, your friend's code reveals that those pins are only for the *external* Stemma QT connector! 

The *internal* PCF8574 chip (the one controlling the buttons) is completely hidden and wired to **GPIO 20 (SDA)** and **GPIO 21 (SCL)**! 

Here is the exact physical mapping of your 4 buttons from left to right, according to their code:

1. **B1 (Leftmost):** Wired to PCF8574 (Port mask `1 << 1`)
2. **B2 (Middle-Left):** Wired to PCF8574 (Port mask `1 << 2`)
3. **B3 (Middle-Right):** Wired directly to **GPIO 10**!
4. **B4 (Rightmost):** Wired directly to **GPIO 9**

This means the chip is NOT missing from your board. We were just knocking on the wrong door. This exact mapping is what allowed us to get your physical buttons fully functional on our custom Python App OS!
