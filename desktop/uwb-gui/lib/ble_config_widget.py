from lib.widget import Widget
import threading
import time
import json
import asyncio
import tkinter as tk
import bleak

class BleConfigWidget(Widget):
    def __init__(self, parent, the_name):
        super().__init__(parent, the_name)
        self.type="BleConfigWidget"
        self.border.configure(bg="#555555")

        self.refresh_thread = None
        self.entry_check_thread = None
        self.print_remote_thread = None

        # first Frame with Widgets
        self.top_frame = tk.Frame(self.border)
        self.top_frame.pack(side="top", fill="x")
        
        self.refresh_button = tk.Button(self.top_frame, text="Refresh", command=self.refresh_devices)
        self.refresh_button.pack(side="left")

        self.device_label = tk.Label(self.top_frame, text="Device:")
        self.device_label.pack(side="left")

        self.device_var = tk.StringVar(value="")
        self.device_menu = tk.OptionMenu(self.top_frame, self.device_var, "")
        self.device_menu.pack(side="left")

        self.services_button = tk.Button(self.top_frame, text="Connect", command=self.connect)
        self.services_button.pack(side="left")

        # second Frame with Widgets
        self.bottom_frame = tk.Frame(self.border)
        self.bottom_frame.pack(side="bottom", fill="x")

        self.set_button = tk.Button(self.bottom_frame, text="Upload Config", command=self.update_remote)
        self.set_button.pack(side="left")

        self.save_button = tk.Button(self.bottom_frame, text="Save Config", command=self.save_config)
        self.save_button.pack(side="left")
        
        # Create a list to hold individual device frames
        self.device_frames = []
        for i in range(5):
            device_frame, entry_fields = self.create_device_frame(i+2)
            self.device_frames.append((device_frame, entry_fields))

        self.address = None
        self.client = bleak.BleakClient("")
        asyncio.set_event_loop(asyncio.new_event_loop())
        self.loop = asyncio.get_event_loop()

        self.check_entry()

    def destroy(self) -> None:
        if self.refresh_thread is not None:
            self.refresh_thread.join()
        if self.print_remote_thread is not None:
            self.print_remote_thread.join()
        if self.check_entry_thread is not None:
            self.check_entry_thread.join()
        return super().destroy()
    
    def create_device_frame(self, device_id):
        device_frame = tk.Frame(self.border)
        device_frame.pack(side="top", fill="x")

        # Widgets for each device frame
        device_label = tk.Label(device_frame, text=f"Device: 0x{device_id:02X}")
        device_label.pack(side="left")

        # Add entry fields for x, y, and z for each device
        entry_fields = {}  # Ein leeres Dictionary zum Speichern der Eingabefelder
        for param in ["x", "y", "z"]:
            entry_label = tk.Label(device_frame, text=f"{param}:")
            entry_label.pack(side="left")
            entry = tk.Entry(device_frame)
            entry.pack(side="left")
            entry_fields[param] = entry  # Speichern Sie das Eingabefeld im Dictionary

        return device_frame, entry_fields  # Rückgabe von device_frame und entry_fields
    
    def check_entry(self):
        self.check_entry_thread = threading.Thread(target=self.check_entry_task)
        self.check_entry_thread.start()

    def check_entry_task(self):
        while True:
            if self.address is not None:
                # Holen Sie die Konfiguration aus dem Bluetooth JSON einmalig
                while self.loop.is_running():
                    pass
                config_data_str = self.loop.run_until_complete(self.get_config_from_bluetooth())
                config_data = json.loads(config_data_str)  # Zeichenfolge in ein Dictionary umwandeln

                # Durchlaufen Sie alle Geräte und Parameter, um die Eingaben zu überprüfen
                for i, (device_frame, entry_fields) in enumerate(self.device_frames):
                    device_id = i + 2
                    for param in ["x", "y", "z"]:
                        # Holen Sie den aktuellen Inhalt des Textfelds
                        entry_value_str = entry_fields[param].get()
                        # Versuchen Sie, die Eingabe in eine Gleitkommazahl zu konvertieren
                        try:
                            entry_value = float(entry_value_str)
                        except ValueError:
                            entry_value = None  # Wenn die Konvertierung fehlschlägt, setzen Sie den Wert auf None

                        device_config = config_data.get(f"0x{device_id:02X}")
                        # Überprüfen Sie, ob die Eingabe geändert wurde
                        if device_config is not None and entry_value is not None and entry_value != device_config.get(param):
                            entry_fields[param].configure(bg="#FF9999")
            time.sleep(0.5)

    async def get_config_from_bluetooth(self):
        # Definieren Sie hier die UUID, von der Sie die Konfiguration lesen möchten
        config_uuid = "76847a0a-2748-4fda-bcd7-74425f0e4a21"

        # Verwenden Sie die BleakClient-Instanz, um die Konfiguration von der UUID zu lesen
        return await self.get_characteristic_value(self.address, config_uuid)

    def connect(self):
        address = self.device_var.get()
        if address:
            self.address = address
            self.print_remote_thread = threading.Thread(target=self.print_remote, args=())
            self.print_remote_thread.start()

    def print_remote(self):
        config_data = self.loop.run_until_complete(self.get_config_from_bluetooth())
        if config_data:
            # Konvertieren Sie das JSON in ein Python-Datenobjekt (z. B. ein Dictionary)
            config_dict = json.loads(config_data)

            # Jetzt haben Sie die Konfigurationsdaten als Python-Datenobjekt und können sie anzeigen oder bearbeiten
            # Hier können Sie die Werte in die richtigen Textfelder einfügen
            for i, (device_frame, entry_fields) in enumerate(self.device_frames):
                device_id = i + 2
                device_config = config_dict.get(f"0x{device_id:02X}")

                if device_config:
                    # Jetzt haben Sie die Konfiguration für das aktuelle Gerät
                    # und können die Werte in die entsprechenden Textfelder einfügen
                    for param, entry in entry_fields.items():
                        entry.delete(0, tk.END)  # Löschen Sie den aktuellen Inhalt des Textfelds
                        entry.insert(tk.END, str(device_config.get(param, "")))  # Fügen Sie den Wert ein
                        entry.configure(bg="lightgreen")

    async def get_characteristic_value(self, address, characteristic):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_char(characteristic)
        return value.decode()

    async def get_services(self, address):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        services = await self.client.get_services()
        return services
    
    def refresh_devices(self):
        self.refresh_thread = threading.Thread(target=self.refresh_devices_task, args=())
        self.refresh_thread.start()

    def refresh_devices_task(self):
        async def discover_devices():
            devices = await bleak.discover()
            return devices
        
        devices = self.loop.run_until_complete(discover_devices())
        self.device_menu["menu"].delete(0, tk.END)
        for d in devices:
            label = f"{d.address} - {d.name}" if d.name else d.address
            self.device_menu["menu"].add_command(label=label, command=tk._setit(self.device_var, d.address))

    def set_characteristic(self, value, characteristic_uuid):
        self.set_value_thread = threading.Thread(target=self.set_characteristic_thread, args=(value, characteristic_uuid))
        self.set_value_thread.start()

    def set_characteristic_thread(self, value, characteristic_uuid):
        async def set_characteristic_value(address, characteristic_uuid, value):
            if not self.client.is_connected:
                self.client = bleak.BleakClient(address)
                await self.client.connect()
            await self.client.write_gatt_char(characteristic_uuid, value.encode())

        address = self.device_var.get()
        if address and value:
            task = set_characteristic_value(address, characteristic_uuid, value)
            while self.loop.is_running():
                pass
            self.loop.run_until_complete(task)

    def save_config(self):
        self.set_characteristic("1", "76847a0a-2748-4fda-bcd7-74425f0e4a13")
        #   TODO: Reconnect/Clear oder update emtrys

    def update_remote(self):        
        for i, (device_frame, entry_fields) in enumerate(self.device_frames):
            device_id = i + 2
            
            updated_device = {
                "id": device_id,  # Hier verwenden Sie die ID des Geräts, das Sie aktualisieren möchten
                "x": float(entry_fields["x"].get()),  # Wert von "x" aus dem entsprechenden Eingabefeld extrahieren
                "y": float(entry_fields["y"].get()),  # Wert von "y" aus dem entsprechenden Eingabefeld extrahieren
                "z": float(entry_fields["z"].get())   # Wert von "z" aus dem entsprechenden Eingabefeld extrahieren
            }
            updated_device_str = json.dumps(updated_device)
            self.set_characteristic(updated_device_str, "76847a0a-2748-4fda-bcd7-74425f0e4a11")
        return