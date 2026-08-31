# Security Model & Threat Mitigation

The DEFCON 32 badge was designed to be an exploratory piece of hardware. SuperBadge OS locks it down, turning it into a cryptographic utility that follows offline-first security paradigms.

## 1. Zero-Network Trust (Air-Gapped)
The ESP32-C3 chip features built-in WiFi, but in SuperBadge OS, **WiFi is completely disabled**. 
- **Threat Mitigated:** Remote Over-The-Air (OTA) exploits, network scanning, or malicious firmware flashing over local WiFi networks.
- **Implementation:** The badge receives its UNIX timestamp and configurations exclusively over Bluetooth Low Energy (BLE) from an authenticated smartphone.

## 2. Hardware-Backed Secret Storage
Time-Based One-Time Password (TOTP) algorithms (like Google Authenticator) require a Base32 secret to be stored on the device.
- **Threat Mitigated:** Extraction of 2FA tokens via RAM dumping or serial sniffing.
- **Implementation:** Secrets are written directly to the ESP32's Non-Volatile Storage (NVS) using the `Preferences.h` library. The `lucadentella/TOTP-Arduino` library calculates the HMAC-SHA1 hash locally on the ESP32. The secrets never leave the badge after initial injection.

## 3. Cryptographic BLE Bonding (Passkey Entry)
Standard BLE devices often use "Just Works" pairing, which is highly susceptible to Man-In-The-Middle (MITM) attacks where an attacker intercepts the unencrypted key exchange.
- **Threat Mitigated:** BLE Eavesdropping (Sniffing) and MITM token injection.
- **Implementation:** SuperBadge OS utilizes the Apache NimBLE stack. We configure the device capabilities as `BLE_HS_IO_DISPLAY_ONLY` and enforce Man-in-the-Middle protection (`NimBLEDevice::setSecurityAuth(true, true, true)`). 
- **The Flow:** When a phone attempts to write to the badge, NimBLE intercepts the request and fires `onPassKeyRequest`. The badge generates a random 6-digit PIN, clears the TFT screen, and displays the PIN. The Android/iOS device prompts the user to enter this PIN. The devices then generate Long Term Keys (LTKs) and encrypt the connection. Any unauthenticated device attempting to write to the badge is immediately dropped.

## 4. Physical UI Locking
If the physical badge is stolen or left unattended, an attacker could simply press the buttons to navigate to the TOTP menu and view the 6-digit codes.
- **Threat Mitigated:** Physical access exploitation.
- **Implementation:** The badge features a physical lock screen. A customizable PIN (configurable via the Web App) must be entered using the physical D-pad (B1, B2, B3, B4) before the main menu is accessible.
