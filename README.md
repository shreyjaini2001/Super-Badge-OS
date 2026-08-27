# Super Badge OS 

A complete reverse engineering, custom firmware, and desktop control application for the **DEF CON 34 Cryptocurrency Village Badge**.

## Repository Structure

- `/firmware` - Custom PlatformIO C++ firmware for the ESP32-C3 badge. Features a raw binary high-speed image upload protocol, ST7789 display drivers, and NeoPixel control.
- `/app` - Python Desktop UI (Tkinter) that communicates with the badge over USB Serial to upload custom text, adjust LED colors, and stream live images to the screen.
- `/docs` - Complete markdown documentation of the reverse engineering process, including the original badge architecture, hidden CTF wallet, and firmware rewrite plans.
- `/dumps` - Original factory firmware and SPIFFS dumps extracted from the badge using `esptool.py`.

## Features Restored / Upgraded
- ✅ **Display Drivers:** Fully configured ST7789 for the 240x320 panel.
- ✅ **NeoPixel LEDs:** 16 addressable LEDs with multiple animation patterns.
- ✅ **High-Speed Image Upload:** Direct RAW binary transfer protocol that bypasses native USB CDC buffer limits.
- ✅ **Non-Volatile Storage:** Badge remembers its last screen state, text, and LED pattern after reboot using `LittleFS` and `Preferences`.

*Work in Progress: Restoring physical button inputs, TV-B-Gone IR blaster, and Laser Tag.*

## Super Badge OS 2.0
The `2.0_Super_Badge` folder contains a complete rewrite of the badge operating system with a standalone state machine!
*   **Menu System**: Fully interactive main menu using B1 (Up), B2 (Down), B3 (Select), B4 (Exit).
*   **Nametag Mode**: Displays text and images. Remembers state across reboots using NVS Persistence.
*   **LED Engine**: Features 7 different patterns including Mixed Cylon, Mixed Twinkle, and Rainbow Sparkle.
*   **Bluetooth Low Energy (BLE)**: Supports updating the badge completely wirelessly using the Android "Serial Bluetooth Terminal" app!
*   **Fast Image Uploading**: Streams raw RGB565 files directly into SPIFFS storage.
