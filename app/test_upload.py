import serial, json, time, sys
from PIL import Image

image_path = "/Users/shreyjain/.gemini/antigravity/brain/e0ee2f0b-f0bb-4943-a75d-c234b97023c5/.user_uploaded/media_1787101466301.jpg"
try:
    img = Image.open(image_path).convert('RGB')
    img = img.resize((320, 240))
except Exception as e:
    print("Error loading image:", e)
    sys.exit(1)

try:
    ser = serial.Serial('/dev/cu.usbmodem14101', 115200, timeout=1)
    time.sleep(1) # wait for reset
    
    print("Sending image_start...")
    ser.write((json.dumps({"cmd": "image_start", "args": {}}) + "\n").encode())
    time.sleep(0.5)

    print("Sending rows RAW...")
    for y in range(240):
        row_data = bytearray()
        for x in range(320):
            r, g, b = img.getpixel((x, y))
            color565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            row_data.append(color565 & 0xFF)
            row_data.append(color565 >> 8)
            
        cmd = {"cmd": "image_raw", "args": {"y": y}}
        ser.write((json.dumps(cmd) + "\n").encode())
        
        # Handshake: wait for send_raw
        t0 = time.time()
        ready = False
        while time.time() - t0 < 1.0:
            if ser.in_waiting:
                resp = ser.readline().decode('utf-8', errors='ignore').strip()
                if "send_raw" in resp and str(y) in resp:
                    ready = True
                    break
            time.sleep(0.001)
            
        if not ready:
            print(f"Row {y} TIMEOUT waiting for send_raw")
            sys.exit(1)
            
        ser.write(row_data)
        ser.flush()
        
        t0 = time.time()
        ok = False
        while time.time() - t0 < 1.0:
            if ser.in_waiting:
                resp = ser.readline().decode('utf-8', errors='ignore').strip()
                if "row_ok" in resp and str(y) in resp:
                    ok = True
                    break
            time.sleep(0.001)
            
        if not ok:
            print(f"Row {y} TIMEOUT waiting for row_ok")
            sys.exit(1)
            
        if y % 20 == 0:
            print(f"Row {y} OK")
            
    print("DONE! Image successfully uploaded!")
    ser.close()
except Exception as e:
    print("Error:", e)
