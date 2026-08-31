# Installation & Usage Guide

Follow these steps to wipe your stock DEFCON 32 badge and install SuperBadge OS.

## 1. Prerequisites
- **Hardware:** DEFCON 32 Badge (ESP32-C3-DevKitM-1 equivalent).
- **Software:** PlatformIO (via VSCode) or the `pio` CLI.
- **Cable:** A USB-C cable capable of data transfer.

## 2. Flashing the Firmware
1. Clone this repository to your local machine.
2. Open the `2.0_Super_Badge/firmware` directory in VSCode with the PlatformIO extension installed.
3. Connect your DEFCON badge to your computer via USB-C.
4. Click the **Upload** button in PlatformIO (or run `pio run -t upload`).
5. PlatformIO will automatically compile the dependencies (NimBLE, ArduinoJson, Adafruit GFX, etc.) and flash the ESP32.
6. The badge will reboot and display the "WELCOME TO SUPER BADGE OS" screen!

## 3. Using the Web App
Because SuperBadge OS uses Web Bluetooth (WebBLE), there is no app to install from the App Store or Google Play.

1. Open a WebBLE-compatible browser on your smartphone (Google Chrome for Android, or Bluefy for iOS).
2. Navigate to the hosted GitHub Pages URL for the Web App (or host `2.0_Super_Badge/web_app/index.html` locally).
3. Click **Connect to Badge**.
4. Your browser will prompt you to select a Bluetooth device. Select **SuperBadge OS**.
5. **The Pairing Phase:** Your badge screen will suddenly clear and display a blue `BLE PAIRING` screen with a 6-digit code.
6. Your phone will pop up a system dialog asking for a PIN. Enter the 6-digit code shown on the badge.
7. Once paired, the Web App UI will unlock, and your phone will automatically securely sync its UNIX timestamp to the badge.

## 4. Managing TOTP Tokens
1. In the Web App, go to the **TOTP Tokens** panel.
2. Enter a Name (e.g., "GitHub") and the Base32 Secret Key provided by your service provider.
3. Select a Slot (0-5) and click **Save Token**.
4. On the physical badge, press any button to enter the Main Menu, navigate to `TOTP Tokens`, and press Select (B3). 
5. The badge will calculate the current 6-digit 2FA code based on the time synced from your phone.

## 5. Setting a Hardware Lock PIN
1. In the Web App, under the **Controls** panel, locate the Set Badge PIN input.
2. Type a physical button combination using numbers 1, 2, 3, and 4 (corresponding to B1, B2, B3, B4 on the badge). E.g., `1234`.
3. Click **Set Badge PIN**.
4. The next time the badge boots or goes to sleep, it will require that exact physical button combination to unlock.
