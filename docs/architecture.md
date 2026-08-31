# System Architecture

## Firmware State Machine (ESP32-C3)

The ESP32 runs a single-threaded Arduino-based application that manages the TFT display, physical buttons, and BLE callbacks.

```mermaid
stateDiagram-v2
    [*] --> MODE_WELCOME
    
    MODE_WELCOME --> MODE_MENU : Button Press
    MODE_WELCOME --> MODE_BLE_PAIRING : BLE Pairing Request
    
    MODE_BLE_PAIRING --> MODE_WELCOME : Pairing Success/Fail
    
    MODE_MENU --> MODE_NAMETAG : Select B1
    MODE_MENU --> MODE_GAMES : Select B2
    MODE_MENU --> MODE_TOTP : Select B3
    MODE_MENU --> MODE_LOCK : Select B4
    
    MODE_LOCK --> MODE_MENU : Enter Correct PIN
```

## Web Bluetooth (WebBLE) Data Flow

```mermaid
sequenceDiagram
    participant User
    participant WebApp (Browser)
    participant Badge (ESP32)

    User->>WebApp: Clicks "Connect to Badge"
    WebApp->>Badge: Request BLE Connection
    Badge-->>WebApp: Accepts Connection (Unencrypted)
    
    WebApp->>Badge: sendCommand("sync_time", {t: 1788212009}) (Requires Encryption)
    Badge-->>WebApp: INSUFFICIENT_AUTHENTICATION
    
    WebApp->>Badge: Initiates SMP Pairing Request
    Badge->>Badge: Generates Random 6-digit PIN
    Badge->>Badge: switch_state(MODE_BLE_PAIRING)
    Badge->>User: Displays PIN on TFT Screen
    
    WebApp->>User: OS Prompts for PIN
    User->>WebApp: Types PIN
    WebApp->>Badge: Sends PIN Hash for Verification
    
    alt PIN matches
        Badge-->>WebApp: Pairing Complete (Encrypted: 1)
        Badge->>Badge: switch_state(previous)
        WebApp->>Badge: Re-sends "sync_time" Command
        Badge-->>WebApp: {"status": "ok"}
        WebApp->>User: Reveals Control Panels
    else PIN fails or Cancelled
        Badge-->>WebApp: Connection Dropped
        Badge->>Badge: switch_state(previous)
        WebApp->>User: Leaves UI Locked
    end
```

## Image Blasting via SPIFFS

When an image is uploaded from the Web App, it must be chunked to avoid Bluetooth MTU limits and stored in the ESP32's SPIFFS before it can be decoded to the screen.

```mermaid
flowchart TD
    A[Browser selects JPEG] --> B[FileReader converts to Base64/Bytes]
    B --> C[WebApp chunks data into 500-byte packets]
    C --> D{Send Chunk over BLE}
    D --> E[ESP32 handles `handle_jpeg_chunk`]
    E --> F[Appends to /image.jpg in SPIFFS]
    F --> G{Is Last Chunk?}
    G -- No --> D
    G -- Yes --> H[TJpg_Decoder loads /image.jpg]
    H --> I[Swaps RGB565 Bytes]
    I --> J[Pushes pixels to ST7735 TFT]
```
