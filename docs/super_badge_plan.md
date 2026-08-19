# Implementation Plan: Super Badge WiFi Controller

Based on your requirements, here is the detailed technical plan for building the badge firmware and the controller app. Our motto is **Functionality First, UI Second.**

## 1. The Controller App (Python)

**The Best Approach:** We will build a Python desktop application using the **`customtkinter`** library. 
* *Why?* It provides a modern, sleek interface with built-in Light/Dark mode toggling. It is incredibly easy to wire up functional buttons quickly before we worry about making it look pretty.
* *Communication:* The app will communicate with the badge over Wi-Fi using fast HTTP requests or WebSockets for zero-latency control.

### App Layout (Sidebar Mode Selector)
* **🎨 Display & LED Mode:**
  * A text input box that instantly updates the badge's LCD screen.
  * An interactive **Color Wheel** to change the badge's NeoPixel LEDs.
  * A dropdown menu to trigger pre-programmed LED light patterns (Rainbow, Breathe, Chase, Matrix, etc.).
* **📺 TV Blaster Mode:**
  * A massive "FIRE" button that commands the badge to shoot the TV-B-Gone infrared power-off sequence.
* **🔫 Laser Tag Mode:**
  * Interface to manage team color, health, and trigger IR laser shots.
* **🕹️ Game Mode:**
  * A launcher for built-in games (see below).

## 2. The Custom Badge Firmware (C++ / PlatformIO)

We will write a modular ESP32-C3 firmware in C++ using the PlatformIO environment. 

### Core Architecture
1. **Wi-Fi Access Point:** The badge will broadcast its own Wi-Fi network (e.g., `DEFCON_SuperBadge`). You connect your computer to it, completely untethering you from USB cables.
2. **Web Server:** An asynchronous web server running on the badge will listen for API commands from your Python app (e.g., `/led/color?r=255&g=0&b=0`, `/tv/fire`, `/screen/text?msg=Hello`).

### Hardware Integration
* **Display (`TFT_eSPI`):** The fastest library for driving the ST7789 color screen.
* **LEDs (`Adafruit_NeoPixel` or `FastLED`):** To drive the addressable LEDs smoothly.
* **IR Blaster (`IRremoteESP8266`):** We will program this to use the RMT peripheral to blast the universal TV power-off codes.

### 🎮 "Install as many games as we can"
Because we are wiping the text-adventure Zork games, we will instead leverage the color display and the physical buttons (or the app) to install visual games! 
We can easily code classics directly into the firmware:
* **Snake** (Controlled via the app or I2C physical buttons)
* **Pong**
* **Tetris**
* *Note:* If you still want the text-adventure Zork games, we can eventually port the Z-Machine emulator back, but graphical TFT games will be much more impressive for a custom firmware!

---

## Execution Steps
1. **Firmware Foundation:** Write the C++ code to initialize Wi-Fi, the Web Server, the LCD display, and the LEDs.
2. **App Foundation:** Build the basic Python `customtkinter` skeleton with tabs for the modes.
3. **Wire them up:** Connect the App's LED color wheel and text input to the badge's Web Server.
4. **Expand Features:** Add the IR blaster logic and start building the games.

## Open Questions for You

1. **Wi-Fi Approach:** Should the badge broadcast its own Wi-Fi network (Access Point mode - great for conventions where there's no router), or should it connect to your home Wi-Fi network?
2. **Games:** Do you like the idea of visual arcade games (Snake/Pong) on the badge's color screen, or were you specifically hoping to keep the text-adventure (Zork) style games?
