# Engineering Challenges & Discoveries

Building SuperBadge OS required overcoming several undocumented quirks in the ESP32 ecosystem, the Web Bluetooth API, and the Adafruit graphics libraries. Here is a log of the most significant challenges we faced.

## 1. Web Bluetooth MTU Limits
**The Problem:** When attempting to send a JPEG image from the browser to the badge, we initially tried sending the entire image as a single Base64 JSON payload. This immediately crashed the BLE stack because the default Bluetooth Maximum Transmission Unit (MTU) is typically around 23 bytes (up to 512 bytes with negotiation), and our payloads were massive.
**The Solution:** We implemented a dual-sided chunking mechanism. The JavaScript `FileReader` converts the JPEG into raw bytes and slices it into 500-byte chunks. We send these chunks raw (not JSON encoded) to the badge. The ESP32 firmware dynamically intercepts these raw bytes, appends them to `/image.jpg` in the SPIFFS memory, and only attempts to draw the image once the final chunk is received.

## 2. RGB565 Color Inversion (Psychedelic Colors)
**The Problem:** When the badge finally rendered the uploaded JPEG images, the colors were wildly incorrect (appearing neon or psychedelic). 
**The Discovery:** The DEFCON badge uses an ST7735 TFT screen which expects pixel data in the RGB565 format (16-bit color). The `TJpg_Decoder` library decodes the JPEG into RGB565, but the endianness (byte order) of the decoded bytes was reversed compared to what the Adafruit GFX library expected to be pushed to the screen. 
**The Solution:** We called `TJpgDec.setSwapBytes(true)` initially, but eventually determined that passing `false` and letting the Adafruit library handle the push aligned the endianness correctly, restoring true-color rendering.

## 3. NimBLE Passkey Callbacks (Undocumented Behavior)
**The Problem:** We wanted the badge to dynamically generate a 6-digit PIN and display it on the screen to authenticate the Web App connection. We implemented `NimBLESecurityCallbacks::onPassKeyNotify`, but it was never firing, and the Android phone would silently fail to pair.
**The Discovery:** Deep in the source code of the NimBLE-Arduino library (`NimBLEServer.cpp`), we discovered that when the device capability is set to `BLE_HS_IO_DISPLAY_ONLY`, the library completely bypasses `NimBLESecurityCallbacks`. Instead, it routes the PIN generation directly to `NimBLEServerCallbacks::onPassKeyRequest()`.
**The Solution:** We moved all security logic (including `onAuthenticationComplete` and `onConfirmPIN`) into the primary `MyServerCallbacks` class. This allowed the badge to successfully intercept the passkey request, generate a random number, dynamically switch the TFT screen to a custom `MODE_BLE_PAIRING` state, and render the PIN for the user.

## 4. Web App UI Race Conditions
**The Problem:** When the user clicked "Connect" in the Web App, the browser would prompt the user for the Bluetooth PIN. However, while the OS prompt was active, the Web App's JavaScript would continue executing, revealing the control panels before the badge was actually authenticated.
**The Solution:** We deliberately weaponized an encrypted read/write characteristic. Immediately after connecting, the Web App attempts to send the `sync_time` command. Because the characteristic requires `NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN`, this write attempt triggers an `Insufficient Authentication` exception. The JavaScript `await` blocks execution, the OS takes over to negotiate the PIN, and only if the `try/catch` block succeeds does the UI unlock.
