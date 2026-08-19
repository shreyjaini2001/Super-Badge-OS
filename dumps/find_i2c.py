with open("firmware_full.bin", "rb") as f:
    data = f.read()
    
target = b"I2C bus ready on SDA=%d SCL=%d"
idx = data.find(target)
if idx != -1:
    print(f"String found at offset: {hex(idx)}")
    # In ESP32-C3 firmware, the DROM starts at 0x3C000000. 
    # Usually the binary is mapped to 0x42000000 (IROM) and 0x3C000000 (DROM).
    # We can try to grep the raw binary for the bytes that would load these pins.
else:
    print("String not found")
