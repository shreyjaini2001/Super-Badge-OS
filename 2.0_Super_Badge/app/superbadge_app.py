import tkinter as tk
import customtkinter as ctk
from tkcolorpicker import askcolor
from tkinter import filedialog
from PIL import Image
import serial
import json
import base64
import time

# Constants
import serial.tools.list_ports

BAUD_RATE = 115200

def get_serial_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "usbmodem" in port.device:
            return port.device
    return None

SERIAL_PORT = get_serial_port()

ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

class SuperBadgeApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        
        self.ser = None
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            print(f"Connected to badge on {SERIAL_PORT}")
        except Exception as e:
            print(f"Could not open serial port: {e}")

        self.title("Super Badge Controller (USB)")
        self.geometry("850x650")
        
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)

        # --- Sidebar ---
        self.sidebar_frame = ctk.CTkFrame(self, width=200, corner_radius=0)
        self.sidebar_frame.grid(row=0, column=0, sticky="nsew")
        self.sidebar_frame.grid_rowconfigure(6, weight=1)

        self.logo_label = ctk.CTkLabel(self.sidebar_frame, text="Super Badge", font=ctk.CTkFont(size=20, weight="bold"))
        self.logo_label.grid(row=0, column=0, padx=20, pady=(20, 10))

        self.btn_display = ctk.CTkButton(self.sidebar_frame, text="Display & LEDs", command=lambda: self.select_tab("display"))
        self.btn_display.grid(row=1, column=0, padx=20, pady=10)

        self.btn_tv = ctk.CTkButton(self.sidebar_frame, text="TV Blaster", command=lambda: self.select_tab("tv"))
        self.btn_tv.grid(row=2, column=0, padx=20, pady=10)
        
        self.btn_lasertag = ctk.CTkButton(self.sidebar_frame, text="TOTP Tokens", command=lambda: self.select_tab("totp"))
        self.btn_lasertag.grid(row=3, column=0, padx=20, pady=10)

        self.btn_games = ctk.CTkButton(self.sidebar_frame, text="Games", command=lambda: self.select_tab("games"))
        self.btn_games.grid(row=4, column=0, padx=20, pady=10)

        self.btn_home = ctk.CTkButton(self.sidebar_frame, text="🏠 Home Screen", fg_color="#2b7a4b", hover_color="#1e5434", command=self.go_home)
        self.btn_home.grid(row=5, column=0, padx=20, pady=20)

        # Appearance Mode
        self.appearance_mode_label = ctk.CTkLabel(self.sidebar_frame, text="Appearance Mode:", anchor="w")
        self.appearance_mode_label.grid(row=7, column=0, padx=20, pady=(10, 0))
        self.appearance_mode_optionemenu = ctk.CTkOptionMenu(self.sidebar_frame, values=["Light", "Dark", "System"], command=self.change_appearance_mode_event)
        self.appearance_mode_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 20))
        self.appearance_mode_optionemenu.set("Dark")
        ctk.set_appearance_mode("Dark")

        # --- Main View ---
        self.main_frame = ctk.CTkFrame(self)
        self.main_frame.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        self.main_frame.grid_rowconfigure(0, weight=1)
        self.main_frame.grid_columnconfigure(0, weight=1)
        
        self.tabs = {}
        
        self.tabs["display"] = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.setup_display_tab()
        
        self.tabs["tv"] = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.setup_tv_tab()
        
        self.tabs["totp"] = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.setup_totp_tab()
        
        self.tabs["games"] = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.setup_games_tab()

        self.select_tab("display")
        self.text_color = (255, 255, 255)

    def select_tab(self, name):
        for tab in self.tabs.values():
            tab.grid_forget()
        self.tabs[name].grid(row=0, column=0, sticky="nsew")

    def change_appearance_mode_event(self, new_appearance_mode: str):
        ctk.set_appearance_mode(new_appearance_mode)
        
    def send_badge_cmd(self, endpoint, params=None):
        if not params:
            params = {}
        payload = {"cmd": endpoint, "args": params}
        json_str = json.dumps(payload) + "\n"
        print(f">> {json_str.strip()[:100]}...")
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(json_str.encode('utf-8'))
            except Exception as e:
                print(f"Error writing to serial: {e}")
        else:
            print("Badge not connected via USB!")

    def go_home(self):
        self.send_badge_cmd("home_screen")

    # --- Setup Tabs ---
    def setup_display_tab(self):
        f = self.tabs["display"]
        
        # --- Image Upload Section ---
        img_frame = ctk.CTkFrame(f)
        img_frame.pack(fill="x", pady=5, padx=20)
        ctk.CTkLabel(img_frame, text="Custom Image (Supports Emojis/Fonts)", font=ctk.CTkFont(size=16, weight="bold")).pack(pady=5)
        ctk.CTkButton(img_frame, text="Upload Image to Badge", command=self.upload_image).pack(pady=5)
        
        # --- Text Section ---
        text_frame = ctk.CTkFrame(f)
        text_frame.pack(fill="x", pady=10, padx=20)
        ctk.CTkLabel(text_frame, text="Basic Display Text", font=ctk.CTkFont(size=16, weight="bold")).pack(pady=5)
        self.display_entry = ctk.CTkEntry(text_frame, width=400, placeholder_text="Enter text to show on badge...")
        self.display_entry.pack(pady=5)
        
        opt_frame = ctk.CTkFrame(text_frame, fg_color="transparent")
        opt_frame.pack(pady=5)
        self.btn_text_color = ctk.CTkButton(opt_frame, text="Pick Text Color", command=self.pick_text_color, width=120)
        self.btn_text_color.pack(side="left", padx=10)
        
        ctk.CTkLabel(opt_frame, text="Size:").pack(side="left", padx=5)
        self.font_size_menu = ctk.CTkOptionMenu(opt_frame, values=["1", "2", "3", "4", "5"], width=60)
        self.font_size_menu.set("3")
        self.font_size_menu.pack(side="left", padx=5)

        ctk.CTkLabel(opt_frame, text="Align:").pack(side="left", padx=5)
        self.align_menu = ctk.CTkOptionMenu(opt_frame, values=["left", "center", "right", "up", "down"], width=80)
        self.align_menu.set("center")
        self.align_menu.pack(side="left", padx=5)

        ctk.CTkButton(text_frame, text="Update Text", command=self.update_text).pack(pady=10)
        
        # --- LED Section ---
        led_frame = ctk.CTkFrame(f)
        led_frame.pack(fill="x", pady=10, padx=20)
        ctk.CTkLabel(led_frame, text="NeoPixel Control", font=ctk.CTkFont(size=16, weight="bold")).pack(pady=5)
        ctk.CTkButton(led_frame, text="Pick LED Color", command=self.pick_color).pack(pady=5)
        
        ctk.CTkLabel(led_frame, text="Light Patterns").pack(pady=(5, 0))
        patterns = ["Solid", "Rainbow", "Breathe", "Theater Chase", "Mixed Cylon", "Mixed Twinkle", "Rainbow Sparkle"]
        self.pattern_menu = ctk.CTkOptionMenu(led_frame, values=patterns, command=lambda v: self.send_badge_cmd("led_pattern", {"type": v}))
        self.pattern_menu.pack(pady=10)

    def update_text(self):
        msg = self.display_entry.get()
        size = int(self.font_size_menu.get())
        align = self.align_menu.get()
        r, g, b = self.text_color
        self.send_badge_cmd("display_text", {"msg": msg, "size": size, "r": r, "g": g, "b": b, "align": align})

    def pick_text_color(self):
        color = askcolor(parent=self, title="Choose Text Color")
        if color and color[0]:
            self.text_color = [int(c) for c in color[0]]

    def pick_color(self):
        color = askcolor(parent=self, title="Choose LED Color")
        if color and color[0]:
            r, g, b = [int(c) for c in color[0]]
            self.send_badge_cmd("led_color", {"r": r, "g": g, "b": b})

    def upload_image(self):
        file_path = filedialog.askopenfilename(filetypes=[("Image Files", "*.png *.jpg *.jpeg *.bmp")])
        if not file_path:
            return
        
        try:
            img = Image.open(file_path).convert('RGB')
            # Resize and crop to 320x240
            target_ratio = 320 / 240
            img_ratio = img.width / img.height
            if img_ratio > target_ratio:
                new_width = int(img.height * target_ratio)
                left = (img.width - new_width) // 2
                img = img.crop((left, 0, left + new_width, img.height))
            else:
                new_height = int(img.width / target_ratio)
                top = (img.height - new_height) // 2
                img = img.crop((0, top, img.width, top + new_height))
                
            img = img.resize((320, 240), Image.Resampling.LANCZOS)
            
            # Send start command
            self.send_badge_cmd("image_start", {"w": 320, "h": 240})
            time.sleep(0.1)
            
            # Stream rows
            for y in range(240):
                row_data = bytearray()
                for x in range(320):
                    r, g, b = img.getpixel((x, y))
                    color565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                    row_data.append(color565 & 0xFF)
                    row_data.append(color565 >> 8)
                
                self.send_badge_cmd("image_raw", {"y": y})
                
                # Wait for badge to say send_raw
                t0 = time.time()
                ready = False
                while time.time() - t0 < 1.0:
                    if self.ser.in_waiting:
                        resp = self.ser.readline().decode('utf-8', errors='ignore').strip()
                        if "send_raw" in resp and str(y) in resp:
                            ready = True
                            break
                    time.sleep(0.001)
                
                if ready:
                    self.ser.write(row_data)
                    self.ser.flush()
                
                # Handshake: Wait for badge to acknowledge it processed this row
                timeout = time.time() + 1.0
                while time.time() < timeout:
                    if self.ser.in_waiting:
                        resp = self.ser.readline().decode('utf-8', errors='ignore').strip()
                        if "row_ok" in resp and str(y) in resp:
                            break
                    time.sleep(0.001)
                
            print("Image streaming complete.")
        except Exception as e:
            print(f"Failed to process image: {e}")


    def setup_tv_tab(self):
        f = self.tabs["tv"]
        ctk.CTkLabel(f, text="TV-B-Gone Infrared Blaster", font=ctk.CTkFont(size=24, weight="bold")).pack(pady=(50, 20))
        btn_fire = ctk.CTkButton(f, text="FIRE ALL CODES", fg_color="red", hover_color="darkred", 
                                 font=ctk.CTkFont(size=20, weight="bold"), height=80, width=250,
                                 command=lambda: self.send_badge_cmd("tv_fire"))
        btn_fire.pack(pady=30)
        ctk.CTkLabel(f, text="Warning: This will cycle through hundreds of TV power-off codes.").pack()

    def setup_totp_tab(self):
        f = self.tabs["totp"]
        ctk.CTkLabel(f, text="TOTP Authenticator (6 Slots)", font=ctk.CTkFont(size=24, weight="bold")).pack(pady=(20, 5))
        
        # Display current slots
        self.slot_labels = []
        list_frame = ctk.CTkFrame(f)
        list_frame.pack(pady=5, padx=20, fill="x")
        
        ctk.CTkLabel(list_frame, text="Current Slots:", font=ctk.CTkFont(weight="bold")).grid(row=0, column=0, columnspan=2, pady=5)
        
        for i in range(6):
            btn_name = "B1" if i < 2 else "B2" if i < 4 else "B3"
            lbl = ctk.CTkLabel(list_frame, text=f"Slot {i+1} ({btn_name}): [Empty]")
            lbl.grid(row=i//2 + 1, column=i%2, padx=20, pady=2, sticky="w")
            self.slot_labels.append(lbl)
            
        ctk.CTkButton(list_frame, text="Refresh List", command=self.refresh_totps, width=120).grid(row=4, column=0, columnspan=2, pady=10)

        # Form to add/delete
        form = ctk.CTkFrame(f)
        form.pack(pady=5, padx=20, fill="x")
        
        ctk.CTkLabel(form, text="Manage Slot:").pack(pady=(5,0))
        self.totp_slot_var = ctk.StringVar(value="Slot 1 (B1)")
        self.totp_slot_menu = ctk.CTkOptionMenu(form, variable=self.totp_slot_var, values=[
            "Slot 1 (B1)", "Slot 2 (B1)", "Slot 3 (B2)", "Slot 4 (B2)", "Slot 5 (B3)", "Slot 6 (B3)"
        ])
        self.totp_slot_menu.pack(pady=5)
        
        ctk.CTkLabel(form, text="Account Name:").pack(pady=(5,0))
        self.totp_name_entry = ctk.CTkEntry(form, width=250)
        self.totp_name_entry.pack(pady=5)
        
        ctk.CTkLabel(form, text="Base32 Secret:").pack(pady=(5,0))
        self.totp_secret_entry = ctk.CTkEntry(form, width=250)
        self.totp_secret_entry.pack(pady=5)
        
        btn_frame = ctk.CTkFrame(form, fg_color="transparent")
        btn_frame.pack(pady=10)
        ctk.CTkButton(btn_frame, text="Save to Slot", command=self.add_totp, fg_color="green", hover_color="darkgreen", width=120).pack(side="left", padx=10)
        ctk.CTkButton(btn_frame, text="Delete Slot", command=self.delete_totp, fg_color="red", hover_color="darkred", width=120).pack(side="left", padx=10)
        
        time_frame = ctk.CTkFrame(f)
        time_frame.pack(pady=5, padx=20, fill="x")
        ctk.CTkButton(time_frame, text="Sync Time Now", fg_color="blue", hover_color="darkblue", command=self.sync_time).pack(pady=10)

    def refresh_totps(self):
        self.send_badge_cmd("get_totps")
        if not self.ser: return
        import time, json
        t0 = time.time()
        while time.time() - t0 < 1.0:
            if self.ser.in_waiting:
                resp = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if not resp: continue
                try:
                    data = json.loads(resp)
                    if data.get("event") == "totp_list":
                        slots = data.get("data", [])
                        for i, name in enumerate(slots):
                            btn_name = "B1" if i < 2 else "B2" if i < 4 else "B3"
                            disp = name if name else "[Empty]"
                            self.slot_labels[i].configure(text=f"Slot {i+1} ({btn_name}): {disp}")
                        break
                except json.JSONDecodeError:
                    pass

    def delete_totp(self):
        slot_idx = int(self.totp_slot_var.get().split(" ")[1]) - 1
        self.send_badge_cmd("delete_totp", {"slot": slot_idx})
        self.refresh_totps()

    def add_totp(self):
        slot_idx = int(self.totp_slot_var.get().split(" ")[1]) - 1
        name = self.totp_name_entry.get()
        secret = self.totp_secret_entry.get().replace(" ", "").upper()
        if not name or not secret:
            print("Name and Secret required!")
            return
        self.send_badge_cmd("add_totp", {"slot": slot_idx, "name": name, "secret": secret})
        self.sync_time()
        self.refresh_totps()

    def sync_time(self):
        import time
        current_unix = int(time.time())
        self.send_badge_cmd("sync_time", {"t": current_unix})

    def setup_games_tab(self):
        f = self.tabs["games"]
        ctk.CTkLabel(f, text="Arcade Games Launcher", font=ctk.CTkFont(size=24, weight="bold")).pack(pady=(30, 20))
        games = [("🐍 Snake", "snake"), ("🏓 Pong", "pong"), ("🧱 Tetris", "tetris")]
        for name, cmd in games:
            ctk.CTkButton(f, text=name, font=ctk.CTkFont(size=16), height=50, width=200,
                          command=lambda c=cmd: self.send_badge_cmd("game_launch", {"game": c})).pack(pady=10)

if __name__ == "__main__":
    app = SuperBadgeApp()
    app.mainloop()
