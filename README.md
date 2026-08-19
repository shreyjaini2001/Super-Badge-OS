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
