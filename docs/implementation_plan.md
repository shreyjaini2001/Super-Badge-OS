# Goal Description

Update the Super Badge OS to support:
1. All factory LED patterns.
2. A "Home Screen" button with copyright footer.
3. Proper word-wrapping and font formatting.
4. Image uploading (which naturally solves the Emoji problem).
5. **State Persistence**: The badge will remember its last configured text, image, and LED pattern even after being powered off and disconnected from the computer!

## User Review Required

**Emojis & Fonts:** Microcontrollers don't have enough memory to store thousands of Apple Emojis or complex TrueType fonts. However, because we are building the **Image Upload** feature, you will be able to design your text (with any emojis and fonts) on your computer, take a screenshot or save the image, and push it directly to the badge! 

For standard text sent through the app, I will add a custom C++ word-wrap algorithm to ensure words aren't chopped in half at the edges.

**Persistence:** I will use the ESP32's internal Flash memory (`LittleFS` and `Preferences`). Whenever you send a command from the app, the badge will save it to flash storage. When you unplug the USB and power it on battery later, it will read the flash and instantly restore the last image, text, and LED pattern.

## Proposed Changes

### Python App (`superbadge_app.py`)
- **[MODIFY]** Add a "Text Color" picker and "Font Size" dropdown.
- **[MODIFY]** Add "Upload Image" button (Uses `Pillow` to resize/crop and convert to RGB565).
- **[MODIFY]** Add image streaming protocol (`image_row` commands in Base64).
- **[MODIFY]** Add "Return to Home Screen" button.
- **[MODIFY]** Expand LED dropdown to include: Solid, Rainbow, Breathe, Theater Chase, Cylon, Twinkle, Sparkle.

### Firmware (`main.cpp`)
- **[MODIFY]** Include `<Preferences.h>` and `<LittleFS.h>` to save state permanently.
- **[MODIFY]** Implement custom word-wrapping logic for `display_text` to prevent splitting words.
- **[MODIFY]** On `image_row` receive, write the chunks both to the screen AND to `/image.bin` in LittleFS.
- **[MODIFY]** On boot, load the saved LED color, LED pattern, and draw `/image.bin` or the saved text.
- **[MODIFY]** Add the `home_screen` command and update the boot screen with the "ShreyJain" copyright.
- **[MODIFY]** Implement the math for the 7 new LED animations.

## Verification Plan
1. Send text and verify words wrap nicely.
2. Send an image and verify it streams rapidly.
3. Unplug the USB, plug it into a dumb power bank/battery, and verify the custom image and LEDs automatically restore themselves.
