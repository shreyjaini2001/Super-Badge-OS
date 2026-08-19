with open("firmware_full.bin", "rb") as f:
    data = f.read()
    
idx = data.find(b"PCF8574 ready at address")
if idx != -1:
    print(f"String found at offset: {hex(idx)}")
    start = max(0, idx - 128)
    end = min(len(data), idx + 128)
    # Just print the strings around it
    chunk = data[start:end]
    import string
    printable = set(bytes(string.printable, 'ascii'))
    res = ""
    for b in chunk:
        if b in printable:
            res += chr(b)
        else:
            res += "."
    print(res)
else:
    print("Not found")
